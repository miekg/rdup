/* 
 * Copyright (c) 2005 - 2010 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

char *PROGNAME="rdup";
/* options */
gboolean opt_onefilesystem = FALSE;   		      /* stay on one filesystem */
gboolean opt_nobackup      = TRUE;             	      /* don't ignore .nobackup files */
gboolean opt_removed       = TRUE; 		      /* whether to print removed files */
gboolean opt_modified      = TRUE; 		      /* whether to print modified files */
gboolean opt_reverse	   = FALSE;		      /* whether to reverse print the lists */
gboolean opt_tty	   = FALSE;		      /* force write to tty */
gboolean opt_atime	   = FALSE;			      /* reset access time */
char *opt_format 	   = "%p%T %b %t %u %U %g %G %l %s\n%n%C"; /* format of rdup output */
#if 0
char *opt_format 	   = "%p%T %b %t %u %U %g %G %l %s %n\n";
#endif
gint opt_verbose 	   = 0;                       /* be more verbose */
size_t opt_size            = 0;                       /* only output files smaller then <size> */
time_t opt_timestamp       = 0;                       /* timestamp file c|m time */

int sig		           = 0;
extern int opterr;
int opterr		   = 0;
GSList *child		   = NULL;

#define CORRUPT(x)	{ \
			msg((x), l); \
			l++; \
			continue; \
			}
/**
 * subtract tree *b from tree *a, leaving
 * the elements that are only in *a. Essentially
 * a double diff: A diff (A diff B)
 */
static GTree *
g_tree_subtract(GTree *a, GTree *b)
{
	GTree 	         *diff;
	struct subtract s;

	diff = g_tree_new(gfunc_equal);
	s.d = diff;
	s.b = b;
	/* everything in a, but NOT in b 
	 * diff gets filled inside this function */
	g_tree_foreach(a, gfunc_subtract, (gpointer)&s);
	return diff;
}

/**
 * read a filelist, which should hold our previous
 * backup list
 */
