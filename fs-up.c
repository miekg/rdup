/*
 * Copyright (c) 2009 - 2011 Miek Gieben
 * License: GPLv3(+), see LICENSE for details
 * rdup-up -- update an directory tree with and rdup archive
 *
 * File related functions
 *
 */

#include "rdup-up.h"
#include "protocol.h"

extern int sig;
extern gboolean opt_dry;
extern gboolean opt_table;
extern gboolean opt_quiet;
extern gboolean opt_chown;
extern guint opt_strip;
extern gint opt_verbose;
extern GSList *hlink_list;

/* signal.c */
void got_sig(int signal);

/* common.c */
struct rdup *entry_dup(struct rdup *);
void entry_free(struct rdup *);

static gboolean mk_time(struct rdup *e)
{
	struct utimbuf ut;
	/* we don't carry the a_time, how cares anyway with noatime? */
	ut.actime = ut.modtime = e->f_mtime;

	if (utime(e->f_name, &ut) == -1)
		msgd(__func__, __LINE__, _("Failed to set mtime '%s\': %s"),
		     e->f_name, strerror(errno));
	return TRUE;
}

static gboolean
mk_chown(struct rdup *e, GHashTable * uidhash, GHashTable * gidhash)
{
	uid_t u;
	gid_t g;
	gchar *dup;

	u = lookup_uid(uidhash, e->f_user, e->f_uid);
	g = lookup_gid(gidhash, e->f_group, e->f_gid);


	if (lchown(e->f_name, u, g) == -1) {
		if (opt_chown) {
			/* chown failed, use and create ._rdup_. - file, it that fails too
			 * we're out of luck - so we don't care about the return value */
			if (S_ISDIR(e->f_mode)) {
				chown_write(e->f_name, NULL, e->f_uid,
					    e->f_user, e->f_gid, e->f_group);
			} else {
				/* dirname may (will!) mess source string, so
				 * we have to use duplicates! */
				dup = g_strdup(e->f_name);
				chown_write(dirname(dup),
					    basename(e->f_name), e->f_uid,
					    e->f_user, e->f_gid, e->f_group);
				g_free(dup);
			}
		} else {
			if (!opt_quiet) {
				msgd(__func__, __LINE__,
				     _("Failed to chown `%s\': %s"), e->f_name,
				     strerror(errno));
			}
		}
	}
	return TRUE;
}

static gboolean mk_mode(struct rdup *e)
{
	chmod(e->f_name, e->f_mode);
	return TRUE;
}

static gboolean
mk_meta(struct rdup *e, GHashTable * uidhash, GHashTable * gidhash)
{
	mk_mode(e);
	mk_chown(e, uidhash, gidhash);
	mk_time(e);
	return TRUE;
}

static gboolean
mk_dev(struct rdup *e, GHashTable * uidhash, GHashTable * gidhash)
{
	gchar *parent;
	struct stat *st;

	if (opt_dry)
		return TRUE;

	if (!rm(e->f_name))
		return FALSE;

	if (mknod(e->f_name, e->f_mode, e->f_rdev) == -1) {
		if ( errno == ENOENT ) {
			// A path element does not exist, this is probably a -R dump
			// Create path with default perms, dir entry itself will appear later
			// and set things right regarding perms/ownership
			dir_mkpath(e->f_name);

			// Retry mknod
			if (mknod(e->f_name, e->f_mode, e->f_rdev) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to make device `%s\': %s"),
				     e->f_name, strerror(errno));
				return FALSE;
			}
		} else if (errno == EACCES) {
			parent = dir_parent(e->f_name);
			st = dir_write(parent);
			if (mknod(e->f_name, e->f_mode, e->f_rdev) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to make device `%s\': %s"),
				     e->f_name, strerror(errno));
				dir_restore(parent, st);
				g_free(st);
				g_free(parent);
				return FALSE;
			}
			dir_restore(parent, st);
			g_free(st);
			g_free(parent);
		} else {
			msgd(__func__, __LINE__,
			     _("Failed to make device `%s\': %s"), e->f_name,
			     strerror(errno));
			return FALSE;
		}
	}
	mk_meta(e, uidhash, gidhash);
	return TRUE;
}

