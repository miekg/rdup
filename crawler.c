#include "rdump.h"

extern int opt_null;
extern int opt_onefilesystem;
extern int opt_nobackup;
extern int opt_verbose;

struct entry *
entry_dup(struct entry *f)
{
        struct entry *g;
        g = g_malloc(sizeof(struct entry));

        g->f_name  = g_strdup(f->f_name);
        g->f_uid   = f->f_uid;
        g->f_gid   = f->f_gid;
        g->f_mode  = f->f_mode;
	g->f_mtime = f->f_mtime;
        return g;
}

void
entry_free(struct entry *f)
{
	g_free(f->f_name);
	g_free(f);
}

GSList *
dir_prepend(GSList *l, char *path)
{
	char *c;
	char *p;
	struct stat s;
	struct entry *e;

	for(p = path + 1; (c = strchr(p, '/')); p++) {
		*c = '\0';
		if(lstat(path, &s) != 0) {
			fprintf(stderr, "** Could not stat dirpath: %s\n", path);
			return NULL;
		}
		e = g_malloc(sizeof(struct entry));
		e->f_name  = g_strdup(path);
		e->f_uid   = s.st_uid;
		e->f_gid   = s.st_gid;
		e->f_mtime = s.st_mtime;
		e->f_mode  = s.st_mode;
		
		l = g_slist_append(l , (gpointer) entry_dup(e));
		g_free(e);
		
		*c = '/';
		p = c++;
	}
	return l;
}

GSList *
dir_crawl(char *path)
{
	DIR 		*dir;
	struct dirent 	*dent;
	struct entry	*pop;
	char 		*curpath;
	struct stat   	s;
	dev_t 		current_dev;
	GSList  	*list;

	list = NULL;

	/* dir stack */
	gint32 d = 0;
	gint32 dstack_size = 100; /* realloc when hit */
	gint32 dstack_cnt  = 1;
	struct entry **dirstack = g_malloc(dstack_cnt * dstack_size * sizeof(struct entry *));

	/* file stack */
	gint32 f = 0;
	gint32 fstack_size = 200; 
	gint32 fstack_cnt  = 1;
	struct entry **filestack = g_malloc(fstack_cnt * fstack_size * sizeof(struct entry *));

	if(!(dir = opendir(path))) {
		fprintf(stderr, "** Cannot enter: %s\n", path);
		g_free(filestack);
		g_free(dirstack);
		return NULL;
	}

	/* get device */
	if (stat(path, &s) != 0) {
		fprintf(stderr, "** Cannot determine holding device of the directory: %s\n", path);
		closedir(dir);
		g_free(filestack);
		g_free(dirstack);
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
			fprintf(stderr, "** Could not stat path: %s\n", curpath);
			g_free(curpath);
			continue;
		}

		/* catch everything, except dirs */
		if (!S_ISDIR(s.st_mode)) {
			if ((opt_nobackup == 1) && !g_ascii_strcasecmp(dent->d_name, NOBACKUP)) {
				if (opt_verbose) {
					fprintf(stderr, "** " NOBACKUP " in %s\n", curpath);
				}
				/* add this to the backup */
				pop = g_malloc(sizeof(struct entry));
				pop->f_name  = g_strdup(curpath);
				pop->f_uid   = s.st_uid;
				pop->f_gid   = s.st_gid;
				pop->f_mtime = s.st_mtime;
				pop->f_mode  = s.st_mode;
				list = NULL; /* leak from here to Tokio */
				list = g_slist_prepend(list, (gpointer) entry_dup(pop));
				g_free(pop);
				g_free(dirstack);
				g_free(filestack);
				g_free(curpath);
				closedir(dir);
				return list;
			}
			
			filestack[f] = g_malloc(sizeof(struct entry));
			filestack[f]->f_name  = g_strdup(curpath);
			filestack[f]->f_uid   = s.st_uid;
			filestack[f]->f_gid   = s.st_gid;
			filestack[f]->f_mtime = s.st_mtime;
			filestack[f]->f_mode  = s.st_mode;

			if (f++ % fstack_size == 0) {
				filestack = g_realloc(filestack, 
						++fstack_cnt * fstack_size * sizeof(struct entry *));
			}
			g_free(curpath);
			continue;
		} else if(S_ISDIR(s.st_mode)) {
			/* one filesystem */
			if (opt_onefilesystem && s.st_dev != current_dev) {
				fprintf(stderr, "** Walking onto different filesystem\n");
				g_free(curpath);
				continue;
			}

			dirstack[d] = g_malloc(sizeof(struct entry));
			dirstack[d]->f_name  = g_strdup(curpath); 
			dirstack[d]->f_uid   = s.st_uid;
			dirstack[d]->f_gid   = s.st_gid;
			dirstack[d]->f_mtime = s.st_mtime;
			dirstack[d]->f_mode  = s.st_mode;

			if (d++ % dstack_size == 0) {
				dirstack = g_realloc(dirstack, 
						++dstack_cnt * dstack_size * sizeof(struct entry *));
			}
			g_free(curpath);
			continue;
		} else {
			fprintf(stderr, "** Neither file nor directory: %s\n", curpath);
			g_free(curpath);
		}
	}
	closedir(dir);

	/* we use prepend, but we want dir to be first,
	 * so do them last
	 */
	while (f > 0) {
		pop = filestack[--f];
		list = g_slist_prepend(list, (gpointer) entry_dup(pop));
		entry_free(pop);
	}
	while (d > 0) {
		pop = dirstack[--d]; 
		list = g_slist_prepend(list, (gpointer) entry_dup(pop));
		/* recurse */
		list = g_slist_concat(list, dir_crawl(pop->f_name));
		entry_free(pop);
	}
	
	g_free(dirstack);
	g_free(filestack);
	return list;
}