static GTree *
g_tree_read_file(FILE *fp)
{
	gchar 	      *buf, *n, *p, *q;
	gchar 	      delim, linktype;
	mode_t        modus;
	GTree         *tree;
	struct rdup   *e;
	size_t        s;
	size_t 	      l;
	size_t        f_name_size;
	size_t        f_size;
	size_t        str_len;
	dev_t	      f_dev;
	ino_t	      f_ino;

	tree = g_tree_new(gfunc_equal);
	buf  = g_malloc(BUFSIZE + 1);
	s    = BUFSIZE;
	l    = 1;
	delim= '\n';

	if (!fp)
	    return tree;

	while ((rdup_getdelim(&buf, &s, delim, fp)) != -1) {
		if (sig != 0) {
			fclose(fp);
			signal_abort(sig);
		}

		/* comment */
		if (buf[0] == '#') 
			continue;

		if (s < LIST_MINSIZE) 
			CORRUPT("Corrupt entry at line: %zd, line to short"); 

		n = strrchr(buf, '\n');

		/* get modus */
		if (buf[LIST_SPACEPOS] != ' ')
			CORRUPT("Corrupt entry at line: %zd, no space found");

		buf[LIST_SPACEPOS] = '\0';
		modus = (mode_t)atoi(buf);
		if (modus == 0)
			CORRUPT("Corrupt entry at line: %zd, mode should be numerical");

		/* the dev */
		q = buf + LIST_SPACEPOS + 1;
		p = strchr(buf + LIST_SPACEPOS + 1, ' ');
		if (!p)
			CORRUPT("Corrupt entry at line: %zd, no space found");
		
		*p = '\0';
		f_dev = (dev_t)atoi(q);
		if (f_dev == 0)
			CORRUPT("Corrupt entry at line: %zd, zero device");

		/* the inode */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p) 
			CORRUPT("Corrupt entry at line: %zd, no space found");
		
		*p = '\0';
		f_ino = (ino_t)atoi(q);
		if (f_ino == 0)
			CORRUPT("Corrupt entry at line: %zd, zero inode");

		/* hardlink/link or anything else: h/l or * */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p) 
			CORRUPT("Corrupt entry at line: %zd, no link information found");

		linktype = *q;
		if (linktype != '-' && linktype != 'h' && linktype != 'l')
			CORRUPT("Illegal link type at line: %zd");

		/* skip these for now - but useful to have */
		/* uid */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p)
			CORRUPT("Corrupt entry at line: %zd, no space found");
		
		/* gid */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p)
			CORRUPT("Corrupt entry at line: %zd, no space found");

		/* the path size */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p) {
			CORRUPT("Corrupt entry at line: %zd, no space found");
		}
		*p = '\0';
		f_name_size = (size_t)atoi(q);
		if (f_name_size == 0) 
			CORRUPT("Pathname lenght can not be zero at line: %zd");

		/* filesize */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p)
			CORRUPT("Corrupt entry at line: %zd, no space found");

		*p = '\0';
		f_size = (size_t)atoi(q);
 
		/* with getdelim we read the delimeter too kill it here */
		str_len = strlen(p + 1);
		if (str_len == 1)
			CORRUPT("Actual pathname length can not be zero at line: %zd");

		p[str_len] = '\0'; str_len--;
		if (str_len != f_name_size) {
			msg(_("Corrupt entry at line: %zd, length `%zd\' does not match `%zd\'"), l,
					str_len, f_name_size);
			l++; 
			continue;
		}

		e = g_malloc(sizeof(struct rdup));
		e->f_name      = g_strdup(p + 1);

		if (linktype == 'h' || linktype == 'l') {
			e->f_name_size    = strlen(e->f_name);
			e->f_name[f_size] = '\0'; /* set NULL just before the ' -> ' */
			e->f_size         = strlen(e->f_name);
			e->f_target       = e->f_name + f_size + 4;
		} else {
			e->f_name_size = f_name_size;
			e->f_target    = NULL;
			e->f_size      = f_size;
		}

		e->f_mode      = modus;
		e->f_uid       = 0;	/* keep this 0 for now */
		e->f_gid       = 0;	/* keep this 0 for now */
		e->f_ctime     = 0;
		e->f_mtime     = 0;
		e->f_atime     = 0;
		e->f_user      = NULL;
		e->f_group     = NULL;
		e->f_dev       = f_dev;
		e->f_ino       = f_ino;
		if (linktype == 'h')
			e->f_lnk = 1;
		else
			e->f_lnk = 0;

		g_tree_insert(tree, (gpointer)e, VALUE);
		l++;
	}
	g_free(buf);
	return tree;
}

/**
 * return the c_time of the filelist
 */
static time_t
timestamp(char *f, gboolean ctime)
{
	struct stat s;
	if (lstat(f, &s) != 0) {
		return 0;
	}
	if (ctime)
		return s.st_ctime;
	return s.st_mtime;
}

