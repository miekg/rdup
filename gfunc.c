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
extern char *opt_format;
extern time_t opt_timestamp;
extern size_t opt_size;
extern sig_atomic_t sig;

static gboolean
cat(FILE *fp, char *filename)
{
	char buf[BUFSIZE + 1];
	FILE *file;
	size_t i;
		
	if ((file = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "** Could not open '%s\': %s\n",
				filename, strerror(errno));
		return FALSE;
	}
	
	while (!feof(file)) {
		i = fread(buf, sizeof(char), BUFSIZE, file);
		if (fwrite(buf, sizeof(char), i, fp) != i) {
			fprintf(stderr, "** Write failure %s\n",
					strerror(errno));
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * we received a signal 
 */
static void
signal_abort(int signal)
{
	switch(signal) {
		case SIGPIPE:
			fprintf(stderr, "** SIGPIPE received, exiting\n");
			break;
		case SIGINT:
			fprintf(stderr, "** SIGINT received, exiting\n");
			break;
		default:
			fprintf(stderr, "** Unknown signal reveived, exiting\n");
			break;
	}
	exit(EXIT_FAILURE);
}

/**
 * print a struct entry
 *
static void
entry_print(FILE *fp, char plusmin, struct entry *e) {
	switch(opt_contents) {
		case FALSE:
		fprintf(fp, "%c%d %d %d %zd %zd %s",
				plusmin,
				e->f_mode,
				e->f_uid,
				e->f_gid,
				e->f_name_size,
				(size_t)e->f_size,
				e->f_name);
		break;
		case TRUE:
		* do some magic here:
		 * directories -> size to 0 no content
		 * files -> normal size + content
		 * links -> size target path len + target path
		 *
		if (S_ISDIR(e->f_mode)) {
			fprintf(fp, "%c%d %d %d %zd %d %s",
					plusmin,
					e->f_mode,
					e->f_uid,
					e->f_gid,
					e->f_name_size,
					0,
					e->f_name);
			break;
		}
		if (S_ISREG(e->f_mode)) {
			fprintf(fp, "%c%d %d %d %zd %zd %s",
					plusmin,
					e->f_mode,
					e->f_uid,
					e->f_gid,
					e->f_name_size,
					(size_t)e->f_size,
					e->f_name);
			* only print content when we're adding *
			if (plusmin == '+' && !cat(fp, e->f_name)) {
				exit(EXIT_FAILURE);
			}
			break;
		}
		if (S_ISLNK(e->f_mode)) {
			* only print content when we're adding *
			if (plusmin == '+') {
				char buf[BUFSIZE + 1];
				size_t i;
				if ((i = readlink(e->f_name, buf, BUFSIZE)) == -1) {
					fprintf(stderr, "** Error reading link %s %s\n", e->f_name,
					strerror(errno));
					exit(EXIT_FAILURE);
				}
				buf[i + 1] = '\0';
				fprintf(fp, "%c%d %d %d %zd %zd %s%s",
						plusmin,
						e->f_mode,
						e->f_uid,
						e->f_gid,
						e->f_name_size,
						i,
						e->f_name,
						buf);
			} else {
				fprintf(fp, "%c%d %d %d %zd %zd %s",
						plusmin,
						e->f_mode,
						e->f_uid,
						e->f_gid,
						e->f_name_size,
						(size_t)e->f_size,
						e->f_name);
			}
			break;
		}

	}
} */


/**
 * print an escape sequence correctly 
 */
static void
entry_print_escape(char n, FILE *out) {
switch (n) {
	case 'a': fputc('\a', out); break;
	case 'b': fputc('\b', out); break;
	case 'e': fputc('\e', out); break;
	case 'f': fputc('\f', out); break;
	case 'r': fputc('\r', out); break;
	case 't': fputc('\t', out); break;
	case 'v': fputc('\v', out); break;
	case '0': fputc('\0', out); break;

	case 'n':
		/* reverse compatiblity: when -0 is on, put out a NULL. */
		if (opt_null) fputc('\0', out);
			else fputc('\n', out); 
		break;

	default:
		fputc(n, out); 
		break;
	}

return;
}

/**
 * print arbitrary data field 
 */
static void
entry_print_data(FILE *out, char n, struct entry *e) {
	switch (n) {
		case 'n': 
			fputs(e->f_name, out);		
			break;
		case 'l': 
			fprintf(out, "%zd", e->f_name_size);	
			break;
		case 'u': 
			fprintf(out, "%d", e->f_uid);		
			break;
		case 'g': 
			fprintf(out, "%d", e->f_gid);		
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
		case 'C': /* file contents */
			if (!cat(out, e->f_name)) {
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fputc(' ', out);
			break;
	}

	return;
}

/**
 * print function
 */
void 
entry_print(FILE *out, char plusmin, struct entry *e)
{
	char *pos;
	if ((plusmin == '+') && (opt_modified == FALSE)) 
		return;
	if ((plusmin == '-') && (opt_removed == FALSE)) 
		return;

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
	return;
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
	/* name is not freed - this lead to double frees */
	g_free(f);
	return FALSE;
}

/**
 * Write our internal filelist
 */
gboolean 
gfunc_write(gpointer data, __attribute__((unused)) gpointer value, gpointer fp)
{
	if (sig != 0) {
		signal_abort(sig);
	}
	/* mode_path */
	/* this is used to create our filelist */
	fprintf((FILE*) fp, "%d %s", 
			(int) ((struct entry*)data)->f_mode,
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
	if (sig != 0) {
		signal_abort(sig);
	}

	if (S_ISDIR(((struct entry*)data)->f_mode)) {
		entry_print(stdout, '+', (struct entry*)data);
		/*if (opt_null) {
			putc('\0', stdout);
		} else {
			putc('\n', stdout);
		}*/
		return FALSE;
	} 
	if (S_ISREG(((struct entry*)data)->f_mode) ||
			S_ISLNK(((struct entry*)data)->f_mode)) {
		switch (opt_timestamp) {
			case NULL_DUMP:
				if (opt_size != 0 &&
						((struct entry*)data)->f_size >
						opt_size) {
					return FALSE;
				}
				entry_print(stdout, '+', (struct entry*)data);
				/*if (opt_null) {
					putc('\0', stdout);
				} else {
					putc('\n', stdout);
				}*/
				return FALSE;
			default: /* INC_DUMP */
				if (((struct entry*)data)->f_mtime > opt_timestamp) {
					entry_print(stdout, '+', (struct entry*)data);
					/*if (opt_null) {
						putc('\0', stdout);
					} else {
						putc('\n', stdout);
					}*/
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
	if (sig != 0) {
		signal_abort(sig);
	}

	entry_print(stdout, '-', (struct entry*)data);
/*
	if (opt_null) {
		putc('\0', stdout);
	} else {
		putc('\n', stdout);
	}
*/
	return FALSE;
}

/**
 * decide whether 2 struct entries are equal or not
 */
gint
gfunc_equal(gconstpointer a, gconstpointer b)
{
	gint e;

	if (sig != 0) {
		signal_abort(sig);
	}

	e = strcmp(((struct entry*)a)->f_name,
			((struct entry*)b)->f_name);

	if (e == 0) {
		if (((struct entry*)a)->f_mode == 
				((struct entry*)b)->f_mode) {
			return 0;
		}
	}
	return e;
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

	if (sig != 0) {
		signal_abort(sig);
	}

	v = g_tree_lookup((GTree*)((struct substract*)diff)->b, data);

	if (!v) {
		g_tree_replace(((struct substract*)diff)->d, data, value);
	}
	return FALSE;
}

