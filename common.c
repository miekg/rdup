/* common functions for entries */

#include <glib.h>
#include <sys/stat.h>
#include "entry.h"

struct rdup *
entry_dup(struct rdup *f)
{
        struct rdup *g;
        g = g_malloc(sizeof(struct rdup));
	g->plusmin	= f->plusmin;
        g->f_name       = g_strdup(f->f_name);
        g->f_target     = g_strdup(f->f_target);
        g->f_name_size  = f->f_name_size;
        g->f_lnk	= f->f_lnk;
        g->f_uid        = f->f_uid;
        g->f_user       = f->f_user;
        g->f_gid        = f->f_gid;
        g->f_group      = f->f_group;
        g->f_mode       = f->f_mode;
	g->f_ctime      = f->f_ctime;
	g->f_mtime      = f->f_mtime;
	g->f_atime      = f->f_atime;
	g->f_size       = f->f_size;
	g->f_dev        = f->f_dev;
	g->f_rdev       = f->f_rdev;
	g->f_ino        = f->f_ino;
        return g;
}

void
entry_free(struct rdup *f)
{
	g_free(f->f_name);
	g_free(f->f_target);
	g_free(f);
}
