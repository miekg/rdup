#include "rdup-up.h"

/* strip n components from pathname
 * and return the name
 * returns NULL when the path didn't contain enough
 * components to begin with
 */

char *
strippath(char *path, guint strip)
{
	char *p;
	guint i;

	for(i = 1, p = strchr(path, DIR_SEP); p; p = strchr(p + 1, DIR_SEP), i++) {
		if (i > strip)
			break;
	}
	return p;
}
