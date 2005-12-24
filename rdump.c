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

/* prototypes */
GSList * dir_crawl(char *path);
void gfunc_write(gpointer data, gpointer fp);
void gfunc_write2(gpointer data, gpointer fp);
void gfunc_backup(gpointer data, gpointer usr);
void gfunc_remove(gpointer data, gpointer usr);
gint gfunc_str_equal(gconstpointer a, gconstpointer b);


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
	char 	*dirpath;

	curlist = NULL;
	backup = NULL;
	remove = NULL;
	dirpath = NULL;

	dumptype = INC_DUMP;

	list_mtime = mtime("FILELIST");

	if (argc < 2) {
		dirpath = "bin";
	}
	
	if (!(fplist = fopen("FILELIST", "a+"))) {
		fprintf(stderr, "Could not open file\n");
		exit(EXIT_FAILURE);
	} else {
		rewind(fplist);
	}
	curlist = g_slist_read_file(fplist);
	backup = dir_crawl(dirpath);

	remove = g_slist_substract(curlist, backup); 

	g_slist_foreach(backup, gfunc_backup, NULL);
	g_slist_foreach(remove, gfunc_remove, NULL); 

	/* write new filelist */
	ftruncate(fileno(fplist), 0);  
	g_slist_foreach(backup, gfunc_write, fplist);
	fclose(fplist); 
	exit(EXIT_SUCCESS);
}

