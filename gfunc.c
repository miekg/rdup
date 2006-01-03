/*
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 *
 * g_tree_foreach helper functions 
 */

#include "rdup.h"

extern int opt_null;
extern time_t opt_timestamp;

/**
 * print a struct entry
 */
void
entry_print(FILE *fp, char plusmin, struct entry *e) {
	fprintf(fp, "%c%d %d %d %d %s",
			plusmin,
			(int) e->f_mode,
			(int) e->f_uid,
			(int) e->f_gid,
			(int) e->f_size,
			e->f_name);
}


/**
 * free a struct entry
 */
gboolean 
gfunc_free(gpointer data, __attribute__((unused)) gpointer value, 
		__attribute__((unused)) gpointer usr)
{
	struct entry *f;
	f = (struct entry*) data;
	
	g_free(f->f_name);
	g_free(f);
	return FALSE;
}

/**
 * Write our internal filelist
 */
gboolean 
gfunc_write(gpointer data, __attribute__((unused)) gpointer value, gpointer fp)
{
	/* mode_path */
	/* this is used to create our filelist */
	fprintf((FILE*) fp, "%d %s", 
			(int) ((struct entry*)data)->f_mode,
			(char*) ((struct entry*)data)->f_name);
	if (opt_null) {
		putc('\0', fp);
	} else {
		putc('\n', fp);
	}
	return FALSE;
}

#if 0
/**
 * debug function, write a struct entry to fp
 */
gboolean
gfunc_write_all(gpointer data, __attribute__((unused)) gpointer value, gpointer fp)
{
	fprintf((FILE*) fp, "%s ", (char*) ((struct entry*)data)->f_name);
	fprintf((FILE*) fp, "   %d ", (int) ((struct entry*)data)->f_uid);
	fprintf((FILE*) fp, "   %d  ", (int) ((struct entry*)data)->f_gid);
	fprintf((FILE*) fp, "   %d ", (int) ((struct entry*)data)->f_mode);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_mtime);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_size);
	return FALSE;
}
#endif

/**
 * write out the list of to be backupped items
 */
gboolean
gfunc_backup(gpointer data, __attribute__((unused)) gpointer value, 
		__attribute__((unused)) gpointer usr)
{
	if (S_ISDIR(((struct entry*)data)->f_mode)) {
		entry_print(stdout, '+', (struct entry*)data);
		if (opt_null) {
			putc('\0', stdout);
		} else {
			putc('\n', stdout);
		}
		return FALSE;
	} 
	if (S_ISREG(((struct entry*)data)->f_mode) ||
			S_ISLNK(((struct entry*)data)->f_mode)) {
		switch (opt_timestamp) {
			case NULL_DUMP:
				entry_print(stdout, '+', (struct entry*)data);
				if (opt_null) {
					putc('\0', stdout);
				} else {
					putc('\n', stdout);
				}
				return FALSE;
			default: /* INC_DUMP */
				if (((struct entry*)data)->f_mtime > opt_timestamp) {
					entry_print(stdout, '+', (struct entry*)data);
					if (opt_null) {
						putc('\0', stdout);
					} else {
						putc('\n', stdout);
					}
				}
				return FALSE;
		}
	}
	return FALSE;
}

/**
 * write out the list of removed items
 */
gboolean
gfunc_remove(gpointer data, __attribute__((unused)) gpointer value, 
		__attribute__((unused)) gpointer usr)
{
	entry_print(stdout, '-', (struct entry*)data);
	if (opt_null) {
		putc('\0', stdout);
	} else {
		putc('\n', stdout);
	}
	return FALSE;
}

/**
 * decide whether 2 struct entries are equal or not
 */
gint
gfunc_equal(gconstpointer a, gconstpointer b)
{
	gint e;

	e = strcmp(((struct entry*)a)->f_name,
			((struct entry*)b)->f_name);

	if (e == 0) {
		if (((struct entry*)a)->f_mode == 
				((struct entry*)b)->f_mode) {
			return 0;
		}
	}
	return e;
}

/**
 * traverse function
 * implement the tree substraction
 * everything in A, but NOT in b
 * (data := element out of A)
 * (b    := packed in diff, diff->b)
 */
gboolean
gfunc_substract(gpointer data, gpointer value, gpointer diff)
{
	gpointer v;
	v = g_tree_lookup((GTree*)((struct substract*)diff)->b, data);

	if (!v) {
		g_tree_replace(((struct substract*)diff)->d, data, value);
	}
	return FALSE;
}
