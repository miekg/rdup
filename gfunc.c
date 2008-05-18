/*
 * Copyright (c) 2005 - 2008 Miek Gieben
 * See LICENSE for the license
 *
 * g_tree_foreach helper functions
 */

#include "rdup.h"

extern gboolean opt_null;
extern gboolean opt_removed;
extern gboolean opt_modified;
extern gboolean opt_skip;
extern gboolean opt_local;
extern gint opt_verbose;
extern char *opt_format;
extern char qstr[];
extern time_t opt_timestamp;
extern size_t opt_size;
extern sig_atomic_t sig;

/* sha1.c */
int sha1_stream(FILE *stream, void *resblock);

/*
 * calculates a files sha1 sum
 */
static gboolean
sha1(FILE *fp, char *filename) 
{
	unsigned char digest[SHA1_LEN];
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
	for(i = 0; i < SHA1_LEN; i++) {
		fprintf(fp, "%02x", digest[i]);
	}
	return TRUE;
}
/*
 * cat the files' contents
 */
static gboolean
cat(FILE *fp, char *filename, off_t f_size)
{
	char buf[BUFSIZE + 1];
	FILE *file;
	size_t i;
	size_t t;
	size_t missing;

	if ((file = fopen(filename, "r")) == NULL) {
		msg(_("Could not open '%s\': %s"), filename, strerror(errno));
		return FALSE;
	}

	t = 0;
	while (!feof(file) && (!ferror(file))) {
		if (sig != 0) {
			fclose(file);
			signal_abort(sig);
		}
		i = fread(buf, sizeof(char), BUFSIZE, file);
		t += i;
		if (t > (size_t) f_size) {
			/* the file has grown. Break off the write!! */
			msg(_("File grown larger (%zd) than original file size (%zd) , cutting off: `%s\'"), t, (size_t) f_size, filename);
			/* what's missing and what is read in the previous read */
			missing = t - i;
			missing = (size_t) f_size - missing;
			if (missing > 0) {
				/* write the missing bytes till f_size */
				if (fwrite(buf, sizeof(char), missing, fp) != missing) {
					msg(_("Write failure `%s\': %s"), filename, strerror(errno));
					fclose(file);
					return FALSE;
				}
			}
			fclose(file);
			return TRUE;
		}

		if (fwrite(buf, sizeof(char), i, fp) != i) {
			msg(_("Write failure `%s\': %s"), filename, strerror(errno));
			fclose(file);
			return FALSE;
		}
	}
	fclose(file);

	/* file has shrunken! Fill the rest with NULLs, this works
	 * but is slow! */
	if (t < (size_t) f_size) {
		msg(_("File has shrunk, filling with NULLs: `%s\'"), filename);
		for(i = t; i < (size_t) f_size; i++) {
			fputc('\0', fp);
		}
	}
	return TRUE;
}

/*
 * cat the contents, only when adding and only for files/links
 */
