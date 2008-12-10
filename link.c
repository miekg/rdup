/*
 * Copyright (c) 2005, 2008 Miek Gieben
 * See LICENSE for the license
 *
 * dev_t,ino_t -> path look up table
 * needed for hardlink implementation
 */

#include "rdup.h"

/**
 * Find if there is a hardlink for this file
 * If found, return it, otherwise add it to
 * the hashtable.
 */
gchar *
hard_link(GHashTable *t, struct r_entry *e)
{
	gchar *key;
	gchar *name;

	/* use , to make it unique */
	key = g_strdup_printf("%d,%d", (gint)e->f_dev, (gint)e->f_ino);

	if ( ! (name = g_hash_table_lookup(t, (gpointer)key))) {
		g_hash_table_insert(t, g_strdup(key), g_strdup(e->f_name));
	}
	g_free(key);
	return name;
}

/* symlinks; also put the -> name in f_name */
struct r_entry *
sym_link(struct r_entry *e, char *h_lnk) 
{
	char buf[BUFSIZE + 1]; 
	ssize_t i;

	if (h_lnk) {			/* hardlink */
		e->f_size = strlen(e->f_name);  /* old name length */
		h_lnk = g_strdup_printf("%s -> %s", e->f_name, h_lnk);
		e->f_lnk = 1;
		e->f_name = h_lnk;
		e->f_name_size = strlen(e->f_name);
	} else {			/* symlink */
		if ((i = readlink(e->f_name, buf, BUFSIZE)) == -1) {
			msg(_("Error reading link `%s\': %s"), e->f_name, strerror(errno));
		} else {
			buf[i] = '\0';
			e->f_size = strlen(e->f_name); /* old name length */
			e->f_name = g_strdup_printf("%s -> %s", e->f_name, buf);
			e->f_name_size = strlen(e->f_name);
		}
	}
	return e;
}