static gboolean
mk_sock(struct rdup *e, GHashTable * uidhash, GHashTable * gidhash)
{
	gchar *parent;
	struct stat *st;

	if (opt_dry)
		return TRUE;

	if (!rm(e->f_name))
		return FALSE;

	if (mkfifo(e->f_name, e->f_mode) == -1) {
		if ( errno == ENOENT ) {
			// A path element does not exist, this is probably a -R dump
			// Create path with default perms, dir entry itself will appear later
			// and set things right regarding perms/ownership
			dir_mkpath(e->f_name);

			// Retry fifio
			if (mkfifo(e->f_name, e->f_mode) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to make socket  `%s\': %s"),
				     e->f_name, strerror(errno));
				return FALSE;
			}
		} else if (errno == EACCES) {
			parent = dir_parent(e->f_name);
			st = dir_write(parent);
			if (mkfifo(e->f_name, e->f_mode) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to make socket `%s\': %s"),
				     e->f_name, strerror(errno));
				dir_restore(parent, st);
				g_free(st);
				g_free(parent);
				return FALSE;
			}
			dir_restore(parent, st);
			g_free(st);
			g_free(parent);
		} else {
			msgd(__func__, __LINE__,
			     _("Failed to make socket `%s\': %s"), e->f_name,
			     strerror(errno));
			return FALSE;
		}
	}
	mk_meta(e, uidhash, gidhash);
	return TRUE;
}

static gboolean
mk_link(struct rdup *e, char *p, GHashTable * uidhash, GHashTable * gidhash)
{
	struct stat *st;
	gchar *t;
	gchar *parent;

	if (opt_dry)
		return TRUE;

	if (!rm(e->f_name))
		return FALSE;

	/* symlink */
	if (S_ISLNK(e->f_mode)) {
		if (symlink(e->f_target, e->f_name) == -1) {
			if ( errno == ENOENT ) {
				// A path element does not exist, this is probably a -R dump
				// Create path with default perms, dir entry itself will appear later
				// and set things right regarding perms/ownership
				dir_mkpath(e->f_name);

				// Retry symlink
				if (symlink(e->f_target, e->f_name) == -1) {
					msgd(__func__, __LINE__,
					     _("Failed to make symlink `%s -> %s\': %s"),
					     e->f_name, e->f_target, strerror(errno));
					return FALSE;
				}
			} else if (errno == EACCES) {
				parent = dir_parent(e->f_name);
				st = dir_write(parent);
				if (symlink(e->f_target, e->f_name) == -1) {
					msgd(__func__, __LINE__,
					     _
					     ("Failed to make symlink `%s -> %s\': %s"),
					     e->f_name, e->f_target,
					     strerror(errno));
					dir_restore(parent, st);
					g_free(st);
					g_free(parent);
					return FALSE;
				}
				dir_restore(parent, st);
				g_free(st);
				g_free(parent);
			} else {
				msgd(__func__, __LINE__,
				     _
				     ("Failed to make symlink `%s -> %s\': %s"),
				     e->f_name, e->f_target, strerror(errno));
				return FALSE;
			}
		}
		mk_chown(e, uidhash, gidhash);

		// Set mtime and atime through lutimes(2);
		struct timeval times[2];

		// Element 0 is atime, 1 is mtime
		times[0].tv_sec = times[1].tv_sec = e->f_mtime;
		times[0].tv_usec = times[1].tv_usec = 0;
		if ( lutimes(e->f_name, times) == -1)
		{
			msgd(__func__, __LINE__, _("Failed to set mtime on symlink target '%s\': %s"),
			     e->f_name, strerror(errno));
		}
		return TRUE;
	}

	/* hardlink */
	/* target must also fall in backup dir */
	t = g_strdup_printf("%s%s", p, e->f_target);
	e->f_target = t;
	hlink_list = g_slist_append(hlink_list, entry_dup(e));
	return TRUE;
}

