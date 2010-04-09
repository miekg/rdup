/*
 * Copyright (c) 2009,2010 Miek Gieben
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
#ifdef HAVE_LIBNETTLE
gchar *opt_crypt_key	   = NULL;			/* encryption key */
gchar *opt_decrypt_key	   = NULL;			/* decryption key */
struct aes_ctx * aes_ctx   = NULL;
#endif /* HAVE_LIBNETTLE */
gint opt_verbose 	   = 0;                         /* be more verbose */
gint opt_output	           = O_RDUP;			/* default output */
gint opt_input		   = I_RDUP;			/* default intput */

int sig           = 0;
char *o_fmt[] = { "", "tar", "cpio", "pax", "rdup"};	/* O_NONE, O_TAR, O_CPIO, O_PAX, O_RDUP */
extern int opterr;

int opterr = 0;

/* common.c */
struct rdup * entry_dup(struct rdup *f);
void entry_free(struct rdup *f);

#ifdef HAVE_LIBNETTLE
/* encrypt an rdup_entry (just the path of course) */
static struct rdup *
crypt_entry(struct rdup *e, GHashTable *tr)
{
        gchar *crypt, *dest;
	if (! (crypt = crypt_path(aes_ctx,e->f_name, tr))) {
		msg(_("Failed to encrypt path `%s\'"), e->f_name);
		return NULL;
	}

	e->f_name = crypt;
	e->f_name_size = strlen(crypt);

	/* links are special */
	if (S_ISLNK(e->f_mode) || e->f_lnk == 1) {
		dest = crypt_path(aes_ctx, e->f_target, tr);
		e->f_target = dest;
		e->f_size = strlen(crypt); /* use crypt here */
	}
	return e;
}

/* decrypt an rdup_entry */
static struct rdup *
decrypt_entry(struct rdup *e, GHashTable *tr)
{
        gchar *plain, *dest;

	if (! (plain = decrypt_path(aes_ctx, e->f_name, tr))) {
		msg(_("Failed to decrypt path `%s\'"), e->f_name);
		return NULL;
	}

	e->f_name = plain;
	e->f_name_size = strlen(plain);

	/* links are special */
	if (S_ISLNK(e->f_mode) || e->f_lnk == 1) {
		dest = decrypt_path(aes_ctx, e->f_target, tr);
		e->f_target = dest;
		e->f_size = strlen(plain);
	}
        return e;
}
#endif /* HAVE_LIBNETTLE */

/* read filenames from stdin, put them through
 * the childeren, collect the output from the last
 * child and create the archive on stdout
 */
