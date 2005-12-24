#include "rdump.h"


GSList * dir_crawl(char *path);


void 
print(gpointer *data)
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
	char buf[BUFSIZE + 1];
	GSList *list;

	list = NULL;
	while (!(fgets(buf, BUFSIZE, fp))) {
		list = g_slist_prepend(list,
				(gpointer*) g_strdup(buf));
	}
	return list;
}


int 
main(int argc, char **argv) 
{
	GSList *diskfiles; /* on disk stuff */
	GSList *listfiles; /* previous backup list */
	FILE *fplist;

	diskfiles = NULL;
	listfiles = NULL;
	
	if (!(fplist = fopen("FILELIST", "rw"))) {
		fprintf(stderr, "Could not open file\n");
	} else {
		listfiles = g_slist_read_file(fplist);
	}
	
	diskfiles = dir_crawl(argv[1]);

	g_slist_foreach(diskfiles, print, NULL);
	
	printf("\n");
	exit(EXIT_SUCCESS);
}

