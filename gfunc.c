/*
 * Copyright (c) 2005 - 2010 Miek Gieben
 * See LICENSE for the license
 *
 * g_tree_foreach helper functions
 */

#include "rdup.h"
#include "protocol.h"
#include <pcre.h>
#ifdef HAVE_LIBNETTLE
#include <nettle/sha.h>
#else
#define SHA1_DIGEST_SIZE 20
#endif /* HAVE_LIBNETTLE */

extern gboolean opt_removed;
extern gboolean opt_modified;
extern gboolean opt_skip;
extern gboolean opt_atime;
extern gint opt_verbose;
extern char *opt_format;
extern time_t opt_timestamp;
extern size_t opt_size;
extern int sig;
extern GList *list;
extern GSList *child;

/* sha1.c */
int sha1_stream(FILE *stream, void *digest);

/*
 * calculates a files sha1 sum
 */
static gboolean
sha1(FILE *fp, char *filename) 
{
	unsigned char digest[SHA1_DIGEST_SIZE];
	gint i;
	FILE *file;

	if ((file = fopen(filename, "r")) == NULL) {
		msg(_("Could not open '%s\': %s"), filename, strerror(errno));
		return FALSE;
	}
	if (sha1_stream(file, digest) != 0) {
		msg(_("Failed to calculate sha1 digest: `%s\'"), filename);
		fclose(file);
		return FALSE;
	}
	fclose(file);
	for(i = 0; i < SHA1_DIGEST_SIZE; i++) {
		fprintf(fp, "%02x", digest[i]);
	}
	return TRUE;
}
/*
 * cat the files' contents
 */
