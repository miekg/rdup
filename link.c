/*
 * Copyright (c) 2005 - 2011 Miek Gieben
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
gchar *hlink(GHashTable * t, struct rdup *e)
{
	gchar *key;
	gchar *name;

	/* use , to make it unique */
	key = g_strdup_printf("%d,%d", (gint) e->f_dev, (gint) e->f_ino);

	if (!(name = g_hash_table_lookup(t, (gpointer) key))) {
		g_hash_table_insert(t, key, g_strdup(e->f_name));
	}
	return g_strdup(name);	/* entry_free will free this */
}

gchar *slink(struct rdup * e)
{
	char buf[BUFSIZE + 1];
	ssize_t i;

	if ((i = readlink(e->f_name, buf, BUFSIZE)) == -1) {
		msg(_("Error reading link `%s\': %s"), e->f_name,
		    strerror(errno));
		return NULL;
	} else {
		buf[i] = '\0';
		return g_strdup(buf);
	}
}
