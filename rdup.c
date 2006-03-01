/* 
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

/* options */
gboolean opt_null 	   = FALSE;                   /* delimit all in/output with \0  */
gboolean opt_onefilesystem = FALSE;   		      /* stay on one filesystem */
gboolean opt_nobackup      = TRUE;             	      /* don't ignore .nobackup files */
gboolean opt_removed       = TRUE; 		      /* wether to print removed files */
gboolean opt_modified      = TRUE; 		      /* wether to print modified files */
char *opt_format 	   = "%p%m %u %g %l %s %n\n"; /* format of rdup output */
gint opt_verbose 	   = 0;                       /* be more verbose */
size_t opt_size            = 0;                       /* only output files smaller then <size> */
time_t opt_timestamp       = 0;                       /* timestamp file */
/* signals */
sig_atomic_t sig           = 0;

/* crawler.c */
void dir_crawl(GTree *t, char *path);
gboolean dir_prepend(GTree *t, char *path);
/* signal.c */
void got_sig(int signal);
/* no matter what, this prototype is correct */
ssize_t getdelim(char **lineptr, size_t *n, int delim, FILE *stream);

static void
usage(FILE *f) 
{	
	fprintf(f, "USAGE: %s [OPTION...] FILELIST DIR...\n", PROGNAME);
	fprintf(f, "%s generates a full or incremental file list, this\n", PROGNAME);
	fprintf(f, "list can be used to implement a (incremental) backup scheme.\n");
	fprintf(f, "\n   FILELIST\tfile list\n");
	fprintf(f, "   DIR\t\tdirectory or directories to dump\n");
	fprintf(f, "\nOPTIONS:\n");
	fprintf(f, "   -N FILE\tuse the timestamp of FILE for incremental dumps\n");
	fprintf(f, "   \t\tif FILE does not exist, a full dump is performed\n");
	fprintf(f, "   -F FORMAT\tuse specified format string\n");
	fprintf(f, "   \t\tdefaults to: \"%%p%%m %%u %%g %%l %%s %%n\\n\"\n");
	fprintf(f, "   -0\t\tdelimit internal filelist with NULLs\n");
	fprintf(f, "   -V\t\tprint version\n");
	fprintf(f, "   -c\t\tconcatenate the contents (FORMAT=\"%%p%%m %%u %%g %%l %%s\\n%%n%%C\")\n");
	fprintf(f, "   -h\t\tgives this help\n");
	fprintf(f, "   -m\t\tonly print new/modified files (overrides -r)\n");
	fprintf(f, "   -n\t\tignore " NOBACKUP " files\n");
	fprintf(f, "   -r\t\tonly print removed files (overrides -m)\n");
	fprintf(f, "   -s SIZE\tonly output files smaller then SIZE bytes\n");
	fprintf(f, "   -v\t\tbe more verbose (two times for more verbosity)\n");
	fprintf(f, "   -x\t\tstay in local file system\n");
	fprintf(f, "\nFORMAT:\n");
	fprintf(f, "   The following escape sequences are recognized:\n");
	fprintf(f, "   \'%%p\': '+' if new, '-' if removed\n");
        fprintf(f, "   \'%%m\': mode\n");
        fprintf(f, "   \'%%u\': uid\n");
        fprintf(f, "   \'%%g\': gid\n");
        fprintf(f, "   \'%%l\': path length\n");
        fprintf(f, "   \'%%s\': file size\n");
        fprintf(f, "   \'%%n\': path\n");
        fprintf(f, "   \'%%t\': time of modification (epoch)\n");
        fprintf(f, "   \'%%T\': \'type\'\n");
        fprintf(f, "   \'%%C\': file contents\n");
	fprintf(f, "\nReport bugs to <miek@miek.nl>\n");
	fprintf(f, "Licensed under the GPL. See the file LICENSE in the\n");
	fprintf(f, "source distribution of rdup.\n");
}

static void
version(FILE *f) 
{	
	fprintf(f, "%s %s\n", PROGNAME, VERSION);
}

/**
 * subtrace tree *b from tree *a, leaving
 * the elements that are only in *a. Essentially
 * a double diff: A diff (A diff B)
 */
static GTree *
g_tree_substract(GTree *a, GTree *b)
{
	GTree 	         *diff;
	struct substract s;

	diff = g_tree_new(gfunc_equal);
	s.d = diff;
	s.b = b;
	/* everything in a, but NOT in b 
	 * diff gets filled inside this function */
	g_tree_foreach(a, gfunc_substract, (gpointer)&s);
	return diff;
}

/** 
 * read a filelist, which should hold our previous
 * backup list
 */
static GTree *
g_tree_read_file(FILE *fp)
{
	char 	      *buf;
	char          *n;
	char 	      delim;
	mode_t        modus;
	GTree         *tree;
	struct entry *e;
	size_t        s;
	size_t 	      l;

	tree = g_tree_new(gfunc_equal);
	buf  = g_malloc(BUFSIZE + 1);
	s    = BUFSIZE;
	l    = 1;

	if (opt_null) {
		delim = '\0';
	} else {
		delim = '\n';
	}

	while ((getdelim(&buf, &s, delim, fp)) != -1) {
		if (s < LIST_MINSIZE) {
			fprintf(stderr, "** Corrupt entry in filelist at line: %zd\n", l);
			continue;
		}
		if (!opt_null) {
			n = strrchr(buf, '\n');
			if (n) {
				*n = '\0';
			}
		}

		/* get modus */
		buf[LIST_SPACEPOS] = '\0';
		modus = (mode_t)atoi(buf);
		if (modus == 0) {
			fprintf(stderr, "** Corrupt entry in filelist at line: %zd\n", l);
			continue;
		}

		e = g_malloc(sizeof(struct entry));
		e->f_name      = g_strdup(buf + LIST_SPACEPOS + 1);
		e->f_name_size = strlen(e->f_name);
		e->f_mode      = modus;
		e->f_uid       = 0;
		e->f_gid       = 0;
		e->f_size      = 0;
		e->f_mtime     = 0;
		g_tree_replace(tree, (gpointer) e, VALUE);
		l++;
	}
	g_free(buf);
	return tree;
}

