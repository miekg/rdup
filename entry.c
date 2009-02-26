/* 
 * Copyright (c) 2009 Miek Gieben
 * parse_entry.c
 * parse an standard rdup entry and return a
 * struct r_entry
 */

#include "rdup-tr.h"
#include "protocol.h"
#include "io.h"

extern gint opt_input;
extern gint opt_output;
extern gchar *opt_crypt_key;
extern gchar *opt_decrypt_key;

/*
 * parse a standard rdup output entry
 * +- 0775 1000 1000 18 2947 /home/miekg/bin/tt
 *
 * or parse a standard rdup -c output entry
 * +- 0775 1000 1000 18 2947\n
 * /home/miekg/bin/tt
 * <contents>
 *
 * or parse a new style rdup -c output entry
 * +- 0775 1000 1000 18 2947\n
 * /home/miekg/bin/tt
 * <contents>
 *
 * where contents is block based
 * 1BLOCK8192
 * 8192 bytes of data
 * 1BLOCK15
 * 15 bytes of data
 * 1BLOCK0
 * the-end
 *
 *
 * buf is NULL delimited 
 *
 * stat is extra and is used by rdup-up to say that it
 * wants to parse rdup -c ouput. This cannot be handled
 * with opt_input because rdup-tr cannot handle this
 * so opt_input is for normal rdup inupt and
 *
 * XXX could use a cleanup
 */
struct r_entry *
parse_entry(char *buf, size_t l, struct stat *s, gint stat) 
{
	struct r_entry *e;
	gint i;
	gint j;
	char *n, *pos;
	e = g_malloc(sizeof(struct r_entry));
	e->f_ctime = 0;		/* not used in rdup-* */
	
	switch (opt_input) {
		case I_LIST:
			if (lstat(buf, s) == -1) {
				msg(_("Could not stat path `%s\': %s"), buf, strerror(errno));
				return NULL;
			}
			e->plusmin     = PLUS;
			e->f_name      = g_strdup(buf);
			e->f_name_size = strlen(buf);
			e->f_mode      = s->st_mode;
			e->f_uid       = s->st_uid;
			e->f_gid       = s->st_gid;
			e->f_size      = s->st_size;
			e->f_dev       = s->st_dev;
			e->f_ino       = s->st_ino;
			e->f_rdev      = s->st_rdev;
			e->f_lnk       = 0;

			/* you will loose hardlink information here 
			 * as 'stat' cannot check this */
			if (S_ISLNK(e->f_mode)) 	
				e = sym_link(e, NULL);

			break;

		case I_RDUP:
			if (strlen(buf) < LIST_MINSIZE){
				msg(_("Corrupt entry `%s\' in input at line: %zd"), buf, l);
				return NULL;
			}

			/* defaults */
			e->f_dev  = 0;
			e->f_rdev = 0;
			e->f_ino  = 0;
			e->f_lnk  = 0;

			/* 1st char should + or - */
			if (buf[0] != '-' && buf[0] != '+') {
				msg("First character should \'-\' or \'+\', `%s\' at line: %zd", buf, l);
				return NULL;
			}
			if (buf[0] == '+')
				e->plusmin = PLUS;
			if (buf[0] == '-')
				e->plusmin = MINUS;

			if (opt_output != O_RDUP && e->plusmin == MINUS) {
				msg("Removing files is not supported for any output except rdup");
				return NULL;
			}

			/* type */
			switch(buf[1]) {
				case '-': e->f_mode = S_IFREG; break;
				case 'd': e->f_mode = S_IFDIR; break;
				case 'l': e->f_mode = S_IFLNK; break;
				case 'h': e->f_mode = S_IFREG; e->f_lnk = 1; break;
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
			if (i < 0 || i > 07777) {
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

			if (S_ISCHR(e->f_mode) || S_ISBLK(e->f_mode)) {
				int major, minor;
				n = strchr(pos, ',');
				if (!n) {
					msg("No major,minor found for device at line: %zd", l);
					return NULL;
				}
				*n = '\0';
				major = atoi(pos); minor = atoi(n + 1);
				e->f_size = 0;
				e->f_rdev = makedev(major, minor);

				if (stat != NO_STAT_CONTENT) {
					/* there are entries left, correctly
					 * set the pointer 
					 */
					pos = strchr(n + 1, ' ');
					pos++;
				}
			} else {
				/* XXX check */
				if (stat == NO_STAT_CONTENT) {
					e->f_size = atoi(pos);
				} else {
					n = strchr(pos, ' ');
					if (!n) {
						msg("Malformed input for file size at line: %zd", l);
						return NULL;
					}
					/* atoi? */
					e->f_size = atoi(pos);
					pos = n + 1;
				}
			}
			/* all path should begin with / */
			switch(stat) {
				case DO_STAT:
					/* pathname */
					e->f_name = g_strdup(pos);
					if (strlen(e->f_name) != e->f_name_size) {
						msg("Real pathname length is not equal to pathname length at line: %zd", l);
						return NULL;
					}

					if (S_ISLNK(e->f_mode) || e->f_lnk == 1) {
						e->f_name[e->f_size] = '\0';
						j = lstat(e->f_name, s);
						e->f_name[e->f_size] = ' ';
					} else {
						j = lstat(e->f_name, s);
					}

					if (j == -1 && e->plusmin == PLUS) {
						msg(_("Could not stat path `%s\': %s"), e->f_name, strerror(errno));
						return NULL;
					}

					break;
				case NO_STAT:
					/* pathname */
					e->f_name = g_strdup(pos);
					if (strlen(e->f_name) != e->f_name_size) {
						msg("Real pathname length is not equal to pathname length at line: %zd", l);
						return NULL;
					}

					break;
				case NO_STAT_CONTENT:
					/* pathname will be present but after a newline
					 * so there isn't much to do here - this must
					 * be read from within the calling function */
					break;
			}
			break;
	}
	return e;
}

/* ALmost the same of entry_print_data in gfunc.c, but
 * not quite as we don't don't use FILE* structs here
 * for instance. TODO: integrate the two functions?
 * entry_print_data()
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
		if (e->f_lnk == 1)
			t = 'h';
		else
			t = '-';
	}
	
	if (t == 'b' || t == 'c') {
		out = g_strdup_printf("%c%c %.4o %ld %ld %ld %d,%d\n%s", 
			e->plusmin == PLUS ? '+':'-',
			t,
			(int)e->f_mode & ~S_IFMT,
			(unsigned long)e->f_uid,
			(unsigned long)e->f_gid,
			(unsigned long)e->f_name_size,
			(unsigned int)major(e->f_rdev), (unsigned int)minor(e->f_rdev),
			e->f_name);
	} else {
		out = g_strdup_printf("%c%c %.4o %ld %ld %ld %zd\n%s", 
			e->plusmin == PLUS ? '+':'-',
			t,
			(int)e->f_mode & ~S_IFMT,
			(unsigned long)e->f_uid,
			(unsigned long)e->f_gid,
			(unsigned long)e->f_name_size,
			(size_t)e->f_size,
			e->f_name);
	}
	/* XXX bail out? see below too */
	if (write(1, out, strlen(out)) == -1) 
		msg("Failed to write to stdout: %s", strerror(errno));
	return;
}

void
rdup_write_data(__attribute__((unused)) struct r_entry *e, char *buf, size_t len) {
	block_out_header(NULL, len, 1);
	block_out(NULL, len, buf, 1);
/*	if (write(1, buf, len) == -1) 
		msg("Failed to write to stdout: %s", strerror(errno));
XXX TODO errors		*/
	return;
}
