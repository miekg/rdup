/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rdup-up -- update an directory tree with 
 * and rdup archive
 *
 * File related functions
 *
 */

#include "rdup-up.h"
#include "protocol.h"

extern sig_atomic_t sig;
extern gboolean opt_dry;
extern gint opt_verbose;
extern GSList *hlink;

/* signal.c */
void got_sig(int signal);

static gboolean
mk_dev(struct r_entry *e, gboolean exists) {
	/* XXX dir write perms */
	if (opt_dry)
		return TRUE;

	if (exists) {
		if (!rm(e->f_name)) {
			msg(_("Failed to remove existing entry: '%s\'"), e->f_name);
			return FALSE;
		}
	}

	if (mknod(e->f_name, e->f_mode, e->f_rdev) == -1) {
		msg(_("Failed to make device: `%s\': %s"), e->f_name, strerror(errno));
	}
	chmod(e->f_name, e->f_mode);
	if (getuid() == 0)
		if (chown(e->f_name, e->f_uid, e->f_gid) == -1) { }
	return TRUE;
}

static gboolean
mk_sock(struct r_entry *e, gboolean exists) {
	/* XXX dir write perms */
	if (opt_dry)
		return TRUE;

	if (exists) {
		if (!rm(e->f_name)) {
			msg(_("Failed to remove existing entry: '%s\'"), e->f_name);
			return FALSE;
		}
	}

	if (mkfifo(e->f_name, e->f_mode) == -1) {
		msg(_("Failed to make socket: `%s\': %s"), e->f_name, strerror(errno));
	}
	chmod(e->f_name, e->f_mode);
	if (getuid() == 0)
		if (chown(e->f_name, e->f_uid, e->f_gid) == -1) { }
	return TRUE;
}

static gboolean
mk_link(struct r_entry *e, gboolean exists, char *s, char *t, char *p)
{
	struct stat *st;
	gchar *parent;

	if (opt_dry)
		return TRUE;

	/* there is something */
	if (exists) {
		if (!rm(s)) {
			msg(_("Failed to remove existing entry: '%s\'"), s);
			return FALSE;
		}
	}

	/* symlink */
	if (S_ISLNK(e->f_mode)) {
		if (symlink(t, s) == -1) {
			if (errno == EACCES) {
				parent = dir_parent(e->f_name);
				st = dir_write(parent);
				if (symlink(t, s) == -1) {
					msg(_("Failed to make symlink: `%s -> %s\': %s"), s, t, strerror(errno));
					dir_restore(parent, st);
					g_free(parent);
					return FALSE;
				} 
				dir_restore(parent, st);
				g_free(parent);
				return TRUE;
			} else {
				msg(_("Failed to make symlink: `%s -> %s\': %s"), s, t, strerror(errno));
				return FALSE;
			}
		}
		if (getuid() == 0)
			if (lchown(e->f_name, e->f_uid, e->f_gid) == -1) { }
		return TRUE;
	}

	/* hardlink */
	/* make target also fall in the backup dir */
	t = g_strdup_printf("%s%s", p, s + e->f_size + 4);
	e->f_name = g_strdup_printf("%s -> %s", s, t);
	e->f_size = strlen(s);
	e->f_name_size = strlen(e->f_name);
	hlink = g_slist_append(hlink, e);
	return TRUE;
}

static gboolean
mk_reg(FILE *in, struct r_entry *e, gboolean exists)
{
	FILE *out = NULL;
	char *buf;
	size_t  bytes;
	gboolean ok = TRUE;
	struct stat *st;

	/* there is something */
	if (exists && !opt_dry)  {
		if (!rm(e->f_name)) {
			msg(_("Failed to remove existing entry: '%s\'"), e->f_name);
			return FALSE;
		}
	}

	if (!opt_dry && !(out = fopen(e->f_name, "w"))) {
		if (errno == EACCES) {
			st = dir_write(dir_parent(e->f_name));
			if (!(out = fopen(e->f_name, "w"))) {
				msg(_("Failed to open file `%s\': %s"), e->f_name, strerror(errno));
				ok = FALSE;
			} else {
				ok = TRUE;
			}
			dir_restore(dir_parent(e->f_name), st);
		} else {
			msg(_("Failed to open file `%s\': %s"), e->f_name, strerror(errno));
			ok = FALSE;
		}
	} 

	if (ok && !opt_dry)
		chmod(e->f_name, e->f_mode);

	/* only root my chown files */
	if (ok && !opt_dry && getuid() == 0)
		if (fchown(fileno(out), e->f_uid, e->f_gid) == -1) { } /* todo */

	/* we need to read the input to not upset
	 * the flow into rdup-up, but we are not
	 * creating anything when opt_dry is active
	 */
	buf   = g_malloc(BUFSIZE + 1);
	while ((bytes = block_in_header(in)) > 0) {
		if (block_in(in, bytes, buf) == -1) {
			fclose(out);
			return FALSE;
		}
		if (ok && !opt_dry) {
			if (fwrite(buf, sizeof(char), bytes, out) != bytes) {
				msg(_("Write failure `%s\': %s"), e->f_name, strerror(errno));
				fclose(out);
				return FALSE;
			}
		}
	}
	
	g_free(buf);
	if (ok)
		fclose(out); 
	return TRUE;
}

