/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rdup-tr -- rdup translate, transform an
 * rdup filelist to an tar/cpio archive with 
 * per file compression and/or encryption or whatever
 */

/* stdio is only used for errors
 * the rest is all read, write and pipe
 */

#include "rdup-tr.h"
#include "protocol.h"
#include "io.h"

char *PROGNAME = "rdup-tr";
/* options */
char *template;
gboolean opt_tty           = FALSE;			/* force write to tty */
gchar *opt_crypt_key	   = NULL;			/* encryption key */
gchar *opt_decrypt_key	   = NULL;			/* encryption key */
gint opt_verbose 	   = 0;                         /* be more verbose */
gint opt_output	           = O_RDUP;			/* default output */
gint opt_input		   = I_RDUP;			/* default intput */

sig_atomic_t sig           = 0;
char *o_fmt[] = { "", "tar", "cpio", "pax", "rdup"};	/* O_NONE, O_TAR, O_CPIO, O_PAX, O_RDUP */

/* read filenames from stdin, put them through
 * the childeren, collect the output from the last
 * child and create the archive on stdout
 */
void 
stdin2archive(GSList *child)
{
	char		*buf, *readbuf, *n, *out;
	char		delim;
	size_t		i, line;
	ssize_t		len;
	FILE		*fp;
	int		f, j;
	GSList		*pipes;
	GSList		*pids;				/* child pids */
	int		*parent;			/* parent pipe */
	struct archive  *archive;
	struct archive_entry *entry;
	struct stat     s;
	struct r_entry  *rdup_entry = NULL;

	fp      = stdin;
	delim   = '\n';
	i       = BUFSIZE;
	buf     = g_malloc(BUFSIZE + 1);
	readbuf = g_malloc(BUFSIZE + 1);
	j	= ARCHIVE_OK;
	entry   = NULL;
	line    = 0;

	if (opt_output == O_RDUP) {
		archive = NULL;
	} else {
		if ( (archive = archive_write_new()) == NULL) {
			msg("Failed to create new archive");
			exit(EXIT_FAILURE);
		}

		switch(opt_output) {
			case O_TAR:
				j = archive_write_set_format_ustar(archive);
				break;
			case O_CPIO:
				j = archive_write_set_format_cpio(archive);
				break;
			case O_PAX:
				j = archive_write_set_format_pax(archive);
				break;
			case O_RDUP:
				/* never reached, but here for completeness */
				j = ARCHIVE_OK;
				break;
		}

		if (j != ARCHIVE_OK) {
			msg("Failed to set archive type to %s", o_fmt[opt_output]);
			exit(EXIT_FAILURE);
		} else {
			archive_write_open_fd(archive, 1);
		}
	}

	while ((rdup_getdelim(&buf, &i, delim, fp)) != -1) {
		line++;
		n = strrchr(buf, '\n');
		if (n) 
			*n = '\0';

		if (!(rdup_entry = parse_entry(buf, line, &s, DO_STAT))) {
			continue;
		}

		if (opt_verbose > 0) {
			out = g_strdup_printf("%s\n", rdup_entry->f_name);
			if (write(2, out, rdup_entry->f_name_size + 1) == -1) {
				msg("Writing to stderr failed");
				exit(EXIT_FAILURE);
			}
		}

		if (sig != 0) {
			signal_abort(sig);
		}

		if (rdup_entry->plusmin == '-') {
			if (opt_output == O_RDUP) {
				rdup_write_header(rdup_entry);
				goto not_s_isreg;
			}
			/* all other outputs cannot handle this, but this
			 * is ALSO checked in parse_entry() */
			continue;
		}

		if (opt_output != O_RDUP) {
			entry = archive_entry_new();
			archive_entry_copy_stat(entry, &s);
			archive_entry_copy_pathname(entry, rdup_entry->f_name);

			/* with list input rdup-tr cannot possibly see
			 * that a file is hardlinked - but the code
			 * to handle that case is here anyway
			 */
			if (S_ISLNK(rdup_entry->f_mode) || rdup_entry->f_lnk == 1) {
				/* source */
				rdup_entry->f_name[rdup_entry->f_size] = '\0';
				archive_entry_copy_pathname(entry, rdup_entry->f_name);
				rdup_entry->f_name[rdup_entry->f_size] = ' ';

				/* target, +4 == ' -> ' */
				if (S_ISLNK(rdup_entry->f_mode))
					archive_entry_copy_symlink(entry, 
						rdup_entry->f_name + rdup_entry->f_size + 4);
				else 
					archive_entry_copy_hardlink(entry, 
						rdup_entry->f_name + rdup_entry->f_size + 4);
			}
		}

		/* size may be changed - we don't care anymore */
		if (opt_output != O_RDUP) {
			archive_write_header(archive, entry);
		} else {
			rdup_write_header(rdup_entry);
		}

		/* bail out for non regular files */
		if (! S_ISREG(rdup_entry->f_mode) || rdup_entry->f_lnk == 1)
			goto not_s_isreg; 

		/* regular files */
		if ((f = open(rdup_entry->f_name, O_RDONLY)) == -1) {
			msg(_("Could not open '%s\': %s"), rdup_entry->f_name, strerror(errno));
			continue;
		}
		if (child != NULL) {
			pids = create_childeren(child, &pipes, f);
			parent = (g_slist_last(pipes))->data;
			/* everything is closed in create_children */
						
			/* now we must read from from the last pipe 
			 * read end, here, parent[0]
			 */
			len = read(parent[0], readbuf, BUFSIZE);
			if (len == -1) {
				msg("Failure to read from pipe: %s", strerror(errno));
				goto write_plain_file;
			}

			if (wait_pids(pids, WNOHANG) == -1) {
				/* weird child exit */
				msg("Child exit, giving you the original file");
				goto write_plain_file;
			}
			/* close f here as we might need if the 
			 * 'goto write_plain_file'
			 * where we happily read from that descriptor
			 */
			close(f); 
			while (len > 0) {
				/* write archive */
				if (sig != 0) {
					signal_abort(sig);
				}
				 
				if (opt_output == O_RDUP) 
					rdup_write_data(rdup_entry, readbuf, len);
				else
					archive_write_data(archive, readbuf, len);
				
				len = read(parent[0], readbuf, BUFSIZE);
			}
			close(parent[0]);  /* we're done */
			if (wait_pids(pids, 0) == -1) {
				/* weird child exit */
				/* Huh and now?   */
			}

		} else {

write_plain_file:
			/* header already sent, don't care about file size
			 * so this is ok
			 */
				
			/* if we had child trouble we need to 
			 * reset the file as some child might
			 * have read from it */
			if (lseek(f, 0, SEEK_SET)  == -1) {
				msg("Failure to rewind...");
				exit(EXIT_FAILURE);
			}
			len = read(f, readbuf, BUFSIZE);
			if (len == -1) {
				msg("Failure to read from file: %s", strerror(errno));
				exit(EXIT_FAILURE); 
			}

			while (len > 0) {
				if (sig != 0) {
					close(f);
					signal_abort(sig);
				}

				if (opt_output == O_RDUP) {
					rdup_write_data(rdup_entry, readbuf, len);
				} else {
					archive_write_data(archive, readbuf, len);
				}
				len = read(f, readbuf, BUFSIZE);
			}
			close(f);
		}
		/* final block for rdup output */
		if (opt_output == O_RDUP)
			block_out_header(NULL, 0, 1);

not_s_isreg: 
		if (opt_output != O_RDUP)
			archive_entry_free(entry);
	}
	if (opt_output != O_RDUP) {
		archive_write_close(archive);
		archive_write_finish(archive);
	}
	g_free(readbuf);
	g_free(buf);
	g_free(rdup_entry);
}

