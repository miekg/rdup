/*
 * Copyright (c) 2009 - 2011 Miek Gieben
 * License: GPLv3(+), see LICENSE for details
 * Parse username:group helper files
 */

#include "rdup.h"

/* signal.c */
void got_sig(int signal);

/* write a chown helper file */
void
chown_write(gchar * dir, gchar * base, uid_t u, gchar * user, gid_t g,
	    gchar * group)
{
	FILE *f;
	gchar *path;
	if (base == NULL) {
		/* a directory, i.e. /tmp/._rdup_. */
		path = g_strdup_printf("%s/%s", dir, USRGRPINFO);
	} else {
		path = g_strdup_printf("%s/%s%s", dir, USRGRPINFO, base);
	}
	if (!(f = fopen(path, "w"))) {
		/* no file found or failure to open */
		g_free(path);
		return;
	}
	g_free(path);
	fprintf(f, "%s:%d/%s:%d\n", user, u, group, g);
	fclose(f);
}

/* parse the chown help file */
struct chown_pack *chown_parse(gchar * dir, gchar * base)
{
	FILE *f;
	gchar *path;
	if (base == NULL) {
		/* a directory */
		path = g_strdup_printf("%s/%s", dir, USRGRPINFO);
	} else {
		path = g_strdup_printf("%s/%s%s", dir, USRGRPINFO, base);
	}
	if (!(f = fopen(path, "r"))) {
		/* no file found or failure to open */
		g_free(path);
		return NULL;
	}
	long int u, g;
	gchar *user = g_malloc(17);
	gchar *group = g_malloc(17);
	struct chown_pack *cp = g_malloc(sizeof(struct chown_pack));
	if (fscanf(f, "%16[^:]:%ld/%16[^:]:%ld\n", user, &u, group, &g) != 4) {
		/* failure to parse the file */
		return NULL;
	}
	cp->u = (uid_t) u;
	cp->g = (gid_t) g;
	cp->user = user;
	cp->group = group;
	return cp;
}
