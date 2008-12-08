/* 
 * Copyright (c) 2009 Miek Gieben
 * parse_entry.c
 * parse an standard rdup entry and return a
 * struct r_entry
 */

#include "rdup-tr.h"

extern gint opt_input;
extern gint opt_output;

/*
 * parse a standard rdup output entry
 * +- 0775 1000 1000 18 2947 /home/miekg/bin/tt
 * buf is NULL delimited 
 */
struct r_entry *
parse_entry(char *buf, size_t l, struct stat *s) 
{
	struct r_entry *e;
	gint i;
	char *n, *pos;
	e = g_malloc(sizeof(struct r_entry));
	e->f_ctime = 0;		/* not used in rdup-tr */

	switch (opt_input) {
		case I_LIST:
			if (stat(buf, s) == -1) {
				msg(_("Could not stat path `%s\': %s"), buf, strerror(errno));
				return NULL;
			}
			e->plusmin     = '+';
			e->f_name      = g_strdup(buf);
			e->f_name_size = strlen(buf);
			e->f_mode      = s->st_mode;
			e->f_uid       = s->st_uid;
			e->f_gid       = s->st_gid;
			e->f_size      = s->st_size;
			e->f_dev       = s->st_dev;
			e->f_ino       = s->st_ino;
			e->f_rdev      = s->st_rdev;
			break;

		case I_RDUP:
			if (strlen(buf) < LIST_MINSIZE){
				msg(_("Corrupt entry in filelist at line: %zd"), l);
				return NULL;
			}

			/* not filled with rdup input */
			e->f_dev       = 0;
			e->f_rdev      = 0;
			e->f_ino       = 0;


			/* 1st char should + or - */
			if (buf[0] != '-' && buf[0] != '+') {
				msg("First character should \'-\' or \'+\' at line: %zd", l);
				return NULL;
			}
			e->plusmin = buf[0];

			if (opt_output != O_RDUP && e->plusmin == '-') {
				msg("Removing files it not supported for any output except rdup");
				return NULL;
			}

			/* type */
			switch(buf[1]) {
				case '-': e->f_mode = S_IFREG; break;
				case 'd': e->f_mode = S_IFDIR; break;
				case 'l': e->f_mode = S_IFLNK; break;
				case 'h': e->f_mode = S_IFLNK; e->f_lnk = 1; break;
				case 'c': e->f_mode = S_IFCHR; break;
				case 'b': e->f_mode = S_IFBLK; break;
				case 'p': e->f_mode = S_IFIFO; break;
				case 's': e->f_mode = S_IFSOCK; break;
				default:
					msg("Type must be one of d, l, h, -, c, b, p or s");
					return NULL;
			}
			/* perm */
			i = (buf[3] - 48) * 512 + (buf[4] - 48) * 64 +	/* oct -> dec */
				(buf[5] - 48) * 8 + (buf[6] - 48);
			if (i < 0 || i > 04777) {
				msg("Invalid permissions at line: %zd", l);
				return NULL;
			}
			e->f_mode |= i;
			
			/* uid  */
			n = strchr(buf + 8, ' ');
			if (!n) {
				msg("Malformed input for uid at line: %zd", l);
				return NULL;
			} else {
				*n = '\0';
			}
			e->f_uid = atoi(buf + 8);
			pos = n + 1;

			/* gid */
			n = strchr(pos, ' ');
			if (!n) {
				msg("Malformed input for gid at line: %zd", l);
				return NULL;
			} else {
				*n = '\0';
			}
			e->f_gid = atoi(pos);
			pos = n + 1;

			/* pathname size */
			n = strchr(pos, ' ');
			if (!n) {
				msg("Malformed input for path length at line: %zd", l);
				return NULL;
			}
			e->f_name_size = atoi(pos); /* checks */
			pos = n + 1;

			/* filesize - may be overloaded for rdev */
			n = strchr(pos, ' ');
			if (!n) {
				msg("Malformed input for file size at line: %zd", l);
				return NULL;
			}
			/* atoi? */
			e->f_size = atoi(pos);
			pos = n + 1;

			/* pathname */
			e->f_name      = g_strdup(pos);
			if (strlen(e->f_name) != e->f_name_size) {
				msg("Real pathname length is not equal to pathname length at line: %zd", l);
				return NULL;
			}
			break;
	}
	return e;
}

/* ALmost the same of entry_print_data in gfunc.c, but
 * not quite as we don't break up symlinks (source -> target)
 * for instance
 */
void
rdup_write_header(struct r_entry *e)
{
	char *out;
	char t;

	if (S_ISDIR(e->f_mode)) {
		t = 'd';
	} else if (S_ISCHR(e->f_mode)) {
		t = 'c';
	} else if (S_ISBLK(e->f_mode)) {
		t = 'b';
	} else if (S_ISFIFO(e->f_mode)) {
		t = 'p';
	} else if (S_ISSOCK(e->f_mode)) {
		t = 's';
	} else if (S_ISLNK(e->f_mode)) {
		t = 'l';
	} else {
		if (e->f_lnk == 1) {
			t = 'h';
		} else
			t = '-';
	}

	out = g_strdup_printf("%c%c %.4o %ld %ld %ld %zd %s", 
			e->plusmin,		
			t,
			(int)e->f_mode & F_PERM,
			(unsigned long)e->f_uid,
			(unsigned long)e->f_gid,
			(unsigned long)e->f_name_size,
			(size_t)e->f_size,
			e->f_name);
	write(1, out, strlen(out));
	return;
}

void
rdup_write_data(__attribute__((unused)) struct r_entry *e, char *buf, size_t len) {
	write(1, buf, len);
	return;
}
