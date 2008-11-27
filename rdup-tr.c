/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rdup-tr -- rdup translate, transform an
 * rdup filelist to an tar/cpio archive with 
 * per file compression and/or encryption
 */

#include "rdup-tr.h"

/* options */
char *opt_format 	   = "%p%T %b %u %g %l %s %n\n"; /* format of rdup output */
gint opt_tty	           = 0;				/* force write to stdout */
gint opt_verbose 	   = 0;                       /* be more verbose */
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

void read_stdin(GSList *pipes)
{
	char		*buf, *readbuf, *n;
	char		delim;
	size_t		len, i;
	FILE		*fp;
	int		f;
	struct archive  *archive;
	struct archive_entry *entry;
	struct stat     s;

	fp      = stdin;
	delim   = '\n';
	i       = BUFSIZE;
	buf     = g_malloc(BUFSIZE + 1);
	readbuf = g_malloc(BUFSIZE + 1);

	pipes = NULL; /* MOET weg */

	if ( (archive = archive_write_new()) == NULL) {
		msg("Failed to create empty archive");
		exit(EXIT_FAILURE);
	}

	if (archive_write_set_format_cpio(archive) != ARCHIVE_OK) {
		msg("Failed to set archive type to cpio");
		exit(EXIT_FAILURE);
	}

	/* wat doe ik hier fout? */
	archive_write_open(archive, NULL, r_archive_open, (archive_write_callback *)r_archive_write, r_archive_close);

	while ((rdup_getdelim(&buf, &i, delim, fp)) != -1) {
		if (sig != 0) {
			fclose(fp);
			signal_abort(sig);
		}
		n = strrchr(buf, '\n');
		if (n) 
			*n = '\0';

		if (stat(buf, &s) == -1) {
			 msg(_("Could not stat path `%s\': %s"), buf, strerror(errno));
			 continue;
		}

		if ((f = open(buf, O_RDONLY)) == -1) {
			msg(_("Could not open '%s\': %s"), buf, strerror(errno));
			continue;
		}

		entry = archive_entry_new();
		archive_entry_copy_stat(entry, &s);
		archive_entry_set_pathname(entry, buf);
		archive_write_header(archive, entry);

		len = read(f, readbuf, sizeof(readbuf));
		while (len > 0) {
			archive_write_data(archive, readbuf, len);
			len = read(f, readbuf, sizeof(readbuf));
		}
		close(f);
		archive_entry_free(entry);
	}
	archive_write_finish(archive);
}


int
main(int argc, char **argv)
{
	struct sigaction sa;
	char		 pwd[BUFSIZE + 1];
	int		 c, i, j;
	pid_t		 *cpid;
	char		 *q, *r;
	GSList		 *p;
	GSList		 *child     = NULL;		/* forked childs args: -P option */
	GSList		 *pipes1    = NULL;		/* reads from child */
	GSList		 *pipes2    = NULL;		/* writes to child */
	GSList		 *pids      = NULL;		/* child pids, for wait */
	char		 **args;
	int		 *p1;
	int		 *p2;

	char		 *buf;

	buf = g_malloc(102);
	
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

	while ((c = getopt (argc, argv, "cP:F:hV")) != -1) {
		switch (c) {
			case 'c':
				opt_tty = 1;
				break;
			case 'P':
				/* allocate new for each child */
				args  = g_malloc((MAX_CHILD_OPT + 2) * sizeof(char *));
				p1 = g_malloc(2 * sizeof(int));
				p2  = g_malloc(2 * sizeof(int));

				q = g_strdup(optarg);
				/* this should be a comma seprated list
				 * arg0,arg1,arg2,...,argN */
				r = strchr(q, ',');
				if (!r) {
					args[0] = g_strdup(q);
					args[1] = NULL;
				} else {
					*r = '\0';
					for(i = 0; r; r = strchr(r + 1, ','), i++) {
						if (i > 4) {
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
				pipes1 = g_slist_append(pipes1, p1);
				pipes2 = g_slist_append(pipes2, p2);
				break;
			case 'F':
				opt_format = optarg;
				break;
			case 'h':
				usage_tr(stdout);
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

	if (!opt_tty && isatty(1) == 1) {
		msg("Will not print to a tty");
		exit(EXIT_FAILURE);
	}

	for (j = 0, p = g_slist_nth(child, 0); p; p = p->next, j++) { 
                if (sig != 0)
                        signal_abort(sig);

		/* fork, exec, child */
                args  = (char**) p->data;

		/* p1[0] child read 
		 * p1[1] parent write
		 * p2[0] parent read 
		 * p2[1] child write 
		 */
		p1 = (int *) g_slist_nth_data(pipes1, j);
		p2 = (int *) g_slist_nth_data(pipes2, j);

		if ( pipe(p1) == -1 || pipe(p2) == -1) {
			msg("Error making pipes");
			exit(EXIT_FAILURE);
		}
		
		cpid = g_malloc(sizeof(pid_t));

		if ( (*cpid = fork()) == -1) {
			msg("Error forking");
			exit(EXIT_FAILURE);
		}

		if (*cpid == 0) {			/* child */
			close(p1[1]);	

			if (dup2(p1[0], 0) == -1)
				exit(EXIT_FAILURE);

			close(p2[0]);

			if (dup2(p2[1], 1) == -1)
				exit(EXIT_FAILURE);

			if ( execvp(args[0], args) == -1) {
				msg("Failed to exec `%s\': %s\n", args[0], strerror(errno));
				exit(EXIT_SUCCESS);
			}
			/* never reached */
			exit(EXIT_SUCCESS);
		} else {				/* parent */
			int k;
			close(p1[0]);

			pids = g_slist_append(pids, cpid);

			k = write(p1[1], "hallo\n", 6);
			if (k == -1) 
				printf("write %s\n", strerror(errno));
			printf("writen %d\n", k);

			close(p1[1]);

			printf("%d\n", p2[0]);
			if ( (k = read(p2[0], buf, 100)) != -1) {
				printf("%d\n", k);
				buf[k]='\0';
				printf("Komt hier: %s\n", buf);
			} else {
				printf("%s\n", strerror(errno));
			}
			close(p2[0]);
		

			waitpid(*cpid, NULL, 0);
		}
        }

	for (p = g_slist_nth(pids, 0); p; p = p->next) { 
                if (sig != 0)
                        signal_abort(sig);

		cpid = p->data;
		fprintf(stderr, "pids %d\n", *cpid);
	}
	exit(EXIT_SUCCESS);
}
