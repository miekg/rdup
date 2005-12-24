#include "rdump.h"

/* TODO:
 * we statting to much, once in the crawler
 * and now here. Don't optimize yet
 *
 * print out uid guid and perm make bu use that
 * info too.
 */

/* cmd options */
int opt_null;
int opt_onefilesystem;
int opt_nobackup;
int opt_verbose;

int dumptype;
time_t list_mtime;

GSList * dir_crawl(char *path);

void 
gfunc_print(gpointer *data)
{
	fprintf(stdout, "%s\n", (char*) data);
}

/**
 * subtrace list *b from list *a, leaving
 * the elements that are only in *a. Essentially
 * a double diff: A diff (A diff B)
 */
#if 0
GSList *
g_slist_substract(GSList *a, GSList *b)
{
	GSList *diff;

	diff = NULL;

	return diff;
}
#endif

GSList *
g_slist_read_file(FILE *fp)
{
	char 	buf[BUFSIZE + 1];
	GSList 	*list;

	list = NULL;
	while (!(fgets(buf, BUFSIZE, fp))) {
		list = g_slist_prepend(list,
				(gpointer*) g_strdup(buf));
	}
	return list;
}

void
gfunc_backup(gpointer *data)
{
	struct stat s;

	if (lstat((char*)data, &s) != 0) {
		fprintf(stderr, "could not stat\n");
		return;
	}
	
	if (S_ISDIR(s.st_mode)) {
		/* directory: print +/-dpath */
		fprintf(stdout, "+d%s", (char*)data);
		putc('\n', stdout);
		return;
	} 
	if (S_ISREG(s.st_mode)) {
		/* file: print +/- path */
		switch (dumptype) {
			case NULL_DUMP:
				fprintf(stdout, "+ %s", (char*)data);
				putc('\n', stdout);
				return;
			case INC_DUMP:
				if (s.st_mtime > list_mtime) {
					fprintf(stdout, "+ %s", (char*)data);
					putc('\n', stdout);
				}
				return;
		}
	}
	/* neither */
	return;
}

void
gfunc_remove(gpointer *data)
{
	struct stat s;

	if (lstat((char*)data, &s) != 0) {
		fprintf(stderr, "could not stat\n");
		return;
	}
	
	if (S_ISDIR(s.st_mode)) {
		/* directory: print +/-dpath */
		fprintf(stdout, "-d%s", (char*)data);
		putc('\n', stdout);
		return;
	} 
	if (S_ISREG(s.st_mode)) {
		fprintf(stdout, "- %s", (char*)data);
		putc('\n', stdout);
		return;
	}
	/* neither */
	return;
}


int 
main(int argc, char **argv) 
{
	GSList 	*backup; 	/* on disk stuff */
	GSList 	*remove;	/* what needs to be rm'd */
	GSList 	*curlist; 	/* previous backup list */
	FILE 	*fplist;
	char 	*dirpath;

	curlist = NULL;
	backup = NULL;
	remove = NULL;
	dirpath = NULL;

	if (argc < 2) {
		dirpath = "bin";
	}
	
	if (!(fplist = fopen("FILELIST", "rw"))) {
		fprintf(stderr, "Could not open file\n");
		dumptype = NULL_DUMP;
	} else {
		curlist = g_slist_read_file(fplist);
		dumptype = INC_DUMP;
	}
	
	backup = dir_crawl(dirpath);
/*	remove = g_slist_substract(curlist, backup); */

	g_slist_foreach(backup, gfunc_backup, NULL);
/*	g_slist_foreach(remove, gfunc_remove, NULL); */

	/* write new filelist */
	
	exit(EXIT_SUCCESS);
}

