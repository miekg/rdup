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

	/* er gaat hier iets mis met symlinks  ... */
	fprintf(stderr, "rm [%s]\n", p);

	if (g_file_test(p, G_FILE_TEST_IS_SYMLINK)) {
		if (g_remove(p) == -1) {
			msg("Failed to remove `%s\': %s", p, "errno");
			return FALSE;
		}
	}

	/* do this again (race conditions) */
	if (!g_file_test(p, G_FILE_TEST_EXISTS)) 
		return TRUE;		/* heh :), it's already gone */

	if (g_file_test(p, G_FILE_TEST_IS_DIR)) {
		ret = g_remove(p);
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

					rm(dirp);
					/* a) uhh, error checking */
					/* b) max diepte */
				}
				g_dir_close(d);
				return TRUE;
			} else {
				msg("Failed to remove directory `%s\': %s", p, "errno");
				return FALSE;
			}
		}
	} else {
		if (g_remove(p) == -1) {
			msg("Failed to remove `%s\': %s", p, "errno");
			return FALSE;
		}
		return TRUE;
	}

	return TRUE;
}

