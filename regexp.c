/*
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 *
 * regexp helper functions
 */

#include "rdup.h"
#include <regex.h>

extern gboolean opt_null;
GSList *regex_list = NULL;

/**
 * Read the filename and create the compiled regexp
 * in two linked lists; one for files and one for 
 * directories
 */
gboolean
regexp_init(char *file) {
	FILE 	      	*fp;
	char 		*buf;
	char 		*errbuf;
	char          	delim;
	gpointer 	d;
	size_t 	 	l;
	size_t		s;
	ssize_t	 	j;
	regex_t		R;
	int 		i;

	if ((fp = fopen(file, "r")) == NULL) {
		msg("Could not open '%s\': %s", file, strerror(errno));
		exit(EXIT_FAILURE);
	}	
	/* read the regexp */

	buf  = g_malloc(BUFSIZE + 1);
	errbuf = g_malloc(BUFSIZE + 1);
	s    = BUFSIZE;

	if (opt_null) {
                delim = '\0';
        } else {
                delim = '\n';
        }

	l = 1;
	while ((j = getdelim(&buf, &s, delim, fp)) != -1) {
		if (buf[0] == '#' || buf[0] == '\n') 
			continue;

		/* buf[j - 1] holds the delimeter */
		buf[j - 1] = '\0';
		if ((i = regcomp(&R, buf, REG_EXTENDED | REG_NOSUB)) != 0) {
			fclose(fp);
			(void)regerror(i, &R, errbuf, BUFSIZE);
			msg("Corrupt regular expression line: %zd: %s", l, errbuf); 
			return FALSE;
		} else {
			d = g_malloc(sizeof R);
			d = memcpy(d, &R, sizeof R);
			regex_list = g_slist_append(regex_list, d);
		}
		l++;
	}
	fclose(fp);
	return TRUE;
}
