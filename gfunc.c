/*
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 */

#include "rdump.h"

/* g_slist_foreach helper functions */

extern int dumptype;
extern int opt_null;
extern time_t list_mtime;

/**
 * free a struct entry
 */
void 
gfunc_free(gpointer data, __attribute__((unused)) gpointer usr)
{
	struct entry *f;
	f = (struct entry*) data;
	
	g_free(f->f_name);
	g_free(f);
}

/**
 * Write our internal filelist
 */
void 
gfunc_write(gpointer data, gpointer fp)
{
	/* mode_path */
	/* this is used to create our filelist, we cannot parse
	 * that back in, is its null delimited, so don't do that
	 */
	fprintf((FILE*) fp, "%d %s", 
			(int) ((struct entry*)data)->f_mode,
			(char*) ((struct entry*)data)->f_name);
	putc('\n', (FILE*) fp);
}

#ifndef NDEBUG
/**
 * debug function, write a struct entry to fp
 */
void
gfunc_write_all(gpointer data, gpointer fp)
{
	fprintf((FILE*) fp, "%s\n", (char*) ((struct entry*)data)->f_name);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_uid);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_gid);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_mtime);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_mode);
}
#endif

/**
 * write out the list of to be backupped items
 */
void
gfunc_backup(gpointer data, __attribute__((unused)) gpointer usr)
{
	char *p;
	p = ((struct entry*)data)->f_name;

	if (S_ISDIR(((struct entry*)data)->f_mode)) {
		/* directory, print: +d_uid_gid_mode_path (_ = space) */
		fprintf(stdout, "+%d %d %d %s", 
				(int) ((struct entry*)data)->f_mode,
				(int) ((struct entry*)data)->f_uid,
				(int) ((struct entry*)data)->f_gid,
				p);
		if (opt_null) {
			putc('\0', stdout);
		} else {
			putc('\n', stdout);
		}
		return;
	} 
	if (S_ISREG(((struct entry*)data)->f_mode) ||
			S_ISLNK(((struct entry*)data)->f_mode)) {
		/* file, print: +f_uid_gid_mode_path  (_ = space) */
		switch (dumptype) {
			case NULL_DUMP:
				fprintf(stdout, "+%d %d %d %s", 
						(int) ((struct entry*)data)->f_mode,
						(int) ((struct entry*)data)->f_uid,
						(int) ((struct entry*)data)->f_gid,
						p);
				if (opt_null) {
					putc('\0', stdout);
				} else {
					putc('\n', stdout);
				}
				return;
			case INC_DUMP:
				if (((struct entry*)data)->f_mtime > list_mtime) {
					fprintf(stdout, "+%d %d %d %s", 
							(int) ((struct entry*)data)->f_mode,
							(int) ((struct entry*)data)->f_uid,
							(int) ((struct entry*)data)->f_gid,
							p);
					if (opt_null) {
						putc('\0', stdout);
					} else {
						putc('\n', stdout);
					}
				}
				return;
		}
	}
	return;
}

/**
 * write out the list of removed items
 */
void
gfunc_remove(gpointer data, __attribute__((unused)) gpointer usr)
{
	char *p;
	p = ((struct entry*)data)->f_name;
	fprintf(stdout, "-%d %d %d %s", 
			(int) ((struct entry*)data)->f_mode,
			(int) ((struct entry*)data)->f_uid,
			(int) ((struct entry*)data)->f_gid,
			p);
	if (opt_null) {
		putc('\0', stdout);
	} else {
		putc('\n', stdout);
	}
	return;
}

/**
 * decide whether 2 struct entries are equal 
 * or not
 */
gint
gfunc_equal(gconstpointer a, gconstpointer b)
{
	if (g_str_equal(((struct entry*)a)->f_name, 
				((struct entry*)b)->f_name)) {
		if (((struct entry*)a)->f_mode == 
				((struct entry*)b)->f_mode) {
			return 0;
		}
	}
	return 1;
}
