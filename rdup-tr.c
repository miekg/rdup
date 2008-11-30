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

/* close all pipes except no1 and no2 (if not -1) */
void
close_pipes(GSList *pipes, int no1, int no2)
{
	GSList *p;
	int *q;
	int j;

	for (j = 0, p = g_slist_nth(pipes, 0); p; p = p->next, j++) { 
		q = p->data;
		if ( (j != -1 && j != no1) && (j != -1 && j != no2) ) {
			fprintf(stderr, "closing %d\n", j);
			close(q[0]);
			close(q[1]);
		}
	}
}

void
write_pipe(int writefd, int tmpfd) 
{
	int k;

	k = write(writefd, "hallo\n", 6);
	if (k == -1) {
		msg("write error: %s\n", strerror(errno));
	}

	close(writefd);
	close(tmpfd);
}

void 
read_stdin(GSList *pipes)
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
	GSList		 *child    = NULL;		/* forked childs args: -P option */
	GSList		 *pipes    = NULL;		/* inter child pipes */
	GSList		 *pids     = NULL;		/* child pids */
	char		 **args;
	int		 *pips;
	int		 childs    = 0;			/* number of childs */
	int		 tmpfile;
	
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
				args = g_malloc((MAX_CHILD_OPT + 2) * sizeof(char *));
				pips = g_malloc(2 * sizeof(int));

				if (pipe(pips) == -1) {
					msg("Error creating pipe");
					exit(EXIT_FAILURE);
				}
				pipes = g_slist_append(pipes, pips);

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
				childs++;
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

	/* tmp file to put the contents in */
	tmpfile = mkstemp("/tmp/rdup.XXXXXX/tmp");
	/* extra pipe for parent and first child communication */
	pips = g_malloc(2 * sizeof(int));
	if (pipe(pips) == -1) {
		msg("Failed to create parent/child pipe");
		exit(EXIT_FAILURE);
	}
	pipes = g_slist_prepend(pipes, pips);

	for (j = 0, p = g_slist_nth(child, 0); p; p = p->next, j++) { 
                if (sig != 0)
                        signal_abort(sig);

		/* fork, exec, child */
                args = (char**) p->data;
		cpid = g_malloc(sizeof(pid_t));
		pips = (g_slist_nth(pipes, j))->data;

		if ( (*cpid = fork()) == -1) {
			msg("Error forking");
			exit(EXIT_FAILURE);
		}

		if (*cpid != 0) {
			/* save the pids, parent */
			pids = g_slist_append(pids, cpid);
		} else {
			/* child */
			if (j != childs) {
				/* not the last one */

				close(tmpfile);

				/* close write end, connect read to stdin */
				close(pips[1]);
				if (dup2(pips[0], 0) == -1) {
					exit(EXIT_FAILURE);
				}

				/* re-use pips */
				pips = (g_slist_nth(pipes, j + 1))->data;

				/* close read end, connect write to stdout */
				close(pips[0]);
				if (dup2(pips[1], 1) == -1) {
					exit(EXIT_FAILURE);
				}

				close_pipes(pipes, j, j+1);
			} else {
				/* last one, conn to stdout */

				if (dup2(tmpfile, 1) == -1) {
					exit(EXIT_FAILURE);
				}

				/* close write end, connect read to stdin */
				close(pips[1]);
				if (dup2(pips[0], 0) == -1) {
					exit(EXIT_FAILURE);
				}
				close_pipes(pipes, j, -1);
			}

			/* finally ... exec */
			if ( execvp(args[0], args) == -1) {
				msg("Failed to exec `%s\': %s\n", args[0], strerror(errno));
				exit(EXIT_SUCCESS);
			}
			/* never reached */
			exit(EXIT_SUCCESS);
		}
        }

	pips = (g_slist_nth(pipes, 0))->data;
	close(pips[0]);
	close_pipes(pipes, 0, -1);

	/* read a file, write it to pips[1] 
	 * when done, read tmpfile and print it out
	 * rinse, repeat
	 */
	write_pipe(pips[1], tmpfile);
	/* als geen, dan /bin/cat */

	/* cleanup */
	for (p = g_slist_nth(pids, 0); p; p = p->next) { 
                if (sig != 0)
                        signal_abort(sig);

		/* timeout and then kill the process? */

		waitpid(*(pid_t* )(p->data), NULL, 0);
	}
	exit(EXIT_SUCCESS);
}
