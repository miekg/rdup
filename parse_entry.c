/* 
 * Copyright (c) 2009 Miek Gieben
 * parse_entry.c
 * parse an standard rdup entry and return a
 * struct r_entry
 */

#include "rdup-tr.h"

/* buf is NULL delimited */
struct r_entry *
parse_entry(char *buf) 
{
	buf = buf;

	return NULL;
}

#if 0

		if (s < LIST_MINSIZE) {
			msg(_("Corrupt entry in filelist at line: %zd"), l);
			l++;
			continue;
		}
		if (!opt_null) {
			n = strrchr(buf, '\n');
			if (n)
				*n = '\0';
		}

		/* get modus */
		if (buf[LIST_SPACEPOS] != ' ') {
			msg(_("Corrupt entry in filelist at line: %zd, no space found"), l, buf);
			l++; 
			continue;
		}

		buf[LIST_SPACEPOS] = '\0';
		modus = (mode_t)atoi(buf);
		if (modus == 0) {
			msg(_("Corrupt entry in filelist at line: %zd, `%s\' should be numerical"), l, buf);
			l++; 
			continue;
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
			l++; 
			continue;
		}

		/* the inode */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p) {
			msg(_("Corrupt entry in filelist at line: %zd, no space found"), l);
			l++; 
			continue;
		}
		*p = '\0';
		f_ino = (ino_t)atoi(q);
		if (f_ino == 0) {
			msg(_("Corrupt entry in filelist at line: %zd, zero inode"), l);
			l++; 
			continue;
		}
		/* the path size */
		q = p + 1;
		p = strchr(p + 1, ' ');
		if (!p) {
			msg(_("Corrupt entry in filelist at line: %zd, no space found"), l);
			l++; 
			continue;
		}
		/* the file's name */
		*p = '\0';
		f_name_size = (size_t)atoi(q);
		str_len = strlen(p + 1);
		if (str_len != f_name_size) {
			msg(_("Corrupt entry in filelist at line: %zd, length `%zd\' does not match `%zd\'"), l,
					str_len, f_name_size);
			l++; 
			continue;
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
		g_tree_insert(tree, (gpointer)e, VALUE);
		l++;
	}
	g_free(buf);
	return tree;
}
#endif
