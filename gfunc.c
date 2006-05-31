/*
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 *
 * g_tree_foreach helper functions 
 */

#include "rdup.h"

extern gboolean opt_null;
extern gboolean opt_removed;
extern gboolean opt_modified;
extern gint opt_verbose;
extern char *opt_format;
extern char qstr[];
extern time_t opt_timestamp;
extern size_t opt_size;
extern sig_atomic_t sig;

/**
 * we received a signal 
 */
static void
signal_abort(int signal)
{
	switch(signal) {
		case SIGPIPE:
			msg("SIGPIPE received, exiting");
			break;
		case SIGINT:
			msg("SIGINT received, exiting");
			break;
		default:
			msg("Unhandled signal reveived, exiting");
			break;
	}
	exit(EXIT_FAILURE);
}

/*
 * cat the files' contents
 */
static gboolean
cat(FILE *fp, char *filename)
{
	char buf[BUFSIZE + 1];
	FILE *file;
	size_t i;
		
	if ((file = fopen(filename, "r")) == NULL) {
		msg("Could not open '%s\': %s", filename, strerror(errno));
		return FALSE;
	}

	while (!feof(file) && (!ferror(file))) {
		if (sig != 0) {
			fclose(file);
			signal_abort(sig);
		}
		
		i = fread(buf, sizeof(char), BUFSIZE, file);
		if (fwrite(buf, sizeof(char), i, fp) != i) {
			msg("Write failure `%s\': %s", filename, strerror(errno));
			fclose(file);
			return FALSE;
		}
	}
	fclose(file);
	return TRUE;
}

/*
 * cat the contents, only when adding and only for files/links
 */
static void
entry_cat_data(FILE *fp, struct entry *e)
{
	if (S_ISREG(e->f_mode)) {
		if (!cat(fp, e->f_name)) {
			exit(EXIT_FAILURE);
		}
		return;
	}
	if (S_ISLNK(e->f_mode)) {
		char buf[BUFSIZE + 1];
		ssize_t i;
		if ((i = readlink(e->f_name, buf, BUFSIZE)) == -1) {
			msg("Error reading link %s: '%s\'", e->f_name, strerror(errno));
			exit(EXIT_FAILURE);
		}
		buf[i] = '\0';
		fprintf(fp, "%s", buf);
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
entry_print_data(FILE *out, char n, struct entry *e) 
{
	switch (n) {
		case 'n': 
			fputs(e->f_name, out);		
			break;
		case 'l': 
			fprintf(out, "%zd", e->f_name_size);	
			break;
		case 'u': 
			fprintf(out, "%zd", (size_t) e->f_uid);		
			break;
		case 'g': 
			fprintf(out, "%zd", (size_t) e->f_gid);		
			break;
		case 'm': 
			fprintf(out, "%d", e->f_mode);	
			break;
		case 't': 
			fprintf(out, "%ld", (unsigned long)e->f_mtime);	
			break;	
		case 's': 
			/* don't report size for directories. */
			if (S_ISDIR(e->f_mode)) {
				putchar('0');
				break;
			}
			fprintf(out, "%zd", (size_t)e->f_size);
			break;
		case 'T': /* file type */
			if (S_ISDIR(e->f_mode)) {
				putchar('d');
			} else if (S_ISLNK(e->f_mode)) {
				putchar('l');
			} else {
				putchar('-');
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
	
	if (!S_ISDIR(e->f_mode)) {
		/* check if the file has changed since we first
		 * visited it. If so, skip it as it will tear
		 * up the entire print. Esp. when also printing
		 * the contents. The recheck here, minimizes the
		 * race, it's NOT GONE!!
		 *
		 * This is not a problem for directories
		 */
		if (lstat(e->f_name, &s) != 0) {
			msg("Could not stat path `%s\': %s", e->f_name, 
				strerror(errno));
			return;
		}
		if (e->f_size != s.st_size) {
			msg("File size has changed, skipping `%s\'", e->f_name);
			return;
		}
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
 * free a struct entry
 */
gboolean 
gfunc_free(gpointer data, __attribute__((unused)) gpointer value, 
		__attribute__((unused)) gpointer usr)
{
	struct entry *f;
	f = (struct entry*) data;
	g_free(f->f_name);
	g_free(f);
	return FALSE;
}

/**
 * Write our internal filelist
 */
gboolean 
gfunc_write(gpointer data, __attribute__((unused)) gpointer value, gpointer fp)
{
	if (sig != 0)
		signal_abort(sig);

	if (value == NO_PRINT) 
		return FALSE;

	/* mode_path */
	/* this is used to create our filelist */
	fprintf((FILE*) fp, "%d %s", (int) ((struct entry*)data)->f_mode, 
			(char*) ((struct entry*)data)->f_name);
	if (opt_null) {
		putc('\0', fp);
	} else {
		putc('\n', fp);
	}
	return FALSE;
}

/**
 * write out the list of to be backupped items
 */
gboolean
gfunc_backup(gpointer data, __attribute__((unused)) gpointer value, 
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
				if (((struct entry*)data)->f_mtime > opt_timestamp) {
					entry_print(stdout, '+', (struct entry*)data);
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
gfunc_remove(gpointer data, __attribute__((unused)) gpointer value, 
		__attribute__((unused)) gpointer usr)
{
	if (sig != 0) 
		signal_abort(sig);

	/* should not have these here!! */
	if (value == NO_PRINT) {
		msg("Internal error: NO_PRINT in remove tree!");
		return FALSE;
	}

	entry_print(stdout, '-', (struct entry*)data);
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
		if (((struct entry*)a)->f_mode == ((struct entry*)b)->f_mode) {
			return 0;
		}
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
		
		g_tree_replace(
			((struct remove_path *)r)->tree, (gpointer) data, NO_PRINT);
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

	if (!v) {
		g_tree_replace(((struct substract*)diff)->d, data, value);
	}
	return FALSE;
}
