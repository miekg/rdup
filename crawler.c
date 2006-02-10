#include "rdup.h"

extern int opt_null;
extern int opt_onefilesystem;
extern int opt_nobackup;
extern int opt_verbose;

static struct entry *
entry_dup(struct entry *f)
{
        struct entry *g;
        g = g_malloc(sizeof(struct entry));

        g->f_name       = g_strdup(f->f_name);
        g->f_name_size  = f->f_name_size;
        g->f_uid        = f->f_uid;
        g->f_gid        = f->f_gid;
        g->f_mode       = f->f_mode;
	g->f_mtime      = f->f_mtime;
	g->f_size       = f->f_size;
        return g;
}

static void
entry_free(struct entry *f)
{
	g_free(f->f_name);
	g_free(f);
}

/**
 * prepend path leading up to backup directory to the tree
 */
gboolean
dir_prepend(GTree *t, char *path)
{
	char *c;
	char *p;
	char *path2;
	size_t len;
	struct stat s;
	struct entry *e;

	path2 = g_strdup(path);
	len   = strlen(path);

	/* add closing / */
	if (path2[len - 1] != '/') {
		path2 = g_realloc(path2, len + 2);
		path2[len] = '/';
		path2[len + 1] = '\0';
	}

	for(p = path2 + 1; (c = strchr(p, DIR_SEP)); p++) {
		*c = '\0';
		if(lstat(path2, &s) != 0) {
			fprintf(stderr, "** Could not stat dirpath: %s\n", path2);
			return FALSE;
		}
		e = g_malloc(sizeof(struct entry));
		e->f_name      = path2;
		e->f_name_size = strlen(path2);
		e->f_uid       = s.st_uid;
		e->f_gid       = s.st_gid;
		e->f_mtime     = s.st_mtime;
		e->f_mode      = s.st_mode;
		e->f_size      = s.st_size;
		
		/* leak; need destroy function for old value */
		g_tree_replace(t, (gpointer) entry_dup(e), VALUE);
		g_free(e);
		
		*c = '/';
		p = c++;
	}
	g_free(path2);
	return TRUE;
}

gboolean
dir_crawl(GTree *t, char *path)
{
	DIR 		*dir;
	struct dirent 	*dent;
	struct entry	*pop;
	char 		*curpath;
	struct stat   	s;
	dev_t 		current_dev;
	size_t 		curpath_len;

	/* dir stack */
	gint32 d = 0;
	gint32 dstack_size = D_STACKSIZE; /* realloc when hit */
	gint32 dstack_cnt  = 1;
	struct entry **dirstack = g_malloc(dstack_cnt * dstack_size * sizeof(struct entry *));

	/* file stack */
	gint32 f = 0;
	gint32 fstack_size = F_STACKSIZE; 
	gint32 fstack_cnt  = 1;
	struct entry **filestack = g_malloc(fstack_cnt * fstack_size * sizeof(struct entry *));

	if(!(dir = opendir(path))) {
		fprintf(stderr, "** Cannot enter: %s\n", path);
		g_free(filestack);
		g_free(dirstack);
		return TRUE;
	}

	/* get device */
	if (fstat(dirfd(dir), &s) != 0) {
		fprintf(stderr, "** Cannot determine holding device of the directory: %s\n", path);
		closedir(dir);
		g_free(filestack);
		g_free(dirstack);
		return TRUE;
	}
	current_dev = s.st_dev;

	while((dent = readdir(dir))) {
		if(!g_ascii_strcasecmp(dent->d_name, ".") || 
				!g_ascii_strcasecmp(dent->d_name, ".."))
			continue;

		curpath = g_strdup_printf("%s%c%s", path, DIR_SEP, dent->d_name);
		curpath_len = strlen(curpath);

		/* we're statting the file */
		if(lstat(curpath, &s) != 0) {
			fprintf(stderr, "** Could not stat path: %s\n", curpath);
			g_free(curpath);
			continue;
		}

		if (!S_ISDIR(s.st_mode)) {
			if ((opt_nobackup == 1) && !g_ascii_strcasecmp(dent->d_name, NOBACKUP)) {
				if (opt_verbose) {
					fprintf(stderr, "** " NOBACKUP " in %s\n", curpath);
				}
				/* add .nobackup to the list */
				pop = g_malloc(sizeof(struct entry));
				pop->f_name      = curpath;
				pop->f_name_size = curpath_len;
				pop->f_uid       = s.st_uid;
				pop->f_gid       = s.st_gid;
				pop->f_mtime     = s.st_mtime;
				pop->f_mode      = s.st_mode;
				pop->f_size      = s.st_size;

				g_tree_replace(t, (gpointer) entry_dup(pop), VALUE);

				g_free(pop);
				g_free(dirstack);
				g_free(filestack);
				closedir(dir);
				return TRUE;
			}
			
			filestack[f] = g_malloc(sizeof(struct entry));
			filestack[f]->f_name       = g_strdup(curpath);
			filestack[f]->f_name_size  = curpath_len;
			filestack[f]->f_uid        = s.st_uid;
			filestack[f]->f_gid        = s.st_gid;
			filestack[f]->f_mtime      = s.st_mtime;
			filestack[f]->f_mode       = s.st_mode;
			filestack[f]->f_size       = s.st_size;

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
			dirstack[d]->f_name       = g_strdup(curpath); 
			dirstack[d]->f_name_size  = curpath_len;
			dirstack[d]->f_uid        = s.st_uid;
			dirstack[d]->f_gid        = s.st_gid;
			dirstack[d]->f_mtime      = s.st_mtime;
			dirstack[d]->f_mode       = s.st_mode;
			dirstack[d]->f_size       = s.st_size;

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

	while (d > 0) {
		pop = dirstack[--d]; 
		g_tree_replace(t, (gpointer) entry_dup(pop), VALUE);
		/* recurse */
		/* potentially expensive operation. Better would be to when we hit
		 * .nobackup to go up the tree and delete some nodes */
		dir_crawl(t, pop->f_name);
		entry_free(pop);
	}
	while (f > 0) {
		pop = filestack[--f];
		g_tree_replace(t, (gpointer) entry_dup(pop), VALUE);
		entry_free(pop);
	}
	
	g_free(dirstack);
	g_free(filestack);
	return TRUE;
}