int
main(int argc, char **argv)
{
	struct sigaction sa;
	char		 pwd[BUFSIZE + 1];
	int		 c, i;
	char		 *q, *r;
	GSList		 *child    = NULL;		/* forked childs args: -P option */
	char		 **args;
	int		 childs    = 0;			/* number of childs */
	
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

	while ((c = getopt (argc, argv, "cP:O:t:LhVvX:Y:")) != -1) {
		switch (c) {
			case 'c':
				opt_tty = TRUE;
				break;
			case 'v':
				opt_verbose++;
				break;
			case 'L':
				opt_input = I_LIST;
				break;
			case 'P':
				/* allocate new for each child */
				args = g_malloc((MAX_CHILD_OPT + 2) * sizeof(char *));
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
				childs++;
				break;
			case 'O':
				opt_output = O_NONE;

				if (strcmp(optarg, o_fmt[O_TAR]) == 0)
					opt_output = O_TAR;
				if (strcmp(optarg, o_fmt[O_CPIO]) == 0)
					opt_output = O_CPIO;
				if (strcmp(optarg, o_fmt[O_PAX]) == 0)
					opt_output = O_PAX;
				if (strcmp(optarg, o_fmt[O_RDUP]) == 0)
					opt_output = O_RDUP;

				if (opt_output == O_NONE) {
					msg("Invalid output format: `%s\'", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'X':
				/* read from file XXX */
				opt_crypt_key = "0123456789abcdef";

				struct aes_ctx *c;
				c = crypt_init(opt_crypt_key, 16, TRUE);
				fprintf(stderr, "crypto to test `%s\'", crypt_path(c, "/home", 5));

				exit(0);
				break;
			case 'Y':
				/* read from file XXX */
				opt_decrypt_key = "0123456789abcdef";
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

	/* read stdin, create childeren and make an archive */
	stdin2archive(child);
	exit(EXIT_SUCCESS);
}
