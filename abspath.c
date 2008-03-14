#include "rdup.h"

/*
 * Remove /./ and /../ and // from a pathname
 * An absolute pathname argument is required. 
 * Returns NULL on error, otherwise NULL terminated
 * sanitized pathname.
 *
 * Also see realpath(3)
 */
char * abspath(char *path) {
	char *p, *c;
	char *slash, *abspath2;
	char *abspath;
	int i;

	if (!path)
		return NULL;

	if (!g_path_is_absolute(path))
		return NULL;

	/* abspath can be NULL or abspath[0] == '\0'. The NULL 
	 * is initial. the [0] == '\0' is when we got back to 
	 * the root
	 */
	abspath = NULL;
	abspath2 = g_strdup(path);
	i = strlen(abspath2);
	if (i > BUFSIZ)
		return NULL;

	/* add closing / (guard) */
	if (abspath2[i - 1] != DIR_SEP) {
		abspath2 = g_realloc(abspath2, i + 2);
		abspath2[i] = DIR_SEP;
		abspath2[i + 1] = '\0';
	}

	/* jump from slash to slash */
	for (p = abspath2; (c = strchr(p, DIR_SEP)); p++) {
		*c = '\0';
		if (*p == '\0' || (strcmp(p, ".") == 0)) {
			/* do nothing */
			p = c;
		} else if (strcmp(p, "..") == 0) {
			/* go back a slash */
			if (abspath == NULL || abspath[0] == '\0') {
				abspath = g_strdup(DIR_SEP_STR);
			} else {
				slash = strrchr(abspath, DIR_SEP);
				*slash = '\0';
				*c = DIR_SEP;
			}
		} else {
			if (abspath == NULL || abspath[0] == '\0' ||
				(strcmp(abspath, DIR_SEP_STR) == 0) )  {
				abspath = g_strconcat(DIR_SEP_STR, p, NULL);
			} else {
				abspath = g_strjoin(DIR_SEP_STR, abspath, p, NULL);
			}
			*c = DIR_SEP;
		}
		p = c;
	}
	if (abspath == NULL || abspath[0] == '\0')
		abspath = g_strdup(DIR_SEP_STR);

	return abspath;
}