static gboolean
mk_reg(FILE * in, struct rdup *e, GHashTable * uidhash, GHashTable * gidhash)
{
	FILE *out = NULL;
	char *buf;
	gchar *parent;
	size_t bytes;
	gboolean ok = TRUE;
	gboolean old_dry = opt_dry;
	struct stat *st;

	/* with opt_dry we can't just return TRUE; as we may
	 * need to suck in the file's content - which is thrown
	 * away in that case */

	if (!e->f_name) {
		/* fake an opt_dry */
		opt_dry = TRUE;
	}

	if (!opt_dry) {
		if (!rm(e->f_name)) {
			opt_dry = old_dry;
			return FALSE;
		}
	}
	if (!opt_dry && !(out = fopen(e->f_name, "w"))) {
		if ( errno == ENOENT ) {
			// A path element does not exist, this is probably a -R dump
			// Create path with default perms, dir entry itself will appear later
			// and set things right regarding perms/ownership
			dir_mkpath(e->f_name);

			// Reopen file now
			if (!(out = fopen(e->f_name, "w"))) {
				msgd(__func__, __LINE__,
				     _("ENOENT file `%s\': %s"),
				     e->f_name, strerror(errno));
				ok = FALSE;
			}
		} else if (errno == EACCES) {
			parent = dir_parent(e->f_name);
			st = dir_write(parent);
			if (!(out = fopen(e->f_name, "w"))) {
				msgd(__func__, __LINE__,
				     _("Failed to open file `%s\': %s"),
				     e->f_name, strerror(errno));
				g_free(st);
				g_free(parent);
				ok = FALSE;
			}
			dir_restore(parent, st);
			g_free(st);
			g_free(parent);
		} else {
			msgd(__func__, __LINE__,
			     _("Failed to open file `%s\': %s"), e->f_name,
			     strerror(errno));
			ok = FALSE;
		}
	}

	/* we need to read the input to not upset
	 * the flow into rdup-up, but we are not
	 * creating anything when opt_dry is active
	 */
	buf = g_malloc(BUFSIZE + 1);
	while ((bytes = block_in_header(in)) > 0) {
		if (block_in(in, bytes, buf) == -1) {
			if (out)
				fclose(out);
			opt_dry = old_dry;
			g_free(buf);
			return FALSE;
		}
		if (ok && !opt_dry) {
			if (fwrite(buf, sizeof(char), bytes, out) != bytes) {
				msgd(__func__, __LINE__,
				     _("Write failure `%s\': %s"), e->f_name,
				     strerror(errno));
				if (out)
					fclose(out);
				opt_dry = old_dry;
				g_free(buf);
				return FALSE;
			}
		}
	}
	g_free(buf);
	if (ok && out)
		fclose(out);

	if (ok && !opt_dry)
		mk_meta(e, uidhash, gidhash);

#ifdef DEBUG
	msgd(__func__, __LINE__, "Wrote file `%s\'", e->f_name);
#endif				/* DEBUG */
	opt_dry = old_dry;
	return TRUE;
}

