/* Function with behaviour like `mkdir -p'  */
/* From: http://niallohiggins.com/2009/01/08/mkpath-mkdir-p-alike-in-c-for-unix/ 
 * with some tweaks
 * libglib'i'fied by Miek Gieben
 */

#include "rdup-up.h"

int
mkpath(const char *s, mode_t mode)
{
        char *q, *r = NULL, *path = NULL, *up = NULL;
        int rv = -1;

        if (strcmp(s, ".") == 0 || strcmp(s, "/") == 0)
                return 0;
 
        if ((path = g_strdup(s)) == NULL)
                return -1;
 
        if ((q = g_strdup(s)) == NULL)
                return -1;
 
        if ((r = dirname(q)) == NULL)
                goto out;
 
        if ((up = g_strdup(r)) == NULL)
                return -1;
 
        if ((mkpath(up, mode) == -1) && (errno != EEXIST)) 
                goto out;
 
        if ((mkdir(path, mode) == -1) && (errno != EEXIST))
                rv = -1;
        else
                rv = 0;
 
out:
        if (up)
                free(up);
        free(q);
        free(path);
        return (rv);
}