/**
 * return the m_time of the filelist
 */
static time_t
timestamp(char *f)
{
	struct stat s;

	if (lstat(f, &s) != 0) {
		return 0;
	}
	return s.st_mtime;
}

int 
main(int argc, char **argv) 
{
	GTree 	*backup; 	/* on disk stuff */
	GTree 	*remove;	/* what needs to be rm'd */
	GTree 	*curtree; 	/* previous backup tree */
	FILE 	*fplist;
	gint    i;
	int 	c;
	char 	*crawl;
	char    pwd[BUFSIZE + 1];

	struct sigaction sa;

	/* setup our signal handling */
	sa.sa_flags   = 0;
	sigfillset(&sa.sa_mask);

	sa.sa_handler = got_sig;
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	curtree = g_tree_new(gfunc_equal);
	backup  = g_tree_new(gfunc_equal);
	remove  = NULL;
	opterr = 0;

	if (((getuid() != geteuid()) || (getgid() != getegid()))) {
		fprintf(stderr, "** For safety reasons " PROGNAME " will not run suid/sgid\n");
		exit(EXIT_FAILURE);
        }

	if (!getcwd(pwd, BUFSIZE)) {
		fprintf(stderr, "** Could not get current working directory\n");
		exit(EXIT_FAILURE);
	}
	for(c = 0; c < argc; c++) {
		if (strlen(argv[c]) > BUFSIZE) {
			fprintf(stderr, "** Argument length overrun\n");
			exit(EXIT_FAILURE);
		}
	}

	while ((c = getopt (argc, argv, "crmhVnN:s:vqx0F:")) != -1) {
		switch (c)
		{
			case 'F':
				opt_format = optarg;
				break;
			case 'c':
				opt_format = "%p%m %u %g %l %s\n%n%C";
				break;
			case 'h':
				usage(stdout);
				exit(EXIT_SUCCESS);
			case 'V':
				version(stdout);
				exit(EXIT_SUCCESS);
			case 'n':
				opt_nobackup = FALSE;
				break;
			case 'N': 
				opt_timestamp = timestamp(optarg);
				/* re-touch the timestamp file, if rdup fails
				 * the user needs to have something */
				if (creat(optarg, S_IRUSR | S_IWUSR) == -1) {
					fprintf(stderr, "** Could not create timestamp file: %s\n",
							optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'v':
				opt_verbose++; 
				if (opt_verbose > 2) {
					opt_verbose = 2;
				}
				break;
			case 'r':
				opt_removed = TRUE;
				opt_modified = FALSE;
				break;
			case 'm':
				opt_removed = FALSE;
				opt_modified = TRUE;
				break;
			case 'x':
				opt_onefilesystem = TRUE;
				break;
			case '0':
				opt_null = TRUE;
				break;
			case 's':
				opt_size = atoi(optarg);
				if (opt_size == 0) {
					fprintf(stderr, "** -s requires a numerical value\n");
					exit(EXIT_FAILURE);
				}
				break;
			default:
				fprintf(stderr, "** Uknown option\n");
				exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2) {
		usage(stdout);
		exit(EXIT_FAILURE); 
	}

	if (!(fplist = fopen(argv[0], "a+"))) {
		fprintf(stderr, "** Could not open filelist `%s\': %s\n", argv[0],
				strerror(errno));
		exit(EXIT_FAILURE);
	} else {
		rewind(fplist);
	}

	curtree = g_tree_read_file(fplist);

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != DIR_SEP) {
			crawl = g_strdup_printf("%s%c%s", pwd, DIR_SEP, argv[i]);
		} else {
			crawl = g_strdup(argv[i]);
		}

		/* add dirs leading up the backup dir */
		if (! dir_prepend(backup, crawl)) {
			exit(EXIT_FAILURE);
		}
		/* descend into the dark, misty directory */
		dir_crawl(backup, crawl);
		g_free(crawl);
	}
	remove = g_tree_substract(curtree, backup); 
	
	/* first what to remove, then what to backup */
	g_tree_foreach(remove, gfunc_remove, NULL); 
	g_tree_foreach(backup, gfunc_backup, NULL);

	/* write new filelist */
	ftruncate(fileno(fplist), 0);  
	g_tree_foreach(backup, gfunc_write, fplist);
	fclose(fplist); 

	g_tree_foreach(curtree, gfunc_free, NULL);
	g_tree_foreach(backup, gfunc_free, NULL);
	g_tree_destroy(curtree);
	g_tree_destroy(backup);
	g_tree_destroy(remove);

	if (opt_verbose > 1) 
		fprintf(stderr, "** DIRECTORIES :");
	for (i = 1; i < argc; i++) {
		if (opt_verbose > 1) 
			fprintf(stderr, " %s", argv[i]);
	}
	if (opt_verbose > 1)
		fprintf(stderr, "\n");

	exit(EXIT_SUCCESS);
}