static gboolean
mk_dir(struct rdup *e, GHashTable * uidhash, GHashTable * gidhash)
{
	struct stat *s;
	struct stat st;
	gchar *parent;

	if (opt_dry)
		return TRUE;

	if (lstat(e->f_name, &st) == 0) {
		if (S_ISDIR(st.st_mode)) {
#if 0
			msgd(__func__, __LINE__,
			     _("Updating current dir `%s\'"), e->f_name);
#endif				/* DEBUG */
			/* some dir is here - update the perms and ownership */
			mk_meta(e, uidhash, gidhash);
			return TRUE;
		}
	}
	/* nothing there */

	if (mkdir(e->f_name, e->f_mode) == -1) {
		if ( errno == ENOENT ) {
			// A path element does not exist, this is probably a -R dump
			// Create path with default perms, dir entry itself will appear later
			// and set things right regarding perms/ownership
			dir_mkpath(e->f_name);

			// Retry mkdir
			if (mkdir(e->f_name, e->f_mode) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to create directory `%s\': %s"),
				     e->f_name, strerror(errno));
                                return FALSE;
			}
		} else if (errno == EACCES) {
			/* make parent dir writable, and try again */
			parent = dir_parent(e->f_name);
#ifdef DEBUG
			msgd(__func__, __LINE__, _("EACCES for `%s\'"), parent);
#endif				/* DEBUG */
			s = dir_write(parent);
			if (!s)
				msgd(__func__, __LINE__,
				     _("Failed to make parent writable"));

			if (mkdir(e->f_name, e->f_mode) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to create directory `%s\': %s"),
				     e->f_name, strerror(errno));
				dir_restore(parent, s);
				g_free(s);
				g_free(parent);
				return FALSE;
			}
			dir_restore(parent, s);
			g_free(s);
			g_free(parent);
		} else {
			msgd(__func__, __LINE__,
			     _("Failed to create directory `%s\': %s"),
			     e->f_name, strerror(errno));
			return FALSE;
		}
	}
	mk_meta(e, uidhash, gidhash);
	return TRUE;
}

/* make (or delete) an object in the filesystem */
gboolean
mk_obj(FILE * in, char *p, struct rdup * e, GHashTable * uidhash,
       GHashTable * gidhash)
{
	if (opt_verbose >= 1 && e->f_name) {
		if (S_ISLNK(e->f_mode) || e->f_lnk)
			fprintf(stderr, "%s -> %s\n", e->f_name, e->f_target);
		else
			fprintf(stderr, "%s\n", e->f_name);
	}
	if (opt_table)
		rdup_write_table(e, stdout);

	/* split here - or above - return when path is zero length
	 * for links check that the f_size is zero */
	switch (e->plusmin) {
	case MINUS:
		if (opt_dry || !e->f_name)
			return TRUE;

		return rm(e->f_name);
	case PLUS:
		/* opt_dry handled within the subfunctions */

		/* only files, no hardlinks! */
		if (S_ISREG(e->f_mode) && !e->f_lnk)
			return mk_reg(in, e, uidhash, gidhash);

		/* no name, we can exit here - for files this is handled
		 * in mk_reg, because we may need to suck in data */
		if (e->f_name == NULL)
			return TRUE;

		if (S_ISDIR(e->f_mode))
			return mk_dir(e, uidhash, gidhash);

		/* First sym and hardlinks and then regular files */
		if (S_ISLNK(e->f_mode) || e->f_lnk)
			return mk_link(e, p, uidhash, gidhash);

		if (S_ISBLK(e->f_mode) || S_ISCHR(e->f_mode))
			return mk_dev(e, uidhash, gidhash);

		// There's no way to restore a named socket
		if (S_ISSOCK(e->f_mode))
			return TRUE;

		if (S_ISFIFO(e->f_mode))
			return mk_sock(e, uidhash, gidhash);
	}
	/* only reached during the heat death of the universe */
	return TRUE;
}

/* Create the remaining hardlinks in the target directory */
gboolean mk_hlink(GSList * h)
{
	struct rdup *e;
	GSList *p;
	struct stat *st;
	gchar *parent;

	if (opt_dry)
		return TRUE;

	for (p = g_slist_nth(h, 0); p; p = p->next) {
		e = (struct rdup *)p->data;
		if (link(e->f_target, e->f_name) == -1) {
			if (errno == EACCES) {
				parent = dir_parent(e->f_name);
				st = dir_write(parent);
				if (link(e->f_target, e->f_name) == -1) {
					msgd(__func__, __LINE__,
					     _
					     ("Failed to create hardlink `%s -> %s\': %s"),
					     e->f_name, e->f_target,
					     strerror(errno));
					dir_restore(parent, st);
					g_free(parent);
					return FALSE;
				}
				dir_restore(parent, st);
				g_free(parent);
				return TRUE;
			} else {
				msgd(__func__, __LINE__,
				     _
				     ("Failed to create hardlink `%s -> %s\': %s"),
				     e->f_name, e->f_target, strerror(errno));
				return FALSE;
			}
		}
		entry_free(e);
	}
	return TRUE;
}
