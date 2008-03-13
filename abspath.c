#include "rdup.h"

/*
 * Remove /./ and /../ and // from a pathname
 * ... will trigger an error
 * An absolute pathname argument is required.
 *
 * Returns NULL on error, otherwise NULL terminated
 * sanitized pathname.
 *
 * Also see man realpath(3)
 */

char * abspath(char *path) {
    char *p;
    int lastslash, lastlastslash;
    char *abspath;
    int i, dotseen;
    
    if (path[0] != '/')
	return NULL;

    printf("path %s\n", path);

    /* place guard at the end */
    abspath = g_malloc(BUFSIZ + 1);

    lastslash = lastlastslash = 0;
    for(i = 0, p = path; *p; p++, i++) {
	switch(*p) {
	    case '/':
		lastlastslash = lastslash;
		lastslash = i;
	    break;
	    
	    case '.':
		dotseen++;
		break;
	



	}
	printf("%d %d\n", lastlastslash, lastslash);
    }


    return abspath;
}