static void
entry_cat_data(FILE *fp, struct entry *e)
{
	if (S_ISREG(e->f_mode) && e->f_lnk == 0) {
		if (!cat(fp, e->f_name, e->f_size)) {
			exit(EXIT_FAILURE);
		}
		return;
	}
	/* special case for links isn't needed anymore
	 * link name is now embedded in the path: link -> target
	 */
#if 0
	if (S_ISLNK(e->f_mode)) {
		char buf[BUFSIZE + 1];
		ssize_t i;
		if ((i = readlink(e->f_name, buf, BUFSIZE)) == -1) {
			msg(_("Error reading link `%s\': %s"), e->f_name, strerror(errno));
			exit(EXIT_FAILURE);
		}
		buf[i] = '\0';
		fprintf(fp, "%s", buf);
		return;
	}
#endif
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
entry_print_data(FILE *out, char n, struct entry *e)
{
	switch (n) {
		case 'n':
			fputs(e->f_name, out);
			break;
		case 'l':
			fprintf(out, "%ld", (unsigned long)e->f_name_size);
			break;
		case 'u':
			fprintf(out, "%ld", (unsigned long)e->f_uid);
			break;
		case 'g':
			fprintf(out, "%ld", (unsigned long)e->f_gid);
			break;
		case 'm':
			fprintf(out, "%d", (int)e->f_mode);
			break;
	        case 'b':
                       fprintf(out, "%.4o", (int)e->f_mode & F_PERM);
                       break;
		case 't':
			fprintf(out, "%ld", (unsigned long)e->f_ctime);
			break;
		case 's':
			/* don't report size for directories. */
			if (S_ISDIR(e->f_mode)) {
				fputc('0', out);
				break;
			}
			fprintf(out, "%zd", (size_t)e->f_size);
			break;
		case 'H': /* sha1 hash */
			if (S_ISREG(e->f_mode))
				/* normal file and no hardlink */
				if (e->f_lnk == 0) 
					sha1(out, e->f_name);
			else
				fprintf(out, NO_SHA);
			break;
		case 'T': /* file type */
			if (S_ISDIR(e->f_mode)) {
				fputc('d', out);
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
entry_print(FILE *out, char plusmin, struct entry *e)
{
	char *pos;
	struct stat s;

	if ((plusmin == '+') && (opt_modified == FALSE)) {
		return;
	}

	if ((plusmin == '-') && (opt_removed == FALSE)) {
		return;
	}

	if (opt_verbose > 1) {
		fputs("** ", stderr);
		fputc(plusmin, stderr);
		fprintf(stderr, " %s\n", e->f_name);
	}

	if (S_ISREG(e->f_mode) && plusmin == '+' && !opt_local
		&& e->f_lnk == 0) {
		/* check if the file has changed since we first
		 * visited it. If so, skip it as it will tear
		 * up the entire print. Esp. when also printing
		 * the contents. The recheck here, minimizes the
		 * race, it's NOT GONE!!
		 *
		 * This is not a problem for directories, nor sym- and
		 * hard links
		 */
		if (lstat(e->f_name, &s) != 0) {
			msg(_("Could not stat path `%s\': %s"), e->f_name,
				strerror(errno));
			return;
		}
#ifndef _DEBUG_RACE
		if (e->f_size != s.st_size) {
			msg(_("File size has changed, skipping `%s\'"), e->f_name);
			return;
		}
#endif /* _DEBUG_RACE */
	}

	for (pos = opt_format; *pos != '\0';  ++pos) {
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
						  fputc(plusmin, out);
						  break;
					case 'C':
						  if (plusmin == '+') {
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
	struct entry *e = (struct entry*)data;
	if (sig != 0)
		signal_abort(sig);

	if (value == NO_PRINT)
		return FALSE;

	fprintf((FILE*) fp, "%ld %ld %ld %ld %s", (long int)e->f_mode, (long int)e->f_dev, 
			(long int)e->f_ino, (long int)e->f_name_size, e->f_name);
	if (opt_null) {
		fputc('\0', (FILE*)fp);
	} else {
		fputc('\n', (FILE*)fp);
	}
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

	if (S_ISDIR(((struct entry*)data)->f_mode)) {
		entry_print(stdout, '+', (struct entry*)data);
		return FALSE;
	}
	if (S_ISREG(((struct entry*)data)->f_mode) ||
			S_ISLNK(((struct entry*)data)->f_mode)) {

		if (opt_size != 0 && ((struct entry*)data)->f_size > (ssize_t)opt_size) {
			return FALSE;
		}
		switch (opt_timestamp) {
			case NULL_DUMP:
				entry_print(stdout, '+', (struct entry*)data);
				return FALSE;
			default: /* INC_DUMP */
				if (((struct entry*)data)->f_ctime > opt_timestamp)
					entry_print(stdout, '+', (struct entry*)data);
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

	entry_print(stdout, '-', (struct entry*)data);
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

	entry_print(stdout, '+', (struct entry*)data);
	return FALSE;
}

/**
 * decide whether 2 struct entries are equal or not
 */
gint
gfunc_equal(gconstpointer a, gconstpointer b)
{
	gint e;

	if (sig != 0)
		signal_abort(sig);

	e = strcmp(((struct entry*)a)->f_name, ((struct entry*)b)->f_name);
	if (e == 0) {
		if (((struct entry*)a)->f_dev != ((struct entry*)b)->f_dev)
			return -1;
		if (((struct entry*)a)->f_ino != ((struct entry*)b)->f_ino)
			return -1;
		if (((struct entry*)a)->f_mode == ((struct entry*)b)->f_mode)
			return 0;
	}
	return e;
}

/**
 * used in the crawler, remove specific paths
 */
gboolean
gfunc_remove_path(gpointer data, gpointer __attribute__((unused)) value, gpointer r)
{
	if (sig != 0)
		signal_abort(sig);

	if (!strncmp(((struct entry*)data)->f_name,
				((struct remove_path *)r)->path,
				((struct remove_path *)r)->len)) {

		/* don't remove the directory itself */
		if (S_ISDIR( ((struct entry*)data)->f_mode))
			return FALSE;

		g_tree_insert(
			((struct remove_path *)r)->tree, data, NO_PRINT);
	}
	return FALSE;
}

/**
 * traverse function
 * implement the tree substraction
 * everything in A, but NOT in b
 * (data := element out of A)
 * (b    := packed in diff, diff->b)
 * This function is essentially the most expensive function
 * in rdup...
 */
gboolean
gfunc_substract(gpointer data, gpointer value, gpointer diff)
{
	gpointer v;
	if (sig != 0)
		signal_abort(sig);

	v = g_tree_lookup((GTree*)((struct substract*)diff)->b, data);

	if (!v) 
		g_tree_insert(((struct substract*)diff)->d, data, value);

	return FALSE;
}

/**
 * Walk the linked list and return TRUE when a regexp
 * matches, otherwise return FALSE
 */
gboolean
gfunc_regexp(GSList *l, char *n)
{
        GSList *k;
        regex_t *R;
	int e;

        for (k = g_slist_nth(l, 0); k; k = g_slist_next(k)) { 
		if (sig != 0)
			signal_abort(sig);

                R = (regex_t*) k->data;
                if ((e = regexec(R, n, 0, NULL, 0)) == 0)
                        return TRUE;
        }
        return FALSE;
}
