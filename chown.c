/*
 * Copyright (c) 2012 Miek Gieben
 * Parse username:group helper files
 */

#include "rdup.h"

/* signal.c */
void got_sig(int signal);

/* write a chown helper file */
void
chown_write(gchar *dir, gchar *base, uid_t u, gchar *user, gid_t g, gchar *group)
{
        FILE *f;
        gchar *path = g_strdup_printf("%s/%s%s", dir, USRGRPINFO, base);
        if ( ! ( f = fopen(path, "r"))) {
                /* no file found or failure to open */
                g_free(path);
                return;
        }
        g_free(path);
        fprintf(f, "%s:%d/%s:%d", user, u, group, g);
        fclose(f);
}

/* parse the chown help file */
chown_pack
chown_parse(gchar *dir, gchar *base)
{
        dir = dir; base = base;
        return NULL;
}
