/*
 * Copyright (c) 2005-2010 Miek Gieben
 * See LICENSE for the license
 * Reverse a GTree by putting all element in a GList
 * and reverse print that. Needed for rdup's -R switch
 */

#include "rdup.h"

/* Let's make it global... *sigh* */
GList *list;

/* Walk each element and put it in a list */
GList *
reverse(GTree *g)
{
	list = NULL;
	g_tree_foreach(g, gfunc_tree2list, NULL);
	return list;
}

/**
 * wrappers that call gfunc_xxx for GLists
 * In the conversion function (gfunc_tree2list) we
 * skip all NO_PRINT entries, hence to harcoded
 * used of VALUE here.
 */
void
gfunc_new_list(gpointer data, gpointer userdata)
{
	(void)gfunc_new(data, VALUE, userdata);
}

void
gfunc_remove_list(gpointer data, gpointer userdata)
{
	(void)gfunc_remove(data, VALUE, userdata);
}

void
gfunc_backup_list(gpointer data, gpointer userdata)
{
	(void)gfunc_backup(data, VALUE, userdata);
}
