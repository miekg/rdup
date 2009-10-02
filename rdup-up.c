/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rdup-up -- update an directory tree with 
 * and rdup archive
 */

#include "rdup-up.h"
#include "io.h"

char *PROGNAME="rdup-up";
/* options */
gint opt_verbose 	   = 0;                         /* be more verbose */
gint opt_output		   = O_RDUP;			/* set these 2 so we can use parse_entry */
gint opt_input	           = I_RDUP;
gboolean opt_dry	   = FALSE;			/* don't touch the filesystem */
gboolean opt_top	   = FALSE;			/* create top dir if it does not exist */
gchar *opt_path_strip	   = NULL;			/* -r PATH, strip PATH from pathnames */
guint opt_path_strip_len   = 0;				/* number of path labels in opt_path_strip */
guint opt_strip		   = 0;				/* -s: strippath */
sig_atomic_t sig           = 0;
GSList *hlink_list		   = NULL;			/* save hardlink for post processing */		
extern int opterr;
int opterr		   = 0;

/* update the directory with the archive */
static gboolean
update(char *path)
{
	struct rdup    *rdup_entry;
	size_t         line, i, pathsize;
	size_t	       pathlen;
	char           *buf, *pathbuf, *n, *p;
	char           delim;
	FILE           *fp;
	struct stat    s;
	gboolean       ok;

	buf	= g_malloc(BUFSIZE + 1);
	pathbuf = g_malloc(BUFSIZE + 1);
	i       = BUFSIZE;
	fp  	= stdin;
	delim   = '\n';
	line    = 0;
	ok      = TRUE;
	pathlen = strlen(path);

	while ((rdup_getdelim(&buf, &i, delim, fp)) != -1) {
		line++;
		n = strrchr(buf, '\n');
		if (n) 
			*n = '\0';

		if (!(rdup_entry = parse_entry(buf, line, &s))) {
			/* msgs from entry.c */
			exit(EXIT_FAILURE);
		}

		/* we have a valid entry, read the filename */
		pathsize = fread(pathbuf, sizeof(char), rdup_entry->f_name_size, stdin);

		if (pathsize != rdup_entry->f_name_size) {
			msg(_("Reported name size (%zd) does not match actual name size (%zd)"),
					rdup_entry->f_name_size, pathsize);
			exit(EXIT_FAILURE);
		}
		pathbuf[pathsize] = '\0';
		if (pathbuf[0] != '/') {
			msg(_("Pathname does not start with /: `%s\'"), pathbuf);
			exit(EXIT_FAILURE);
		}

		rdup_entry->f_name = pathbuf;

		/* extract target from rdup_entry */
		if (S_ISLNK(rdup_entry->f_mode) || rdup_entry->f_lnk) {
			// filesize is spot where to cut
			rdup_entry->f_name[rdup_entry->f_size] = '\0';
			rdup_entry->f_target = rdup_entry->f_name + 
				rdup_entry->f_size + 4;
		} else {
			rdup_entry->f_target = NULL;
		}

		/* strippath must be inserted here */
		if (opt_strip)
			strippath(rdup_entry);

		if (opt_path_strip) 
			strippathname(rdup_entry);

		if (!rdup_entry->f_name)
			p = NULL;
		else {
			/* avoid // at the beginning */
			if (pathlen == 1 && path[0] == '/') {
				p = g_strdup(rdup_entry->f_name);
			} else {
				p = g_strdup_printf("%s%s", path, rdup_entry->f_name);
				rdup_entry->f_name_size += pathlen;
				if (S_ISLNK(rdup_entry->f_mode) || rdup_entry->f_lnk)
					rdup_entry->f_size += pathlen;
			}
		}
		rdup_entry->f_name = p;
		if (mk_obj(stdin, path, rdup_entry) == FALSE)
			ok = FALSE;
	}

	/* post-process hardlinks */
	if (mk_hlink(hlink_list) == FALSE)
		ok = FALSE;

	g_free(buf);
	g_free(pathbuf);
	return ok;
}

int
main(int argc, char **argv)
{
	struct sigaction sa;
	char		 pwd[BUFSIZE + 1];
	int		 c;
	char		 *path;
	
#ifdef ENABLE_NLS
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

	while ((c = getopt (argc, argv, "thnVvs:r:")) != -1) {
		switch (c) {
			case 'v':
				opt_verbose++;
				break;
			case 'h':
				usage_up(stdout);
				exit(EXIT_SUCCESS);
			case 'n':
				opt_dry = TRUE;
				break;
			case 's':
				if (opt_path_strip != NULL) {
					msg(_("The -r and -s option can not be used together"));
					exit(EXIT_FAILURE);
				}
                                opt_strip = abs(atoi(optarg));
                                break;
			case 'r':
				if (opt_strip != 0) {
					msg(_("The -r and -s option can not be used together"));
					exit(EXIT_FAILURE);
				}
				if (strlen(optarg) == 0) {		/* does this help? XX */
					msg(_("-r needs an arugment"));
					exit(EXIT_FAILURE);
				}

				/* expand relative paths */
				if (!g_path_is_absolute(optarg))
					opt_path_strip = abspath(g_strdup_printf("%s/%s", pwd, optarg));
				else
					opt_path_strip = abspath(optarg);

				if (!opt_path_strip) {
					msg(_("Failed to expand path `%s\'"), optarg);
					exit(EXIT_FAILURE);
				}

				if (opt_path_strip[strlen(opt_path_strip) - 1] != '/')
					opt_path_strip = g_strdup_printf("%s/", opt_path_strip);

				/* count the number of labels */
				guint i;
				for(i = 0; i < strlen(opt_path_strip); i++) {
					if (opt_path_strip[i] == '/')
						opt_path_strip_len++;
				}
				opt_path_strip_len--;	/* we added the closing slash, so need -1 here */
				break;
			case 't':
				opt_top = TRUE;
				break;
			case 'V':
				fprintf(stdout, "%s %s\n", PROGNAME, VERSION);
				exit(EXIT_SUCCESS);
			default:
				msg(_("Unknown option seen `%c\'"), optopt);
				exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		msg(_("Destination directory is required"));
		exit(EXIT_FAILURE);
	}
	if (!g_path_is_absolute(argv[0])) 
		path = abspath(g_strdup_printf("%s/%s", pwd, argv[0]));
	else
		path = abspath(argv[0]);

	if (!opt_dry) {
		if (opt_top) {
			/* this is not 100%, but better than nothing */
			if (g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
				msg(_("Failed to create directory `%s\'"), path);
				exit(EXIT_FAILURE);
			}
			if (mkpath(path, 00777) == -1) {
				msg(_("Failed to create directory `%s\': %s"), path, strerror(errno));
				exit(EXIT_FAILURE);
			}
		} else {
			if (!g_file_test(path, G_FILE_TEST_IS_DIR)) {
				msg(_("No such directory: `%s\'"), path);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (update(path) == FALSE)
		exit(EXIT_FAILURE);

	exit(EXIT_SUCCESS);
}
