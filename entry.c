/* 
 * Copyright (c) 2009 Miek Gieben
 * parse_entry.c
 * parse an standard rdup entry and return a
 * struct rdup
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
struct rdup *
parse_entry(char *buf, size_t l, struct stat *s) 
{
	struct rdup *e;
	gint i;
	char *n, *pos;
	e = g_malloc(sizeof(struct rdup));
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
			e->f_target    = NULL;
			e->f_mode      = s->st_mode;
			e->f_uid       = s->st_uid;
			e->f_gid       = s->st_gid;
			e->f_size      = s->st_size;
			e->f_dev       = s->st_dev;
			e->f_ino       = s->st_ino;
			e->f_rdev      = s->st_rdev;
			e->f_lnk       = 0;
			e->f_ctime     = s->st_ctime;
			e->f_mtime     = s->st_mtime;

			/* you will loose hardlink information here 
			 * as 'stat' cannot check this */
			if (S_ISLNK(e->f_mode)) 	
				e->f_target = slink(e);

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
				msg(_("First character should \'-\' or \'+\', `%s\' at line: %zd"), buf, l);
				return NULL;
			}
			if (buf[0] == '+')
				e->plusmin = PLUS;
			if (buf[0] == '-')
				e->plusmin = MINUS;

			if (opt_output != O_RDUP && e->plusmin == MINUS) {
				msg(_("Removing files is not supported for any output except rdup"));
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
					msg(_("Type must be one of d, l, h, -, c, b, p or s"));
					return NULL;
			}
			
			/* perm */
			i = (buf[3] - 48) * 512 + (buf[4] - 48) * 64 +	/* oct -> dec */
				(buf[5] - 48) * 8 + (buf[6] - 48);
			if (i < 0 || i > 07777) {
				msg(_("Invalid permissions at line: %zd"), l);
				return NULL;
			}
			e->f_mode |= i;

			/* m_time */
			n = strchr(buf + 8, ' ');
			if (!n) {
				msg(_("Malformed input for m_time at line: %zd"), l);
				return NULL;
			}
			e->f_mtime = (time_t)atol(buf + 8);
			pos = n + 1;
			
			/* uid  */
			n = strchr(pos, ' ');
			if (!n) {
				msg(_("Malformed input for uid at line: %zd"), l);
				return NULL;
			} else {
				*n = '\0';
			}
			e->f_uid = atoi(pos);
			pos = n + 1;

			/* username */
			n = strchr(pos, ' ');
			if (!n) {
				msg(_("Malformed input for user at line: %zd"), l);
				return NULL;
			} else {
				*n = '\0';
			}
			e->f_user = g_strdup(pos);
			pos = n + 1;

			/* gid */
			n = strchr(pos, ' ');
			if (!n) {
				msg(_("Malformed input for gid at line: %zd"), l);
				return NULL;
			} else {
				*n = '\0';
			}
			e->f_gid = atoi(pos);
			pos = n + 1;

			/* groupname */
			n = strchr(pos, ' ');
			if (!n) {
				msg(_("Malformed input for group at line: %zd"), l);
				return NULL;
			} else {
				*n = '\0';
			}
			e->f_group = g_strdup(pos);
			pos = n + 1;

			/* pathname length */
			n = strchr(pos, ' ');
			if (!n) {
				msg(_("Malformed input for path length at line: %zd"), l);
				return NULL;
			}
			e->f_name_size = atoi(pos); /* checks */
			pos = n + 1;

			/* dev file? */
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
			} else 
				e->f_size = atoi(pos);
			
			break;
	}
	return e;
}

/* NEED TO FIX THIS, the the gfunc equavalent */

/* ALmost the same of entry_print_data in gfunc.c, but
 * not quite as we don't don't use FILE* structs here
 * for instance. TODO: integrate the two functions?
 * entry_print_data()
 */
gint
rdup_write_header(struct rdup *e)
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
		/* device */
		out = g_strdup_printf("%c%c %.4o %ld %ld %s %ld %s %ld %d,%d\n%s", 
			e->plusmin == PLUS ? '+':'-',
			t,
			(int)e->f_mode & ~S_IFMT,
			(unsigned long)e->f_mtime,
			(unsigned long)e->f_uid,
			e->f_user,
			(unsigned long)e->f_gid,
			e->f_group,
			(unsigned long)e->f_name_size,
			(unsigned int)major(e->f_rdev), (unsigned int)minor(e->f_rdev),
			e->f_name);
	} else if (t == 'l' || t == 'h') {
		/* link */
		gchar *n;
		n = g_strdup_printf("%s -> %s", e->f_name, e->f_target);
		e->f_name_size = strlen(n);
		out = g_strdup_printf("%c%c %.4o %ld %ld %s %ld %s %ld %zd\n%s", 
			e->plusmin == PLUS ? '+':'-',
			t,
			(int)e->f_mode & ~S_IFMT,
			(unsigned long)e->f_mtime,
			(unsigned long)e->f_uid,
			e->f_user,
			(unsigned long)e->f_gid,
			e->f_group,
			(unsigned long)e->f_name_size,
			(size_t)e->f_size, n);
	} else {
		out = g_strdup_printf("%c%c %.4o %ld %ld %s %ld %s %ld %zd\n%s", 
			e->plusmin == PLUS ? '+':'-',
			t,
			(int)e->f_mode & ~S_IFMT,
			(unsigned long)e->f_mtime,
			(unsigned long)e->f_uid,
			e->f_user,
			(unsigned long)e->f_gid,
			e->f_group,
			(unsigned long)e->f_name_size,
			(size_t)e->f_size,
			e->f_name);
	}
	if (write(1, out, strlen(out)) == -1) {
		msg(_("Failed to write to stdout: %s"), strerror(errno));
		return -1;
	}
	return 0;
}

gint
rdup_write_data(__attribute__((unused)) struct rdup *e, char *buf, size_t len) {
	if (block_out_header(NULL, len, 1) == -1 ||
		block_out(NULL, len, buf, 1) == -1)
		return -1;
	return 0;
}
