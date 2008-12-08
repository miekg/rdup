/* 
 * Copyright (c) 2009 Miek Gieben
 * parse_entry.c
 * parse an standard rdup entry and return a
 * struct r_entry
 */

#include "rdup-tr.h"

extern gint opt_input;

/*
 * parse a standard rdup output entry
 * +- 0775 1000 1000 18 2947 /home/miekg/bin/tt
 * buf is NULL delimited 
 */
struct r_entry *
parse_entry(char *buf, size_t l, struct stat *s) 
{
	struct r_entry *e;
	e = g_malloc(sizeof(struct r_entry));

	switch (opt_input) {
		case I_LIST:
			if (stat(buf, s) == -1) {
				msg(_("Could not stat path `%s\': %s"), buf, strerror(errno));
				return NULL;
			}
			e->f_name = g_strdup(buf);
			/* other values */
			return e;
		break;

		case I_RDUP:
			if (strlen(buf) < LIST_MINSIZE){
				msg(_("Corrupt entry in filelist at line: %zd"), l);
				return NULL;
			}

			/* when complete parsed fill in the structure */
			e->f_name      = g_strdup(buf);

			/*
			e->f_name_size = f_name_size;
			e->f_mode      = modus;
			e->f_uid       = 0;
			e->f_gid       = 0;
			e->f_size      = 0;
			e->f_ctime     = 0;
			e->f_dev       = f_dev;
			e->f_ino       = f_ino;
			*/

			return e;

		break;

	}
	return NULL;	/* XXX */

}

#if 0
		if (!opt_null) {
			n = strrchr(buf, '\n');
			if (n)
				*n = '\0';
		}

		/* get modus */
		if (buf[LIST_SPACEPOS] != ' ') {
			msg(_("Corrupt entry in filelist at line: %zd, no space found"), l);
			return NULL;
		}

		buf[LIST_SPACEPOS] = '\0';
		modus = (mode_t)atoi(buf);
		if (modus == 0) {
			msg(_("Corrupt entry in filelist at line: %zd, `%s\' should be numerical"), l, buf);
			return NULL;
		}

		/* the dev */
		q = buf + LIST_SPACEPOS + 1;
		p = strchr(buf + LIST_SPACEPOS + 1, ' ');
		if (!p) {
			msg(_("Corrupt entry in filelist at line: %zd, no space found"), l);
			l++; 
			continue;
		}
		*p = '\0';
		f_dev = (dev_t)atoi(q);
		if (f_dev == 0) {
			msg(_("Corrupt entry in filelist at line: %zd, zero device"), l);
			return NULL;
		}

		/* the inode */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p) {
			msg(_("Corrupt entry in filelist at line: %zd, no space found"), l);
			return NULL;
		}
		*p = '\0';
		f_ino = (ino_t)atoi(q);
		if (f_ino == 0) {
			msg(_("Corrupt entry in filelist at line: %zd, zero inode"), l);
			return NULL;
		}
		/* the path size */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p) {
			msg(_("Corrupt entry in filelist at line: %zd, no space found"), l);
			return NULL;
		}
		/* the file's name */
		*p = '\0';
		f_name_size = (size_t)atoi(q);
		str_len = strlen(p + 1);
		if (str_len != f_name_size) {
			msg(_("Corrupt entry in filelist at line: %zd, length `%zd\' does not match `%zd\'"), l,
					str_len, f_name_size);
			return NULL;
		}

		e = g_malloc(sizeof(struct r_entry));
		e->f_name      = g_strdup(p + 1);
		e->f_name_size = f_name_size;
		e->f_mode      = modus;
		e->f_uid       = 0;
		e->f_gid       = 0;
		e->f_size      = 0;
		e->f_ctime     = 0;
		e->f_dev       = f_dev;
		e->f_ino       = f_ino;
	}
}
#endif

void
rdup_write_header(struct r_entry *r)
{
	r=r;
	return;
}

void
rdup_write_data(struct r_entry *r, char *buf, size_t len) {
	r=r;
	buf = buf;
	len = len;

	return;
}
