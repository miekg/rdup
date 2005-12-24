#include "rdump.h"

/* g_slist_foreach helper functions */

extern int dumptype;
extern time_t list_mtime;

void 
gfunc_write(gpointer data, gpointer fp)
{
	/* mode_path */
	fprintf((FILE*) fp, "%d %s", 
			(int) ((struct entry*)data)->f_mode,
			(char*) ((struct entry*)data)->f_name);
	putc('\n', (FILE*) fp);
}

void
gfunc_write_all(gpointer data, gpointer fp)
{
	fprintf((FILE*) fp, "%s\n", (char*) ((struct entry*)data)->f_name);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_uid);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_gid);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_mtime);
	fprintf((FILE*) fp, "   %d\n", (int) ((struct entry*)data)->f_mode);
}

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
		putc('\n', stdout);
		return;
	} 
	if (S_ISREG(((struct entry*)data)->f_mode)) {
		/* file, print: +f_uid_gid_mode_path  (_ = space) */
		switch (dumptype) {
			case NULL_DUMP:
				fprintf(stdout, "+%d %d %d %s", 
						(int) ((struct entry*)data)->f_mode,
						(int) ((struct entry*)data)->f_uid,
						(int) ((struct entry*)data)->f_gid,
						p);
				putc('\n', stdout);
				return;
			case INC_DUMP:
				if (((struct entry*)data)->f_mtime > list_mtime) {
					fprintf(stdout, "+%d %d %d %s", 
							(int) ((struct entry*)data)->f_mode,
							(int) ((struct entry*)data)->f_uid,
							(int) ((struct entry*)data)->f_gid,
							p);
					putc('\n', stdout);
				}
				return;
		}
	}
	return;
}

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
	putc('\n', stdout);
	return;
}

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
