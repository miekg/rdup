/*
 * Copyright (c) 2009 - 2011 Miek Gieben
 * See LICENSE for the license
 * rm.c remove an fs object (recursively)
 */

#include "rdup-up.h"

extern gboolean opt_dry;
extern gboolean opt_verbose;

/* ENOENT */
/* errno */

gboolean rm(gchar * p)
{
	int ret;
	gchar *dirp, *q;
	gchar *parent;
	GDir *d;
	struct stat st;
	struct stat *st2;

	if (opt_dry || !p)
		return TRUE;	/* noop */

	if (lstat(p, &st) == -1) {
		if (opt_verbose > 0 && errno != ENOENT)
			msgd(__func__, __LINE__,
			     _("Failed to remove `%s\': %s"), p,
			     strerror(errno));
		return TRUE;	/* noop */
	}

	if (S_ISDIR(st.st_mode)) {
		ret = remove(p);
		if (ret == -1) {
			switch (errno) {
			case ENOTEMPTY:
				/* recursive into this dir and do our bidding */
				if (!(d = g_dir_open(p, 0, NULL))) {
					msgd(__func__, __LINE__,
					     _
					     ("Failed to open directory `%s\': %s"),
					     p, strerror(errno));
					return FALSE;
				}
				while ((dirp = (gchar *) g_dir_read_name(d))) {
					dirp =
					    g_strdup_printf("%s/%s", p, dirp);
					rm(dirp);
					g_free(dirp);
				}
				g_dir_close(d);
				/* dir should be empty by now */
				if ((ret = remove(p)) == -1)
					msgd(__func__, __LINE__,
					     _
					     ("Failed to remove directory `%s\': %s"),
					     p, strerror(errno));
				return TRUE;

			case EACCES:
				/* no write to dir, make writable */
				parent = dir_parent(p);
				st2 = dir_write(parent);
				if (remove(p) == -1) {
					msgd(__func__, __LINE__,
					     _("Failed to remove `%s\': %s"), p,
					     strerror(errno));
					dir_restore(parent, st2);
					g_free(parent);
					return FALSE;
				}
				dir_restore(parent, st2);
				g_free(parent);
				return TRUE;

			default:
				/* not ENOEMPTY */
				msgd(__func__, __LINE__,
				     _("Failed to remove directory `%s\': %s"),
				     p, strerror(errno));
				return FALSE;
			}
		}
		return TRUE;
	}

	/* dirs are now handled */

	if (remove(p) == -1) {
		switch (errno) {
		case EACCES:
			/* we have no access, ok ... */
			q = g_strdup(p);
			parent = dirname(q);
			st2 = dir_write(parent);
			if (remove(p) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to remove `%s\': %s"), p,
				     strerror(errno));
				dir_restore(parent, st2);
				g_free(q);
				return FALSE;
			}
			dir_restore(parent, st2);
			g_free(q);
			return TRUE;

		case EPERM:
			/* no write on file, reuse st - and why is this needed again? */
			/* this is dead code ... */
			stat(p, &st);
			chmod(p, st.st_mode | S_IWUSR);
			if (remove(p) == -1) {
				msgd(__func__, __LINE__,
				     _("Failed to remove `%s\': %s"), p,
				     strerror(errno));
				chmod(p, st.st_mode);	/* is this usefull then? */
				return FALSE;
			}
			return TRUE;
		}

		msgd(__func__, __LINE__, _("Failed to remove `%s\': %s"), p,
		     strerror(errno));
		return FALSE;
	}
	return TRUE;
}