int
main(int argc, char **argv)
{
	GTree 	*backup; 	/* on disk stuff */
	GTree 	*remove;	/* what needs to be rm'd */
	GTree 	*curtree; 	/* previous backup tree */
	GTree	*new;		/* all that is new */
	GTree	*changed;	/* all that is possibly changed */
	GHashTable *linkhash;	/* hold dev, inode, name stuff */
	GHashTable *userhash;	/* holds uid -> username */
	GHashTable *grouphash;  /* holds gid -> groupname */
	struct utimbuf ut;	/* time to set on timestamp file */

	FILE 	*fplist;
	gint    i;
	int 	c;
	char    pwd[BUFSIZE + 1];
	char	*path, *stamp, *q, *r;
	gchar   **args;
	gboolean devnull = FALSE;	/* hack: remember if we open /dev/null */
	struct sigaction sa;

	ut.actime = time(NULL);
	ut.modtime = ut.actime;
	
	/* i18n, set domain to rdup */
#ifdef ENABLE_NLS
	/* should this be translated? :-) */
	if (!setlocale(LC_MESSAGES, ""))
		 msg(_("Locale could not be set"));
	bindtextdomain(PACKAGE_NAME, LOCALEROOTDIR);
	(void)textdomain(PACKAGE_NAME);
#endif /* ENABLE_NLS */
	
	/* setup our signal handling */
	sa.sa_flags   = 0;
	sigfillset(&sa.sa_mask);

	sa.sa_handler = got_sig;
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	curtree = g_tree_new(gfunc_equal);
	backup  = g_tree_new(gfunc_equal);
	linkhash  = g_hash_table_new(g_str_hash, g_str_equal);
	grouphash = g_hash_table_new(g_int_hash, g_int_equal);
	userhash  = g_hash_table_new(g_int_hash, g_int_equal);
	remove  = NULL;
	opterr = 0;
	stamp = NULL;

	if (((getuid() != geteuid()) || (getgid() != getegid()))) {
		msg(_("Will not run suid/sgid for safety reasons"));
		exit(EXIT_FAILURE);
        }

	if (!getcwd(pwd, BUFSIZE)) {
		msg(_("Could not get current working directory"));
		exit(EXIT_FAILURE);
	}

	for(c = 0; c < argc; c++) {
		if (strlen(argv[c]) > BUFSIZE) {
			msg(_("Argument length overrun"));
			exit(EXIT_FAILURE);
		}
	}
#ifdef DEBUG
	msgd(__func__, __LINE__, _("DEBUG is enabled!"));
#endif /* DEBUG */
	while ((c = getopt (argc, argv, "acrlmhVRnd:N:M:P:s:vqxF:E:")) != -1) {
		switch (c) {
			case 'F':
				opt_format = optarg;
				break;
			case 'E':
				if (!regexp_init(optarg)) 
					exit(EXIT_FAILURE);
				break;
			case 'a':
				opt_atime = TRUE;
				break;
			case 'c':
				opt_tty = TRUE;
				break;
			case 'h':
				usage(stdout);
				exit(EXIT_SUCCESS);
			case 'V':
#ifdef DEBUG
				fprintf(stdout, "%s %s (with --enable-debug)\n", PROGNAME, VERSION);
#else
				fprintf(stdout, "%s %s\n", PROGNAME, VERSION);
#endif /* DEBUG */

				exit(EXIT_SUCCESS);
			case 'n':
				opt_nobackup = FALSE;
				break;
			case 'N': 
				opt_timestamp = timestamp(optarg, TRUE);
				stamp = optarg;
				break;
			case 'M':
				opt_timestamp = timestamp(optarg, FALSE);
				stamp = optarg;
				break;
			case 'R':
				opt_reverse = TRUE;
				break;
			case 'P':
				/* (void)setvbuf(stdout, NULL, _IONBF, 0);
				(void)setvbuf(stdin, NULL, _IONBF, 0); */

                                /* allocate new for each child */
                                args = g_malloc((MAX_CHILD_OPT + 2) * sizeof(char *));
                                q = g_strdup(optarg);
                                /* this should be a comma seprated list
                                 * arg0,arg1,arg2,...,argN */
                                r = strchr(q, ',');
                                if (!r) {
                                        args[0] = q;
                                        args[1] = NULL;
                                } else {
                                        *r = '\0';
                                        for(i = 0; r; r = strchr(r + 1, ','), i++) {
                                                if (i > MAX_CHILD_OPT) {
                                                        msg(_("Only %d extra args per child allowed"), MAX_CHILD_OPT);
                                                        exit(EXIT_FAILURE);
                                                }   
                                                *r = '\0';
                                                args[i] = g_strdup(q);
                                                q = r + 1;
                                        }   
                                        args[i] = g_strdup(q);
                                        args[i + 1] = NULL;
                                }   
                                child = g_slist_append(child, args);
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
			case 's':
				opt_size = atoi(optarg);
				if (opt_size == 0) {
					msg(_("-s requires a numerical value"));
					exit(EXIT_FAILURE);
				}
				break;
			default:
				msg(_("Unknown option seen `%c\'"), optopt);
				exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage(stdout);
		exit(EXIT_FAILURE);
	}

	/* be as irritating as rdup-tr */
	if (!opt_tty && isatty(1) == 1) {
		msg(_("Will not write to a tty"));
		exit(EXIT_FAILURE);
	}

	if (argc == 1) {
		/* default to . as the dir to dump */
		msg(_("No directory given, dumping `.\'"));
		argv[1] = g_strdup(".");
		argc++;
	}

	if (strcmp(argv[0], "/dev/null") == 0)
		devnull = TRUE;

	if (!(fplist = fopen(argv[0], "r"))) {
		curtree = g_tree_read_file(NULL);
	} else {
		curtree = g_tree_read_file(fplist);
		fclose(fplist);
	}

	for (i = 1; i < argc; i++) {
		if (!g_path_is_absolute(argv[i])) 
		    path = abspath(g_strdup_printf("%s/%s", pwd, argv[i]));
		else
		    path = abspath(argv[i]);

		if (!path) {
			msg(_("Skipping `%s\'"), argv[i]);
			continue;
		}

		/* add dirs leading up the dir/file */
		if (!dir_prepend(backup, path, userhash, grouphash)) continue;
		
		/* descend into the dark, misty directory */
		dir_crawl(backup, linkhash, userhash, grouphash, path);
	}

	/* everything that is gone from the filesystem */
	remove  = g_tree_subtract(curtree, backup);

	/* everything that is really new on the filesystem */
	new     = g_tree_subtract(backup, curtree);

	/* all stuff that should be mtime checked, to see if it has changed */
	changed = g_tree_subtract(backup, new);
	/* some dirs might still linger in changed, while they are in fact
	 * removed, kill those here */
	changed = g_tree_subtract(changed, remove);

#ifdef DEBUG
	/* we first crawled the disk to see what is changed
	 * then we output. If we wait here a few seconds
	 * we can remove files that should have been
	 * added. This way we can make a race condition(s)
	 * happen - if they are there in code 
	 */
	msg(_("DEBUG: sleeping for a while"));
	sleep(5);
#endif /* DEBUG */

	/* first what to remove, then what to backup */
	if (opt_reverse) {
		GList *list_remove, *list_changed, *list_new = NULL;
		list_remove = reverse(remove);
		list_changed = reverse(changed);
		list_new = reverse(new);
		g_list_foreach(list_remove, gfunc_remove_list, NULL); 
		g_list_foreach(list_changed, gfunc_backup_list, NULL); 
		g_list_foreach(list_new, gfunc_new_list, NULL); 
	} else {
		g_tree_foreach(remove, gfunc_remove, NULL);
		g_tree_foreach(changed, gfunc_backup, NULL);
		g_tree_foreach(new, gfunc_new, NULL);
	}

	/* write new list */
	if (!devnull) {
	    if (!(fplist = fopen(argv[0], "w"))) {
		    msg(_("Could not write filelist `%s\': %s"), argv[0], strerror(errno));
	    } else {
		/* write temporary file, add little comment */
		fprintf(fplist, 
			"# mode dev inode linktype uid gid pathlen filesize path\n");
		g_tree_foreach(backup, gfunc_write, fplist);
		fclose(fplist);
	    }
	}
	/* re-touch the timestamp file */
	if (stamp) {
		if (creat(stamp, S_IRUSR | S_IWUSR) == -1) {
			msg(_("Could not create timestamp file `%s\': %s"), stamp, strerror(errno));
			exit(EXIT_FAILURE);
		}
		/* and set the time when rdup was started */
		if (utime(stamp, &ut) == -1) {
			msg(_("Failed to reset atime: '%s\': %s"), stamp, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}