static void
stdin2archive(void)
{
	char		*buf, *fbuf, *readbuf, *n, *pathbuf;
	char		delim;
	size_t		i, line, pathsize;
	ssize_t		bytes;
	int		j;
	struct archive  *archive;
	struct archive_entry *entry;
	struct stat     *s;
	struct rdup  *rdup_entry = NULL;
	GSList *hlinks = NULL;
	GSList *hl     = NULL;
	GHashTable *trhash;		/* look up for encrypted/decrypted strs */

	delim   = '\n';
	i       = BUFSIZE;
	buf     = g_malloc(BUFSIZE + 1);
	fbuf	= g_malloc(BUFSIZE + 1);
	readbuf = g_malloc(BUFSIZE + 1);
	pathbuf = g_malloc(BUFSIZE + 1);
	j	= ARCHIVE_OK;
	entry   = NULL;
	line    = 0;
	trhash  = g_hash_table_new(g_str_hash, g_str_equal);

	if (opt_output == O_RDUP) {
		archive = NULL;
	} else {
		if ( (archive = archive_write_new()) == NULL) {
			msg(_("Failed to create archive"));
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
				j = ARCHIVE_OK;
				break;
		}

		if (j != ARCHIVE_OK) {
			msg(_("Failed to set archive type to %s"), o_fmt[opt_output]);
			exit(EXIT_FAILURE);
		} else {
			archive_write_open_fd(archive, 1);
		}
	}

	while ((rdup_getdelim(&buf, &i, delim, stdin)) != -1) {
		line++;
		n = strrchr(buf, '\n');
		if (n)
			*n = '\0';

		if (sig != 0)
			signal_abort(sig);

		if (!(rdup_entry = parse_entry(buf, line))) {
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
                        // filesize is spot where to cut and set new size
                        rdup_entry->f_name[rdup_entry->f_size] = '\0';
			rdup_entry->f_name_size = strlen(rdup_entry->f_name);
                        rdup_entry->f_target = rdup_entry->f_name +
                                rdup_entry->f_size + 4;
                } else {
                        rdup_entry->f_target = NULL;
                }

		if (opt_verbose > 0) {
			if (S_ISLNK(rdup_entry->f_mode) || rdup_entry->f_lnk) {
				fprintf(stderr, "%s -> %s\n", rdup_entry->f_name, rdup_entry->f_target);
			} else {
				fprintf(stderr, "%s\n", rdup_entry->f_name);
			}
		}

		if (sig != 0)
			signal_abort(sig);

		rdup_entry = rdup_entry;
#ifdef HAVE_LIBNETTLE
		if (opt_crypt_key)
			rdup_entry = crypt_entry(rdup_entry, trhash);
		if (opt_decrypt_key)
			rdup_entry = decrypt_entry(rdup_entry, trhash);

		if (!rdup_entry)
			exit(EXIT_FAILURE); /* encryption problem */

#endif /* HAVE_LIBNETTLE */

		if (rdup_entry->plusmin == MINUS) {
			if (opt_output == O_RDUP) {
				(void)rdup_write_header(rdup_entry);
				goto not_s_isreg;
			}
			continue;
		}

		if (opt_output != O_RDUP) {
			if (rdup_entry->f_lnk == 1) {
				/* hardlinks must come last */
				hlinks = g_slist_append(hlinks, entry_dup(rdup_entry));
				continue;
			}

			s = stat_from_rdup(rdup_entry);
			entry = archive_entry_new();
			archive_entry_copy_stat(entry, s);
			archive_entry_copy_pathname(entry, rdup_entry->f_name);

			if (S_ISLNK(rdup_entry->f_mode))
				archive_entry_copy_symlink(entry, rdup_entry->f_target);
		}

		/* size may be changed - we don't care anymore */
		if (opt_output != O_RDUP)
			archive_write_header(archive, entry);
		else
			(void)rdup_write_header(rdup_entry);

		/* bail out for non regular files */
		if (! S_ISREG(rdup_entry->f_mode) || rdup_entry->f_lnk == 1)
			goto not_s_isreg;

		/* regular files */
		while ((bytes = block_in_header(stdin)) > 0) {
			if (block_in(stdin, bytes, fbuf) == -1) {
				msg(_("Failure to read from stdin: %s"), strerror(errno));
				exit(EXIT_FAILURE);
			}

			if (sig != 0)
				signal_abort(sig);

			if (opt_output == O_RDUP)
				(void)rdup_write_data(rdup_entry, fbuf, bytes);
			else
				archive_write_data(archive, fbuf, bytes);
		}

		/* final block for rdup output */
		if (opt_output == O_RDUP)
			block_out_header(NULL, 0, 1);

not_s_isreg:
		if (opt_output != O_RDUP)
			archive_entry_free(entry);

	}
	/* output hardlinks -- if any, is zero for O_RDUP */
	for (hl = g_slist_nth(hlinks, 0); hl; hl = hl->next) {
		s = stat_from_rdup((struct rdup*)hl->data);
		entry = archive_entry_new();
		archive_entry_copy_stat(entry, s);
		archive_entry_copy_pathname(entry, ((struct rdup *)hl->data)->f_name);
		archive_entry_copy_hardlink(entry, ((struct rdup *)hl->data)->f_target);
		archive_write_header(archive, entry);
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
	int		 c;
	
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
				msg(_("Functionality moved to rdup"));
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
					msg(_("Invalid output format: `%s\'"), optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'X':
#ifdef HAVE_LIBNETTLE
				if (opt_decrypt_key) {
					msg(_("Will not do both encryption and decryption"));
					exit(EXIT_FAILURE);
				}
				if (! (opt_crypt_key = crypt_key(optarg)) )
					exit(EXIT_FAILURE);

				aes_ctx = crypt_init(opt_crypt_key, TRUE);
				if (!aes_ctx)
					exit(EXIT_FAILURE);
#else
				msg(_("Compiled without encryption, can not encrypt"));
				exit(EXIT_FAILURE);
#endif /* HAVE_LIBNETTLE */
				break;
			case 'Y':
#ifdef HAVE_LIBNETTLE
				if (opt_crypt_key) {
					msg(_("Can not do both encryption and decryption"));
					exit(EXIT_FAILURE);
				}
				if (! (opt_decrypt_key = crypt_key(optarg)) )
					exit(EXIT_FAILURE);

				aes_ctx = crypt_init(opt_decrypt_key, FALSE);
				if (!aes_ctx)
					exit(EXIT_FAILURE);
#else
				msg(_("Compiled without encryption, can not decrypt"));
				exit(EXIT_FAILURE);
#endif /* HAVE_LIBNETTLE */
				break;
			case 'h':
				usage_tr(stdout);
				exit(EXIT_SUCCESS);
			case 'V':
#ifdef DEBUG
                                fprintf(stdout, "%s %s (with --enable-debug)\n", PROGNAME, VERSION);
#else
                                fprintf(stdout, "%s %s\n", PROGNAME, VERSION);
#endif /* DEBUG */
				exit(EXIT_SUCCESS);
			default:
				msg(_("Unknown option seen `%c\'"), optopt);
				exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (!opt_tty && isatty(1) == 1) {
		msg(_("Will not write to a tty"));
		exit(EXIT_FAILURE);
	}

	/* read stdin and (re)make an archive */
	stdin2archive();
	exit(EXIT_SUCCESS);
}
