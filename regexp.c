/*
 * Copyright (c) 2005 - 2008 Miek Gieben
 * See LICENSE for the license
 *
 * regexp helper functions
 */

#include "rdup.h"
#include <pcre.h>

extern gboolean opt_null;
GSList *pregex_list = NULL;

/**
 * Read the filename and create the compiled regexp
 * in two linked lists; one for files and one for 
 * directories
 */
gboolean
regexp_init(char *file) {
	FILE 	      	*fp;
	char 		*buf;
	const char	*errbuf;
	int		erroff;
	char          	delim;
	gpointer 	d;
	size_t 	 	l;
	size_t		s;
	ssize_t	 	j;
	pcre		*P;

	if ((fp = fopen(file, "r")) == NULL) {
		msg(_("Could not open '%s\': %s"), file, strerror(errno));
		exit(EXIT_FAILURE);
	}	
	/* read the regexp */
	buf  = g_malloc(BUFSIZE + 1);
	s    = BUFSIZE;

	if (opt_null)
                delim = '\0';
        else 
                delim = '\n';

	l = 1;
	while ((j = rdup_getdelim(&buf, &s, delim, fp)) != -1) {
		if (buf[0] == '#' || buf[0] == '\n') 
			continue;

		/* buf[j - 1] holds the delimeter */
		buf[j - 1] = '\0';

		if ((P = pcre_compile(buf, 0, &errbuf, &erroff, NULL)) == NULL) {
			/* error */
			fclose(fp);
			msg(_("Corrupt regular expression line: %zd, column %d: %s"), l, erroff, errbuf); 
			g_free(buf);
			return FALSE;
		} else {
			d = g_malloc(sizeof P);
			d = memcpy(d, &P, sizeof P);
			pregex_list = g_slist_append(pregex_list, d);
		}
		l++;
	}
	fclose(fp);
	g_free(buf);
	return TRUE;
}
