#include "rdup-up.h"

/* strip n components from pathname
 * and return the name
 * returns NULL when the path didn't contain enough
 * components to begin with
 */

char *
strip(char *path, guint strip)
{
	char *p;
	guint i;

	for(i = 1, p = strchr(path, '/'); p; p = strchr(p + 1, '/'), i++) {
		if (i > strip)
			break;
	}
	return p;
}
