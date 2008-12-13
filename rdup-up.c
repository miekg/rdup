/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rdup-up -- update an directory tree with 
 * and rdup archive
 */

#include "rdup-up.h"
#include "io.h"

/* options */
gint opt_verbose 	   = 0;                         /* be more verbose */
gint opt_output		   = O_RDUP;			/* set these 2 so we can use parse_entry */
gint opt_input	           = I_RDUP;
sig_atomic_t sig           = 0;

/* signal.c */
void got_sig(int signal);

static void
msg_va_list(const char *fmt, va_list args)
{
        fprintf(stderr, "** %s: ", PROGNAME);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
}

void
msg(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        msg_va_list(fmt, args);
        va_end(args);
}

/* update the directory with the archive */
void
update(char *path)
{
	path = path;
	struct r_entry *rdup_entry;
	size_t         line, i;
	char           *buf, *pathbuf, *n;
	char           delim;
	FILE           *fp;
	struct stat    s;
	
	buf	= g_malloc(BUFSIZE + 1);
	pathbuf = g_malloc(BUFSIZE + 1);
	i       = BUFSIZE;
	fp  	= stdin;
	delim   = '\n';
	line    = 0;

	while ((rdup_getdelim(&buf, &i, delim, fp)) != -1) {
		line++;
		n = strrchr(buf, '\n');
		if (n) 
			*n = '\0';

		if (!(rdup_entry = parse_entry(buf, line, &s, DO_STAT))) {
			msg("Invalid rdup entry, bailing out");
			exit(EXIT_FAILURE);
		}

		/* we have a valid entry, read the filename */

		/* next read the filecontents */



	}

}


int
main(int argc, char **argv)
{
	struct sigaction sa;
	char		 pwd[BUFSIZE + 1];
	int		 c;
	char		 *path;
	
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PROGNAME, LOCALEROOTDIR);
	(void)textdomain(PROGNAME);
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

	while ((c = getopt (argc, argv, "hVv")) != -1) {
		switch (c) {
			case 'v':
				opt_verbose++;
				break;
			case 'h':
				usage_up(stdout);
				exit(EXIT_SUCCESS);
			case 'V':
				fprintf(stdout, "%s %s\n", PROGNAME, VERSION);
				exit(EXIT_SUCCESS);
			default:
				msg(_("Unknown option seen"));
				exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		msg("A single destination directory is required");
		exit(EXIT_FAILURE);
	}
	if (!g_path_is_absolute(argv[0])) 
		path = abspath(g_strdup_printf("%s%c%s", pwd, DIR_SEP, argv[0]));
	else
		path = abspath(argv[0]);

	if (!g_file_test(path, G_FILE_TEST_IS_DIR)) {
		msg("Not a directory: `%s\'", path);
		exit(EXIT_FAILURE);
	}

	update(path);

	exit(EXIT_SUCCESS);
}
