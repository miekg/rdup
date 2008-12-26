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

extern sig_atomic_t sig;
extern gboolean opt_dry;
extern gint opt_verbose;
extern GSList *hlink;

/* signal.c */
void got_sig(int signal);

gboolean
mk_dev(struct r_entry *e, gboolean exists) {
	if (opt_dry)
		return TRUE;

	if (exists) {
		if (!rm(e->f_name)) {
			msg("Failed to remove existing entry: '%s\'", e->f_name);
			return FALSE;
		}
	}

	if (mknod(e->f_name, e->f_mode, e->f_rdev) == -1) {
		msg("Failed to make device: `%s\': %s", e->f_name, strerror(errno));
	}
	g_chmod(e->f_name, e->f_mode);
	return TRUE;
}

gboolean
mk_sock(struct r_entry *e, gboolean exists) {
	if (opt_dry)
		return TRUE;

	if (exists) {
		if (!rm(e->f_name)) {
			msg("Failed to remove existing entry: '%s\'", e->f_name);
			return FALSE;
		}
	}

	if (mkfifo(e->f_name, e->f_mode) == -1) {
		msg("Failed to make socket: `%s\': %s", e->f_name, strerror(errno));
	}
	g_chmod(e->f_name, e->f_mode);
	return TRUE;
}


gboolean
mk_link(struct r_entry *e, gboolean exists, char *s, char *t, char *p)
{
	if (opt_dry)
		return TRUE;

	/* there is something */
	if (exists) {
		if (!rm(s)) {
			msg("Failed to remove existing entry: '%s\'", s);
			return FALSE;
		}
	}

	/* symlink */
	if (S_ISLNK(e->f_mode)) {
		if (symlink(t, s) == -1) {
			msg("Failed to make symlink: `%s -> %s\': %s", s, t, strerror(errno));
			return FALSE;
		}
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

gboolean
mk_reg(FILE *in, struct r_entry *e, gboolean exists)
{
	FILE *out;
	char *buf;
	size_t   i, j, mod, rest;
	gboolean ok = TRUE;

	/* there is something */
	if (exists && !opt_dry)  {
		if (!rm(e->f_name)) {
			msg("Failed to remove existing entry: '%s\'", e->f_name);
			return FALSE;
		}
	}

	if (!(out = fopen(e->f_name, "w"))) {
		msg("Failed to open file `%s\': %s", e->f_name, strerror(errno));
		ok = FALSE;
	} else {
		/* set permissions right away */
		g_chmod(e->f_name, e->f_mode);
	}

	buf   = g_malloc(BUFSIZE + 1);
	rest = e->f_size % BUFSIZE;	      /* then we need to read this many */
	mod  = (e->f_size - rest) / BUFSIZE;  /* main loop happens mod times */

	/* mod loop */
	for(j = 0; j < mod; j++) {
		i = fread(buf, sizeof(char), BUFSIZE, in);
		if (ok && !opt_dry) {
			if (fwrite(buf, sizeof(char), i, out) != i) {
				msg(_("Write failure `%s\': %s"), e->f_name, strerror(errno));
				fclose(out);
				return FALSE;
			}
		}
	}
	/* rest */
	i = fread(buf, sizeof(char), rest, in);
	if (ok && !opt_dry) {
		if (fwrite(buf, sizeof(char), i, out) != i) {
			msg(_("Write failure `%s\': %s"), e->f_name, strerror(errno));
			fclose(out);
			return FALSE;
		}
	}
	return TRUE;
}

gboolean
mk_dir(struct r_entry *e, struct stat *st, gboolean exists) 
{
	if (opt_dry)
		return TRUE;

	/* there is something and it's a dir, update permissions */
	/* if we don't have write permission to this directory, we
	 * might fail to update any files in it, so check for this */
	if (exists && S_ISDIR(st->st_mode)) {
		if (access(e->f_name, W_OK | X_OK) == -1) {
			/* unable to access this dir IF there any
			 * files placed in this directory,
			 * so set permissive bits and correct that
			 * later on
			 */
		}
		/* which permissions to set? */




		g_chmod(e->f_name, e->f_mode);
		return TRUE;
	}

	if (g_mkdir(e->f_name, e->f_mode) == -1) {
		msg("Failed to created directory `%s\'", e->f_name);
		return FALSE;
	}
	g_chmod(e->f_name, e->f_mode);
	return TRUE;
}


/* make an object in the filesystem */
gboolean
mk_obj(FILE *in, char *p, struct r_entry *e, guint strip) 
{
	char     *s, *t;
	gboolean exists;
	struct stat st;

	/* XXX*/
	strip = strip;

	if (lstat(e->f_name, &st) == -1) 
		exists = FALSE;
	else
		exists = TRUE;

	if (opt_verbose > 0)
		printf("%s\n", e->f_name);

	switch(e->plusmin) {
		case '-':
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
		case '+':
			/* opt_dry handled within the subfunctions */
			if (S_ISDIR(e->f_mode))
				return  mk_dir(e, &st, exists);	

			/* First sym and hardlinks and then * a regular file */
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
	for (p = g_slist_nth(h, 0); p; p = p->next) { 
		e = (struct r_entry *)p->data;

		s = e->f_name;
		s[e->f_size] = '\0';
		t = s + e->f_size + 4; /* ' -> ' */
		if (link(t, s) == -1) {
			msg("Failed to create hardlink `%s -> %s\': %s",
					s, t, strerror(errno));
			return FALSE;
		}
	}
	return TRUE;
}
