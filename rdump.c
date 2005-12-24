#include "rdump.h"

/* TODO:
 * we statting to much, once in the crawler
 * and now here. Don't optimize yet
 *
 * print out uid guid and perm make bu use that
 * info too.
 */

/* cmd options */
int opt_null = 0;
int opt_onefilesystem = 0;
int opt_nobackup = 0;
int opt_verbose = 0;

int dumptype;
time_t list_mtime;

/* prototypes */
GSList * dir_crawl(char *path);
void gfunc_write(gpointer data, gpointer fp);
void gfunc_write2(gpointer data, gpointer fp);
void gfunc_backup(gpointer data, gpointer usr);
void gfunc_remove(gpointer data, gpointer usr);
gint gfunc_str_equal(gconstpointer a, gconstpointer b);


void
usage(FILE *f, char *p) 
{	
	fprintf(f, "Usage: %s [OPTION...] FILELIST DIR...\n", p);
	fprintf(f, "%s generates a full or incremental file list, this\n", p);
	fprintf(f, "list can be used to implement a incremental backup scheme\n");
	fprintf(f, "\n   FILELIST\tincremental file list\n");
	fprintf(f, "   \t\tif not found or empty, a full dump is done\n");
	fprintf(f, "   DIR\t\tdirectory or directories to dump\n");
	fprintf(f, "\nOptions:\n");
	fprintf(f, "   -h\t\tgives this help\n");
	fprintf(f, "   -n\t\tdo not look at" NOBACKUP "files\n");
	fprintf(f, "   -v\t\tbe more verbose\n");
	fprintf(f, "   -x\t\tstay in local file system\n");
	fprintf(f, "   -0\t\tdelimit all output with NULLs\n");
	fprintf(f, "\nReport bugs to <miek@miek.nl>\n");
}

/**
 * subtrace list *b from list *a, leaving
 * the elements that are only in *a. Essentially
 * a double diff: A diff (A diff B)
 */
GSList *
g_slist_substract(GSList *a, GSList *b)
{
	GSList 		*diff;
	guint 		i;
	gpointer 	data;

	diff = NULL;

	/* everything in a, but NOT in b */
	for(i = 0; i < g_slist_length(a); i++) {
		data = g_slist_nth_data(a, i);

		if (!g_slist_find_custom(b, data, gfunc_str_equal)) {
			diff = g_slist_append(diff, data);
		}
	}
	return diff;
}

GSList *
g_slist_read_file(FILE *fp)
{
	char 	buf[BUFSIZE + 1];
	GSList 	*list;

	list = NULL;
	while ((fgets(buf, BUFSIZE, fp))) {
		/* chop annoying newline off */
		buf[strlen(buf) - 1] = '\0';
		list = g_slist_append(list,
				(gpointer) g_strdup(buf));
	}
	return list;
}

time_t
mtime(char *f)
{
	struct stat s;

	if (lstat(f, &s) != 0) {
		return 0;
	}
	return s.st_mtime;
}

int 
main(int argc, __attribute__((unused)) char **argv) 
{
	GSList 	*backup; 	/* on disk stuff */
	GSList 	*remove;	/* what needs to be rm'd */
	GSList 	*curlist; 	/* previous backup list */
	FILE 	*fplist;
	char 	*progname;
	gint    i;
	int 	c;

	curlist = NULL;
	backup = NULL;
	remove = NULL;
	progname = g_strdup(argv[0]);
	opterr = 0;

	while ((c = getopt (argc, argv, "hnvx0")) != -1) {
		switch (c)
		{
			case 'h':
				usage(stdout, progname);
				exit(EXIT_SUCCESS);
			case 'n':
				opt_nobackup = 1;
				break;
			case 'v':
				opt_verbose = 1;
				break;
			case 'x':
				opt_onefilesystem = 1;
				break;
			case '0':
				opt_null = 1;
				break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2) {
		usage(stdout, progname);
		exit(EXIT_FAILURE);
	}

	/* Check for full of incremental dump */
	if ((list_mtime = mtime(argv[0])) == 0) {
		dumptype = NULL_DUMP;
	} else {
		dumptype = INC_DUMP;
	}

	if (!(fplist = fopen(argv[0], "a+"))) {
		fprintf(stderr, "Could not open file\n");
		exit(EXIT_FAILURE);
	} else {
		rewind(fplist);
	}
	curlist = g_slist_read_file(fplist);
	for (i = 1; i < argc; i++) {
		backup = g_slist_concat(backup, 
				dir_crawl(argv[i]));
	}

	remove = g_slist_substract(curlist, backup); 

	g_slist_foreach(backup, gfunc_backup, NULL);
	g_slist_foreach(remove, gfunc_remove, NULL); 

	/* write new filelist */
	ftruncate(fileno(fplist), 0);  
	g_slist_foreach(backup, gfunc_write, fplist);
	fclose(fplist); 
	exit(EXIT_SUCCESS);
}