static gboolean
mk_dir(struct r_entry *e, struct stat *st, gboolean exists) 
{
	struct stat *s;
	gchar *parent;

	if (opt_dry)
		return TRUE;

	if (exists && S_ISDIR(st->st_mode)) {
		/* something is here - update the permissions */
		chmod(e->f_name, e->f_mode);
		return TRUE;
	}

	if (mkdir(e->f_name, e->f_mode) == -1) {
		if (errno == EACCES) {
			/* make parent dir writable, and try again */
			parent = dir_parent(e->f_name);
			fprintf(stderr, "debug %s\n", parent);
			s = dir_write(parent);
			if (mkdir(e->f_name, e->f_mode) == -1) {
				msg(_("Failed to create directory `%s\': %s"), e->f_name, strerror(errno));
				dir_restore(parent, s);
				g_free(parent);
				return FALSE;
			}
			dir_restore(parent, s);
			g_free(parent);
			return TRUE;
		} else {
			msg(_("Failed to create directory `%s\': %s"), e->f_name, strerror(errno));
			return FALSE;
		}
	}
	/* only root my chown files */
	if (getuid() == 0)
		if (chown(e->f_name, e->f_uid, e->f_gid) == -1) { }
			
	return TRUE;
}


/* make an object in the filesystem */
gboolean
mk_obj(FILE *in, char *p, struct r_entry *e, guint strip) 
{
	char     *s, *t;
	gboolean exists;
	struct stat st;

	/* XXX not yet implemented, not sure if ever */
	strip = strip;

	if (lstat(e->f_name, &st) == -1) 
		exists = FALSE;
	else
		exists = TRUE;

	if (opt_verbose > 0)
		printf("%s\n", e->f_name);

	switch(e->plusmin) {
		case MINUS:
			if (opt_dry)
				return TRUE;

			/* remove all stuff you can find */
			if (S_ISLNK(e->f_mode) || e->f_lnk) {
				/* get out the source name */
				s = e->f_name;
				s[e->f_size] = '\0';
			} else {
				s = e->f_name;
			}
			return rm(s);
		case PLUS:
			/* opt_dry handled within the subfunctions */
			if (S_ISDIR(e->f_mode))
				return  mk_dir(e, &st, exists);	

			/* First sym and hardlinks and then regular files */
			if (S_ISLNK(e->f_mode) || e->f_lnk) {
				/* get out the source name and re-stat it */
				s = e->f_name;
				s[e->f_size] = '\0';
				t = s + e->f_size + 4; /* ' -> ' */

				if (lstat(s, &st) == -1) 
					exists = FALSE;
				else
					exists = TRUE;

				return mk_link(e, exists, s, t, p);
			}

			if (S_ISREG(e->f_mode))
				return mk_reg(in, e, exists);

			if (S_ISBLK(e->f_mode) || S_ISCHR(e->f_mode))
				return mk_dev(e, exists);

			if (S_ISSOCK(e->f_mode))
				return mk_sock(e, exists);
	}
	/* huh still alive */
	return TRUE;
}

/* Create the remaining hardlinks in the target directory */
gboolean
mk_hlink(GSList *h)
{
	struct r_entry *e;
	GSList *p;
	char *s, *t;
	struct stat *st;
	gchar *parent;

	if (opt_dry)
		return TRUE;

	for (p = g_slist_nth(h, 0); p; p = p->next) { 
		e = (struct r_entry *)p->data;

		s = e->f_name;
		s[e->f_size] = '\0';
		t = s + e->f_size + 4; /* ' -> ' */
		if (link(t, s) == -1) {
			if (errno == EACCES) {
				parent = dir_parent(e->f_name);
				st = dir_write(parent);
				if (link(t, s) == -1) {
					msg(_("Failed to create hardlink `%s -> %s\': %s"),
							s, t, strerror(errno));
					dir_restore(parent, st);
					g_free(parent);
					return FALSE;
				}
				dir_restore(parent, st);
				g_free(parent);
				return TRUE;
			} else {
				msg(_("Failed to create hardlink `%s -> %s\': %s"),
						s, t, strerror(errno));
				return FALSE;
			}
		}
	}
	return TRUE;
}
