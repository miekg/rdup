#include "rdup.h"

#define GETDELIM_BUFFER 128

extern int sig;

/* copied from xine-devel, same license applies
 * slightly modified to fit my needs
 */
ssize_t
rdup_getdelim(char **lineptr, size_t *n, int delimiter, FILE *stream)
{
        char *p;
        int c;
        size_t len = 0;

        if (!lineptr || !n || (!*lineptr && *n))
                return -1;

        /* **lineptr is allocated */
        p = *lineptr;

        /* read characters from stream */
        while ((c = fgetc(stream)) != EOF) {
		if (sig != 0) {
			fclose(stream);
			signal_abort(sig);
		}
                if (len >= *n) {
			msg(_("Line longer than %d characters"), *n);
			return 0;
#if 0
                        char *np = g_realloc(*lineptr, *n * 2);
                        if (!np)
                                return -1;
                        p = np + (p - *lineptr);
                        *lineptr = np;
                        *n *= 2;
#endif
                }
                *p++ = (char) c;
                len++;
                if (delimiter == c)
                        break;
        }

        /* end of file without any bytes read */
        if ((c == EOF) && (len == 0))
                return -1;

        /* trailing "\0" */
        if (len >= *n) {
                char *np = g_realloc(*lineptr, *n + 1);
                if (!np)
                        return -1;
                p = np + (p - *lineptr);
                *lineptr = np;
                *n += 1;
        }
        *p = '\0';
        return len;
}
