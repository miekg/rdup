#include "rdump.h"


int
dir_crawl(FILE *fp, char *path)
{
	DIR 	*dir;
	char 	*pop;
	char 	*curpath;
	struct 	dirent *dent;
	struct 	stat s;
	dev_t 	current_dev;
	
	uint32_t total_find = 0; /* keep track of how many files we find */

	/* dir stack */
	uint32_t d = 0;
	uint32_t dstack_size = 500; /* realloc when hit */
	uint32_t dstack_cnt  = 1;
	char **dirstack = g_malloc(dstack_cnt * dstack_size * sizeof(char*));

	/* file stack */
	uint32_t f = 0;
	uint32_t fstack_size = 2500; 
	uint32_t fstack_cnt  = 1;
	char **filestack = g_malloc(fstack_cnt * fstack_size * sizeof(char *));

	if(!(dir = opendir(path))) {
		fprintf(stderr, "Cannot enter the directory: %s", path);
		return 0;
	}

	/* get device */
	if (stat(path, &s) != 0) {
		fprintf(stderr, "Cannot determine holding device of the directory: %s", path);
		return 0;
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
				g_free(dirstack);
				g_free(filestack);
				return 0;
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

	total_find = f; 
	while (f > 0) {
		pop = filestack[--f];
		fprintf(fp, "%s\n", pop);
		g_free(pop);
	}
	
	while (d > 0) {
		pop = dirstack[--d]; 
		fprintf(fp, "%s\n", pop);
		total_find += dir_crawl(fp, pop);
		g_free(pop);
	}
	g_free(dirstack);
	g_free(filestack);
	return total_find;
}



#if 0
int
do_dir_walk(phost_t host)
{
	int i;
	unsigned int err;
	unsigned files = 0;
	char *filelist;
	FILE *fp;
	struct stat s;
	int de, fe, di, fi;

	/* these hold the patters */
	reg_exp dexclude = {0,};
	reg_exp dinclude = {0,};
	reg_exp fexclude = {0,};
	reg_exp finclude = {0,};

	/* reset the counters */
	de = fe = di = fi = 0;
	
	/* prepare the regular expression and give the list
	 * walk_dir. Also split up the files and dirs
	 */
	i = 0; err = 0;
	while (host->exclude[i] != '\0') {
		if (hash_slash(host->exclude[i])) {
			/* directory, kill the last slash */
			host->exclude[i][(strlen(host->exclude[i]) - 1)] = '\0';
			LOGDEBUG("dir exclude: [%s]", host->exclude[i]);
			err = regcomp(&dexclude.expr[de++], host->exclude[i], REG_EXTENDED | REG_NOSUB);
			dexclude.max++;
		} else {
			/* file */
			LOGDEBUG("file exclude: [%s]", host->exclude[i]);
			err = regcomp(&fexclude.expr[fe++], host->exclude[i], REG_EXTENDED | REG_NOSUB);
			fexclude.max++;
		}
		if (err > 0)  
			FATAL_HOST(host, "%s: %s", "Regular expression did not compile", host->exclude[i]);

		if (i++ > MAXDIR) {
			WARN("%s", "Maximum amount regular expression seen. Bailing out");
			break;
		}
	}
	i = 0; err = 0;
	while (host->include[i] != '\0') {
		if (hash_slash(host->include[i])) {
			/* directory, kill the slash */
			LOGDEBUG("dir include pattern: [%s]", host->include[i]);
			host->include[i][(strlen(host->include[i]) - 1)] = '\0';
			err = regcomp(&dinclude.expr[di++], host->include[i], REG_EXTENDED | REG_NOSUB);
			dinclude.max++;
		} else {
			/* file */
			LOGDEBUG("file include pattern: [%s]", host->include[i]);
			err = regcomp(&finclude.expr[fi++], host->include[i], REG_EXTENDED | REG_NOSUB);
			finclude.max++;
		}

		if (err > 0) 
			FATAL_HOST(host, "%s: %s", "Regular expression did not compile", host->include[i]);
		
		if (i++ > MAXDIR) {
			WARN("%s", "Maximum amount regular expression seen. Bailing out");
			break;
		}
	}
	
	/* what do we have processed */
	LOGDEBUG("DE %d FE %d DI %d FI %d\n", de, fe, di ,fi);

	filelist = g_strconcat(host->dirname_etc, FILELIST, NULL);
	host->filelist = filelist; /* c'est tres important. It still is? MG 3-11-2004 */ 

	fp = fopen(host->filelist, "w");
	if (!fp) 
		FATAL_HOST(host, "Could not open %s for writing", host->filelist);

	/* walk all the directories */
	i = 0;
	while (host->path[i] != '\0') {
		LOGDEBUG("%s: %s", "Walking", host->path[i]);
		/* path[i] can also be a file - test that first */
		if(lstat(host->path[i], &s) != 0) {
			WARN("%s: %s", "Cannot stat", host->path[i]);
			i++; /* BUG #6 */
			continue;
		}

		if (S_ISDIR(s.st_mode)) {
			/* files arg is not used yet */
			files = files + walk_dir(host->path[i], fp, host,
					&dexclude, &fexclude, &dinclude, &finclude);

			VVERBOSE("%s %d", "Files found:", files);
		} else {
			/* it's a file, write directly to the inclist */
			fprintf(fp, "%s", host->path[i]);
	                /* use \0 as delimeter, we give --null to tar  BUG #3 */
			putc(0, fp);
			VVERBOSE("Adding %s directly", host->path[i]);
		}
		i++;
	}
	/* valgrind, free these */
	for(i = 0; i < dexclude.max; i++) {
		regfree(&dexclude.expr[i]);
	}
	for(i = 0; i < dinclude.max; i++) {
		regfree(&dinclude.expr[i]);
	}
	for(i = 0; i < fexclude.max; i++) {
		regfree(&fexclude.expr[i]);
	}
	for(i = 0; i < finclude.max; i++) {
		regfree(&finclude.expr[i]);
	}
	fclose(fp);
	return files;
}

/**
 * Read a directory. Process the files first and put all subdirs
 * onto a stack. After all the files are processed, pop the stack
 * and start on the first subdirectory.
 * If a .nobackup is found the directory and its decendents are 
 * discarded.
 */
int 
walk_dir(char *path, FILE *fp, phost_t host,
		reg_exp *dexclude, reg_exp *fexclude, 
		reg_exp *dinclude, reg_exp *finclude)
{
	DIR *dir;
	char *pop;
	char *newpath;
	struct dirent *d;
	struct stat s;
	dev_t current_dev;
	
	unsigned int dsize = 50;
	unsigned int dirs = 1;
	unsigned int files = 1;
	unsigned int fsize = 250; 

	unsigned int total_find = 0; /* keep track of how many files we find */

	/* dir stack */
	char **dirstack = g_malloc(dirs * dsize * sizeof(char*));
	unsigned int di = 0;

	/* file stack */
	char **filestack = g_malloc(files * fsize * sizeof(char *));
	unsigned int fi = 0;

	if(!(dir = opendir(path))) {
		WARN("%s: %s", "Cannot enter the directory", path);
		return 0;
	}

	/* get device */
	if (stat(path, &s) != 0) {
		WARN("%s: %s","Cannot determine holding device of the directory:", path);
		return 0;
	}
	current_dev = s.st_dev;


	while((d = readdir(dir))) {
		if (hdup_sig)
			hdup_cleanup(-1, host);

		if(!g_ascii_strcasecmp(d->d_name, ".") || 
				!g_ascii_strcasecmp(d->d_name, ".."))
			continue;

		/* if path is / this will lead to //, making regexp fail.
		 * From; Boris */
		if (!g_ascii_strcasecmp(path, "/"))
			newpath = g_strdup_printf("/%s", d->d_name);
		else
			newpath = g_strdup_printf("%s/%s", path, d->d_name);

		if(strlen(newpath) + 2 > MAXPATHLEN) {
			WARN("%s", "Directory path too long");
			g_free(dirstack); g_free(filestack);
			return 0;
		}

		/* we're statting the file */
		if(lstat(newpath, &s) != 0) {
			WARN("%s %s", "Could not stat path:", newpath);
			continue;
		}

		/* catch everything, except dirs */
		if (!S_ISDIR(s.st_mode)) {
			/* if we see a .nobackup, the entire file and dir
			 * stacks we have build up can be discarded and we
			 * can return from our recursion
			 */
			if (host->nobackup != NULL && !g_ascii_strcasecmp(d->d_name, host->nobackup)) {
				/* if include match -> descend
				 * if exclude match -> don't decend */
				/* otherwise add */
				g_free(dirstack);
				g_free(filestack);
				return 0;
			}

			/* file excludes */
			if (reg_do_match(fexclude, newpath)) {
				/* a match don't add this file */
				continue;
			}


			/* file includes  */
			if (reg_do_match(finclude, newpath)) {
				filestack[fi++] = g_strdup(newpath);
				
				/* hack to show this */
				if (verbose < 1 && debug == 1) {
					fprintf(stderr, "[WALKER] %s\n", newpath);
				}
				
				if (fi % fsize == 0)
					filestack = g_realloc(filestack, 
							++files * fsize * sizeof(char *));
				continue;
			}
			

		} else if(S_ISDIR(s.st_mode)) {
			/* one filesystem */
			if (host->onefile && (s.st_dev != current_dev)) {
				WARN("%s", "Walking onto different filesystem");
				continue;
			}

			/* dir excludes */
			if (reg_do_match(dexclude, newpath)) {
				/* a match don't add this dir - but
				 * continue with the rest 
				 */

				continue;
#if 0
				g_free(dirstack);
				g_free(filestack);
				return 0;
#endif
			}

			/* dir includes */
			if (reg_do_match(dinclude, newpath)) {
				dirstack[di++] = g_strdup(newpath);

				if (di % fsize == 0)
					dirstack = g_realloc(dirstack, 
							++dirs * dsize * sizeof(char *));
				continue;
			}

			dirstack[di++] = g_strdup(newpath);

			if (di % dsize == 0) 
				dirstack = g_realloc(dirstack, 
						++dirs * dsize * sizeof(char *));
		} else {
			WARN("%s %s", "Neither file nor directory:", newpath);
		}
	}
	closedir(dir);

	/* we have walked the dir - and seen no .nobackup files
	 * Dump what we have */
	/* total_find = fi + di; also include directories here */
	total_find = fi; /* don't as hdup cannot see if nothing is found */
	while (fi > 0) {
		pop = filestack[--fi];
		fprintf(fp, "%s", pop);
		/* use \0 as delimeter, we give --null to tar */
		putc(0, fp);
		g_free(pop);
	}
	
	/* do it! do it! do it! */
	while (di > 0) {
		pop = dirstack[--di]; 
		/* GT fix - use tar --no-recursion to make the backup */
		if (patched_tar == 1) {
			fprintf(fp, "%s", pop);
			/* use \0 as delimeter, we give --null to tar */
			putc(0, fp);
		}
		total_find += walk_dir(pop, fp, host, dexclude, fexclude, dinclude, finclude);
		g_free(pop);
	}
	g_free(dirstack);
	g_free(filestack);
	return total_find;
}

/* walk the array and report back
 * any matches (TRUE) or FALSE when there
 * are none
 */
gboolean
reg_do_match(reg_exp *reg, char *path)
{
	int i;
	int res;

	for(i = 0; i < reg->max ; i++) {
		if ((res = regexec(&reg->expr[i], path, 0, NULL, 0)) == 0) {
			/* a match */
			return TRUE;
		}
	}
	return FALSE;
}
#endif
