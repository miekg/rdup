/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rm.c remove an fs object (recursively)
 */

#include "rdup-up.h"

/* ENOENT */
/* errno */

gboolean
rm(gchar *p)
{
	int ret;
	gchar *dirp;
	GDir *d;
	struct stat st;
	struct stat *st2;

	if (lstat(p, &st) == -1)
		return TRUE;    /* the easy life */

	if (S_ISDIR(st.st_mode)) {
		ret = remove(p);
		if (ret == -1) {
			/* hmm, failed... */
			if (errno == ENOTEMPTY) {
				/* recursive into this dir and do our bidding */
				if (!(d = g_dir_open(p, 0, NULL))) {
					msg("Failed to open directory `%s\': %s", p, "errno");
					return FALSE;
				}
				while ( (dirp = (gchar*)g_dir_read_name(d))) {
					/* dirp is relative to the current directory */

					/* gaat die ALTIJD goed? */
					dirp = g_strdup_printf("%s%c%s", p, DIR_SEP, dirp);

/* XXX					fprintf(stderr, "removing the next %s\n", dirp); */
					rm(dirp);
					/* a) uhh, error checking */
					/* b) max diepte */
				}
				g_dir_close(d);
				return TRUE;
			} else {
				/* not ENOEMPTY */
				msg("Failed to remove directory `%s\': %s", p, strerror(errno));
				return FALSE;
			}
		}
		ret = remove(p);	/* try to remove the top level dir again */
		return TRUE;
	}

	if (remove(p) == -1) {
		if (errno == EACCES) {
			/* we have no access, ok ... */
			st2 = dir_write(dirname(p));
			if (remove(p) == -1) {
				msg("Still failing to remove `%s\'`: %s", p, strerror(errno));
				dir_restore(dirname(p), st2);
				return FALSE;
			}
			dir_restore(dirname(p), st2);
			return TRUE;
		}
		msg("Failed to remove `%s\': %s", p, strerror(errno));
		return FALSE;
	}
	return TRUE;
}
