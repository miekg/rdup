/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rdup-tr -- rdup translate, transform an
 * rdup filelist to an tar/cpio archive with 
 * per file compression and/or encryption
 */

#include "rdup.h"

/* options */
gboolean opt_null 	   = FALSE;                   /* delimit all in/output with \0  */
char *opt_format 	   = "%p%T %b %u %g %l %s %n\n"; /* format of rdup output */
char qstr[BUFSIZE + 1];				      /* static string for quoting */
gint opt_verbose 	   = 0;                       /* be more verbose */
time_t opt_timestamp       = 0;                       /* timestamp file */
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

int
main(int argc, char **argv)
{
	struct sigaction sa;
	char		 pwd[BUFSIZE + 1];
	int		 c, i;
	char		 *q, *r;
	GSList		 *p;
	GSList		 *child = NULL;		/* to forked childs: -P option */
	GSList		 *child_args = NULL;	/* list with the args for childs */
	char		 *args[5 + 1];
	
	/* i18n, set domain to rdup */
	/* really need LC_ALL? */
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
		msg(_("Will not run suid/sgid for safety reasons"), PROGNAME);
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

	while ((c = getopt (argc, argv, "P:F:hV")) != -1) {
		switch (c) {
			case 'P':
				printf("%s\n", optarg);
				q = g_strdup(optarg);
				/* this should be a comma seprated list
				 * arg0,arg1,arg2,...,argN */
				r = strstr(q, ",");
				if (!r) {
					printf("child alone %s\n", q);
					child = g_slist_append(child, q);
				} else {
					*r = '\0';
					printf("child %s\n", q);

					for(i = 0; r; r = strstr(r + 1, ","), i++) {
						if (i > 4) {
							printf("only 5 args allowed");
							break;
						}
						*r = '\0';
						printf("%s\n", q);
						args[i] = g_strdup(q);
						q = r + 1;
						printf("%d\n", i);
					}
					printf("%s\n", q);
					args[i] = g_strdup(q);

				}

				break;
			case 'F':
				opt_format = optarg;
				break;
			case 'h':
				/*usage(stdout); */
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

	if (argc < 2) {
		/* usage(stdout); */
		exit(EXIT_FAILURE);
	}

	for (p = g_slist_nth(child, 0); p; p = g_slist_next(p)) { 
                if (sig != 0)
                        signal_abort(sig);

                q = (char*) p->data;
		printf("%s\n", q);
        }
        return FALSE;


	exit(EXIT_SUCCESS);
}
