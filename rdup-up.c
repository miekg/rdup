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


/* make the object in the fs */
void	/* XXX failure?? */
mk_obj(FILE *in, char *p, struct r_entry *e) 
{
	char     *buf;
	size_t   i, mod, rest;
	char     *s, *t;
	FILE	 *out;

	/* PERMISSIONS!! */
	/* devices sockets and other stuff! */

	/* hardlink */
	if (e->f_lnk || S_ISLNK(e->f_mode)) {
		s = g_strdup(e->f_name);
		t = s + e->f_size + 4; /* ' -> ' */
		s[e->f_size] = '\0';

		if (g_file_test(s, G_FILE_TEST_EXISTS) || g_file_test(e->f_name, G_FILE_TEST_IS_SYMLINK)) {
			rm(s);
		}

		if (S_ISLNK(e->f_mode)) {
			fprintf(stderr, "s %s||%s\n", s, t);
			if (symlink(t, s) == -1) {
				msg("Failed to make symlink: `%s -> %s\': %s", s, t, strerror(errno));
				return;
			}
		} else {
			/* make target also fall in the backup dir */
			t = g_strdup_printf("%s%s", p, s + e->f_size + 4);
			fprintf(stderr, "h %s||%s\n", s, t);
			/* safe in link list with hardlinks */
		}
		return;
	}

	/* for all other objs we can call rm */
	if (g_file_test(e->f_name, G_FILE_TEST_EXISTS)) {
		/* XXX return value */
		rm(e->f_name);
	}

	/* regular file */
	if (S_ISREG(e->f_mode)) {

		if (!(out = fopen(e->f_name, "w"))) {
			msg("Failed to open file `%s\': %s", e->f_name, strerror(errno));
			if (!(out = fopen("/dev/null", "w"))) {
				msg("Failed to open `/dev/null\': %s", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		buf   = g_malloc(BUFSIZE + 1);
		rest = e->f_size % BUFSIZE;	      /* then we need to read this many */
		mod  = (e->f_size - rest) / BUFSIZE;  /* main loop happens mod times */

		/* mod loop */
		for(i = 0; i < mod; i += BUFSIZE) {
			i = fread(buf, sizeof(char), BUFSIZE, in);
			if (fwrite(buf, sizeof(char), i, out) != i) {
				msg(_("Write failure `%s\': %s"), e->f_name, strerror(errno));
				fclose(out);
				return; /* XXX */
			}
		}
		/* rest */
		i = fread(buf, sizeof(char), rest, in);
		if (fwrite(buf, sizeof(char), i, out) != i) {
			msg(_("Write failure `%s\': %s"), e->f_name, strerror(errno));
			fclose(out);
			return; /* XXX */
		}
		g_chmod(e->f_name, e->f_mode);
		return;
	}

	/* directory */
	if (S_ISDIR(e->f_mode)) {
		fprintf(stderr, "d %s\n", e->f_name);
		g_mkdir(e->f_name, e->f_mode);
		return;
	}

	return;
}

/* update the directory with the archive */
void
update(char *path)
{
	struct r_entry *rdup_entry;
	size_t         line, i, pathsize;
	char           *buf, *pathbuf, *n, *p;
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

		if (!(rdup_entry = parse_entry(buf, line, &s, NO_STAT_CONTENT))) {
			exit(EXIT_FAILURE);
		}

		/* we have a valid entry, read the filename */
		pathsize = fread(pathbuf, sizeof(char), rdup_entry->f_name_size, stdin);

		if (pathsize != rdup_entry->f_name_size) {
			msg("Reported name size (%zd) does not match actual name size (%zd)",
					rdup_entry->f_name_size, pathsize);
			exit(EXIT_FAILURE);
		}
		pathbuf[pathsize] = '\0';
		if (pathbuf[0] != '/') {
			msg("Pathname does not start with /: `%s\'", pathbuf);
			exit(EXIT_FAILURE);
		}
		p = g_strdup_printf("%s%s", path, pathbuf);
		rdup_entry->f_name_size += strlen(path);
		if (S_ISLNK(rdup_entry->f_mode) || rdup_entry->f_lnk)
			rdup_entry->f_size += strlen(path);

		rdup_entry->f_name = p;

		mk_obj(stdin, path, rdup_entry);

	}
	/* post-process hardlinks */
	/* mk_hardlink(GSList *hl); */
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
