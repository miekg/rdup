#include "rdump.h"


GSList *
dir_crawl(char *path)
{
	DIR 		*dir;
	struct dirent 	*dent;
	char 		*pop;
	char 		*curpath;
	struct stat   	s;
	dev_t 		current_dev;
	GSList  	*list;

	list = NULL;
	/* dir stack */
	gint32 d = 0;
	gint32 dstack_size = 500; /* realloc when hit */
	gint32 dstack_cnt  = 1;
	char **dirstack = g_malloc(dstack_cnt * dstack_size * sizeof(char*));

	/* file stack */
	gint32 f = 0;
	gint32 fstack_size = 2500; 
	gint32 fstack_cnt  = 1;
	char **filestack = g_malloc(fstack_cnt * fstack_size * sizeof(char *));

	if(!(dir = opendir(path))) {
		fprintf(stderr, "Cannot enter the directory: %s", path);
		return NULL;
	}

	/* get device */
	if (stat(path, &s) != 0) {
		fprintf(stderr, "Cannot determine holding device of the directory: %s", path);
		return NULL;
	}
	current_dev = s.st_dev;

	while((dent = readdir(dir))) {
		if(!g_ascii_strcasecmp(dent->d_name, ".") || 
				!g_ascii_strcasecmp(dent->d_name, ".."))
			continue;

		curpath = g_strdup_printf("%s/%s", path, dent->d_name);
		
		/* we're statting the file */
		if(lstat(curpath, &s) != 0) {
			fprintf(stderr, "Could not stat path: %s", curpath);
			continue;
		}

		/* catch everything, except dirs */
		if (!S_ISDIR(s.st_mode)) {
			if (!g_ascii_strcasecmp(dent->d_name, NOBACKUP)) {
				fprintf(stderr, "** " NOBACKUP "\n");
				g_free(dirstack);
				g_free(filestack);
				return NULL;
			}
			
			filestack[f++] = g_strdup(curpath);
			if (f % fstack_size == 0) {
				filestack = g_realloc(filestack, 
						++fstack_cnt * fstack_size * sizeof(char *));
			}
			continue;
			
		} else if(S_ISDIR(s.st_mode)) {
			/* one filesystem */
			if (s.st_dev != current_dev) {
				fprintf(stderr, "Walking onto different filesystem");
				continue;
			}

			dirstack[d++] = g_strdup(curpath);
			if (d % dstack_size == 0) {
				dirstack = g_realloc(dirstack, 
						++dstack_cnt * dstack_size * sizeof(char *));
			}
			continue;
		} else {
			fprintf(stderr, "Neither file nor directory: %s", curpath);
		}
	}
	closedir(dir);

	/* we use prepend, but we want dir to be first,
	 * so do them last
	 */
	while (f > 0) {
		pop = filestack[--f];
		list = g_slist_prepend(list, (gpointer*) pop);
	}
	while (d > 0) {
		pop = dirstack[--d]; 
		list = g_slist_prepend(list, (gpointer*) pop);
		list = g_slist_concat(list, dir_crawl(pop));
	}
	
	g_free(dirstack);
	g_free(filestack);
	return list;
}