static gboolean
cat(FILE *fp, char *filename)
{
	char buf[BUFSIZE + 1];
	FILE *file;
	ssize_t i;
	gboolean nullblock = FALSE;
	GSList *pipes, *pids;
	int *parent;

	if ((file = fopen(filename, "r")) == NULL) {
		msg(_("Could not open '%s\': %s"), filename, strerror(errno));
		return FALSE;
	}

	if (child == NULL) {
		while (!feof(file) && (!ferror(file))) {
			if (sig != 0) {
				fclose(file);
				signal_abort(sig);
			}
			i = fread(buf, sizeof(char), BUFSIZE, file);
			if (block_out_header(fp, i, -1) == -1 ||
					block_out(fp, i, buf, -1)) {
				msg(_("Write failure `%s\': %s"), filename, strerror(errno));
				fclose(file);
				return FALSE;
			}

			/* there is no diff between 0 bytes block and a ending block */
			if (i == 0)
				nullblock = TRUE;
		}
		fclose(file);
		
	} else {
		pids = create_childeren(child, &pipes, fileno(file));

		if (!pids) {
			fclose(file);
			return FALSE;
		}

		parent = (g_slist_last(pipes))->data;
		i = read(parent[0], buf, BUFSIZE);
		if (i == -1) {
			msg(_("Failure to read from pipe: %s"), strerror(errno));
			fclose(file);
			return FALSE;
		}
#if 0
		/* use 0 for the flags field */
		if (wait_pids(pids, WNOHANG) == -1) {
			msg(_("Wait pid failure"));
			fclose(file);
			return FALSE;
		}
#endif
		while (i > 0) {
			if (sig != 0)
				signal_abort(sig);

			if (block_out_header(fp, i, -1) == -1 ||
					block_out(fp, i, buf, -1)) {
				msg(_("Write failure `%s\': %s"), filename, strerror(errno));
				fclose(file);
				close(parent[0]);
				return FALSE;
			}

			/* there is no diff between 0 bytes block and a ending block */
			if (i == 0)
				nullblock = TRUE;

			i = read(parent[0], buf, BUFSIZE);
		}
		close(parent[0]);
		fclose(file);
		if (sig != 0)
			signal_abort(sig);
		
		/* wait for our children */
		if (wait_pids(pids, 0) == -1) {
			msg(_("Weird child exit `%s\': %s"), filename, strerror(errno));
			/* weird child exit */
			return FALSE;
		}
	}
	
	if (!nullblock) {
		if (block_out_header(fp, 0, -1) == -1) {
			msg(_("Write failure `%s\': %s"), filename, strerror(errno));
			fclose(file);
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * cat the contents, only when adding and only for files
 */
static void
entry_cat_data(FILE *fp, struct rdup *e)
{
        struct utimbuf ut;
	ut.actime  = e->f_atime;
	ut.modtime = e->f_mtime;

	if (S_ISREG(e->f_mode) && e->f_lnk == 0) {
		if (!cat(fp, e->f_name))
			exit(EXIT_FAILURE);
		
		if (opt_atime) 
			if (utime(e->f_name, &ut) == -1)
				msg(_("Failed to reset atime: '%s\': %s"), e->f_name, strerror(errno));
		return;
	}
}

/**
 * print an escape sequence correctly
 */
static void
entry_print_escape(char n, FILE *out)
{
	switch (n) {
		case 'a':
			fputc('\a', out);
			break;
		case 'b':
			fputc('\b', out);
			break;
		case 'e':
			fputc('\e', out);
			break;
		case 'f':
			fputc('\f', out);
			break;
		case 'r':
			fputc('\r', out);
			break;
		case 't':
			fputc('\t', out);
			break;
		case 'v':
			fputc('\v', out);
			break;
		case '0':
			fputc('\0', out);
			break;
		case 'n':
			fputc('\n', out);
			break;
		default:
			fputc(n, out);
			break;
	}
}

/**
 * print arbitrary data field
 */
static void
entry_print_data(FILE *out, char n, struct rdup *e)
{
	switch (n) {
		case 'n':
			fputs(e->f_name, out);
			if (S_ISLNK(e->f_mode) || e->f_lnk == 1) {
				fputs(" -> ", out);
				fputs(e->f_target, out);
			}
			break;
		case 'N':
			fputs(e->f_name, out);
			break;
		case 'l':
#if 0
			if (S_ISLNK(e->f_mode) || e->f_lnk == 1) {
				fprintf(stderr, "[%s] [%s]\n", e->f_name, e->f_target);
				fprintf(stderr, "%d %d\n", (int)e->f_name_size, strlen(e->f_target));
				fprintf(out, "%ld", (unsigned long)e->f_name_size + 4 + 
						(unsigned long)strlen(e->f_target));
			} else
#endif
				fprintf(out, "%ld", (unsigned long)e->f_name_size);

			break;
		case 'u':
			fprintf(out, "%ld", (unsigned long)e->f_uid);
			break;
		case 'U':
			if (!e->f_user) 
				fprintf(out, "-");
			else
				fprintf(out, "%s", e->f_user);
			break;
		case 'g':
			fprintf(out, "%ld", (unsigned long)e->f_gid);
			break;
		case 'G':
			if (!e->f_group) 
				fprintf(out, "-");
			else
				fprintf(out, "%s", e->f_group);
			break;
		case 'm':
			fprintf(out, "%d", (int)e->f_mode);
			break;
	        case 'b':
                       fprintf(out, "%.4o", (int)e->f_mode & ~S_IFMT);
                       break;
		case 't':
			fprintf(out, "%ld", (unsigned long)e->f_mtime);
			break;
		case 's':
			/* don't report size for directories. */
			if (S_ISDIR(e->f_mode)) {
				fputc('0', out);
				break;
			}
			/* hijack size for major,minor number when special */
			if (S_ISBLK(e->f_mode) || S_ISCHR(e->f_mode)) {
				fprintf(out, "%d,%d", (unsigned int) major(e->f_rdev),(unsigned int) minor(e->f_rdev));
				break;
			}
			/* links */
			if (S_ISLNK(e->f_mode) || e->f_lnk == 1) {
				fprintf(out, "%ld", (unsigned long)e->f_size);
				break;
			}

			fprintf(out, "%zd", (size_t)e->f_size);
			break;
		case 'H': /* sha1 hash */
			if (S_ISREG(e->f_mode)) {
				/* normal file and no hardlink */
				if (e->f_lnk == 0) {
					sha1(out, e->f_name);
				}
			} else {
				fprintf(out, NO_SHA);
			}
			break;
		case 'T': /* file type */
			if (S_ISDIR(e->f_mode)) {
				fputc('d', out);
			} else if (S_ISCHR(e->f_mode)) {
				fputc('c', out);
			} else if (S_ISBLK(e->f_mode)) {
				fputc('b', out);
			} else if (S_ISFIFO(e->f_mode)) {
				fputc('p', out);
			} else if (S_ISSOCK(e->f_mode)) {
				fputc('s', out);
			} else if (S_ISLNK(e->f_mode)) {
				fputc('l', out);
			} else {
				if (e->f_lnk == 1) {
					fputc('h', out);
				} else
					fputc('-', out);
			}
			break;
		default:
			fputc(' ', out);
			break;
	}
}

/**
 * print function
 */
void
entry_print(FILE *out, guint pm, struct rdup *e, char *fmt)
{
	char *pos;
	if ((pm == PLUS) && (opt_modified == FALSE))
		return;

	if ((pm == MINUS) && (opt_removed == FALSE))
		return;

	if (opt_verbose >= 1) {
		fputs("** ", stderr);
		fputc(pm == PLUS ? '+' : '-', stderr);
		fprintf(stderr, " %s", e->f_name);
		if (S_ISLNK(e->f_mode) || e->f_lnk == 1) 
			fprintf(stderr, " -> %s\n", e->f_target);
		else
			fputs("\n", stderr);
	}

	/* next check if we can read the file, if not - skip it and don't emit
	 * anything */
	if (S_ISREG(e->f_mode) && pm == PLUS && e->f_lnk == 0) {
		if (access(e->f_name, R_OK) == -1) {
			msg("Unable to open file `%s\': %s", e->f_name, strerror(errno));
			return;
		}
	}

	for (pos = fmt; *pos != '\0';  ++pos) {
		switch (*pos) {
			/* c-style escapes are valid */
			case '\\':
				++pos;
				entry_print_escape(*pos, out);
				break;
				/* emit data */
			case '%':
				++pos;
				switch (*pos) {
					case '%':
						  fputc('%', out);
						  break;
					case 'p':
						  fputc(pm == PLUS ? '+' : '-', out);
						  break;
					case 'C':
						  if (pm == PLUS) {
						  	entry_cat_data(out, e);
						  }
						  break;
					default:
						  entry_print_data(out, *pos, e);
						  break;
				}
				break;
				/* don't know? echo it. */
			default:
				fputc(*pos, out);
				break;
		}
	}
}

/**
 * Write our internal filelist
 */
gboolean
gfunc_write(gpointer data, gpointer value, gpointer fp)
{
	struct rdup *e = (struct rdup*)data;
	char linktype = '-';
	size_t file_size, name_size;
	gchar *n;

	if (sig != 0)
		signal_abort(sig);

	if (value == NO_PRINT)
		return FALSE;

	if (e->f_lnk == 1)
		linktype = 'h';
	if (S_ISLNK(e->f_mode))
		linktype = 'l';

	file_size = e->f_size;
	name_size = e->f_name_size;

	if (S_ISLNK(e->f_mode) || e->f_lnk == 1)
		n = g_strdup_printf("%s -> %s", e->f_name, e->f_target);
	else
		n = strdup(e->f_name);

	if (S_ISDIR(e->f_mode)) /* the same as in the normal output */
		file_size = 0;

	fprintf((FILE*) fp, "%5ld %ld %ld %c %ld %ld %ld %ld %s", (long int)e->f_mode, 
			(long int)e->f_dev, 
			(long int)e->f_ino, linktype, 
			(long int)e->f_uid, 
			(long int)e->f_gid,
			(long int)name_size, (long int)file_size, n);
	fputc('\n', (FILE*)fp);
	g_free(n);
	return FALSE;
}

/**
 * write out the list of to be backupped items
 */
gboolean
gfunc_backup(gpointer data, gpointer value,
		__attribute__((unused)) gpointer usr)

{
	if (sig != 0)
		signal_abort(sig);

	/* .nobackup seen, don't print it */
	if (value == NO_PRINT) 
		return FALSE;

	if (S_ISDIR(((struct rdup*)data)->f_mode)) {
		entry_print(stdout, PLUS, (struct rdup*)data, opt_format);
		return FALSE;
	} else {
		if (opt_size != 0 && S_ISREG(((struct rdup*)data)->f_mode) &&
			((struct rdup*)data)->f_size > (ssize_t)opt_size) {
			return FALSE;
		}
		switch (opt_timestamp) {
			case NULL_DUMP:
				entry_print(stdout, PLUS, (struct rdup*)data, opt_format);
				return FALSE;
			default: /* INC_DUMP */
				if (((struct rdup*)data)->f_ctime >= opt_timestamp) {
					entry_print(stdout, PLUS, (struct rdup*)data, opt_format);
#if DEBUG
				msgd(__func__, __LINE__,"%s %ld >= %ld diff %ld\n", 
						((struct rdup*)data)->f_name,
						(long int)((struct rdup*)data)->f_ctime, 
						(long int)opt_timestamp,
						(long int)opt_timestamp -
						(long int)((struct rdup*)data)->f_ctime);
#endif /* DEBUG */
				}
				return FALSE;
		}
	}
	return FALSE;
}

/**
 * write out the list of removed items
 */
gboolean
gfunc_remove(gpointer data, gpointer value, 
		__attribute__((unused)) gpointer usr)

{
	if (sig != 0)
		signal_abort(sig);

	/* should not have these here!! */
	if (value == NO_PRINT) {
		msg(_("Internal error: NO_PRINT in remove tree!"));
		return FALSE;
	}
	entry_print(stdout, MINUS, (struct rdup*)data, opt_format);
	return FALSE;
}

/**
 * Print out the list of new item
 */
gboolean
gfunc_new(gpointer data, __attribute__((unused)) gpointer value, 
		__attribute__((unused)) gpointer usr)
{
	if (sig != 0)
		signal_abort(sig);

	/* it is perfectly possibly to have these here */
	if (value == NO_PRINT) 
		return FALSE;

	if (opt_size != 0 && S_ISREG(((struct rdup*)data)->f_mode) &&
			((struct rdup*)data)->f_size > (ssize_t)opt_size) {
		return FALSE;
	}

	entry_print(stdout, PLUS, (struct rdup*)data, opt_format);
	return FALSE;
}

/**
 * decide whether 2 struct entries are equal or not
 * 0 = equal
 */
gint
gfunc_equal(gconstpointer a, gconstpointer b)
{
	gint e;
	struct rdup *ae, *be;

	ae = (struct rdup *)a;
	be = (struct rdup *)b;

	if (sig != 0)
		signal_abort(sig);

	e = strcmp(((struct rdup*)a)->f_name, ((struct rdup*)b)->f_name);
	if (e == 0) {
		if (ae->f_dev != be->f_dev)
			return -1;
		if (ae->f_ino != be->f_ino) 
			return -2;

		/* if we are looking at a directory and only the mode has changed
		 * don't let rdup remove the entire directory */
		if (S_ISDIR(ae->f_mode) && S_ISDIR(be->f_mode) && 
				((ae->f_mode & ~S_IFMT) != (be->f_mode & ~S_IFMT)) )
			return 0;

		if (ae->f_mode != be->f_mode) 
			return -3;
	}
	return e;
}

/**
 * used in the crawler, remove specific paths on finding a .nobackup
 */
gboolean
gfunc_remove_path(gpointer data, gpointer __attribute__((unused)) value, gpointer r)
{
	if (sig != 0)
		signal_abort(sig);

	if (strncmp(((struct rdup*)data)->f_name,
				((struct remove_path *)r)->path, 
				((struct remove_path *)r)->len) == 0) {

		/* don't remove the directory itself */
		if (S_ISDIR( ((struct rdup*)data)->f_mode))
			return FALSE;

		g_tree_insert(
			((struct remove_path *)r)->tree, data, NO_PRINT);
	}
	return FALSE;
}

/**
 * traverse function
 * implement the tree subtraction
 * everything in A, but NOT in b
 * (data := element out of A)
 * (b    := packed in diff, diff->b)
 * This function is essentially the most expensive function
 * in rdup...
 */
gboolean
gfunc_subtract(gpointer data, gpointer value, gpointer diff)
{
	gpointer v;
	if (sig != 0)
		signal_abort(sig);

	v = g_tree_lookup((GTree*)((struct subtract*)diff)->b, data);

	if (!v) 
		g_tree_insert(((struct subtract*)diff)->d, data, value);

	return FALSE;
}

/**
 * Walk the linked list and return TRUE when a regexp
 * matches, otherwise return FALSE
 */
gboolean
gfunc_regexp(GSList *l, char *n, size_t len)
{
        GSList *k;
        pcre *P;
	int ovector[REG_VECTOR];

        for (k = g_slist_nth(l, 0); k; k = k->next) { 
		if (sig != 0)
			signal_abort(sig);

                P = (pcre*) k->data;
		/* pcre_exec errors are all < 0, so >= 0 is some kind
		 * of success
		 */
                if (pcre_exec(P, NULL, n, len, 0, 0, ovector, REG_VECTOR) >= 0)
                        return TRUE;
        }
        return FALSE;
}

/**
 * put an element of the tree in a double linked list
 */
gboolean
gfunc_tree2list(gpointer data, gpointer value, 
		__attribute__((unused)) gpointer l)  
{
        if (sig != 0)
                signal_abort(sig);

	if (value == NO_PRINT)
		return FALSE;

        list = g_list_prepend(list, data);
        return FALSE;
}
