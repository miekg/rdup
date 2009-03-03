/* common functions for entries */

#include <glib.h>
#include <sys/stat.h>
#include "entry.h"

struct r_entry *
entry_dup(struct r_entry *f)
{
        struct r_entry *g;
        g = g_malloc(sizeof(struct r_entry));

	g->plusmin	= f->plusmin;
        g->f_name       = g_strdup(f->f_name);
        g->f_name_size  = f->f_name_size;
        g->f_lnk	= f->f_lnk;
        g->f_uid        = f->f_uid;
        g->f_gid        = f->f_gid;
        g->f_mode       = f->f_mode;
	g->f_ctime      = f->f_ctime;
	g->f_size       = f->f_size;
	g->f_dev        = f->f_dev;
	g->f_rdev       = f->f_rdev;
	g->f_ino        = f->f_ino;
        return g;
}

void
entry_free(struct r_entry *f)
{
	g_free(f->f_name);
	g_free(f);
}
