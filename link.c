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
hardlink(GHashTable *t, struct r_entry *e)
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
