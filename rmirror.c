/* 
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 */

/* remotely mirror. This program is essentially the same thing
 * as mirror.sh, except that it is written in C to handle the
 * binary stuff better
 */

#define _GNU_SOURCE
#include "rdup.h"

#define AFTER_MODUS 6

int
main(int argc, char **argv)
{
	/* readline 
	 * read contents
	 * until eof */
	ssize_t s;
	size_t t;
	char   *buf;
	char   *bufp;
	char   *p;

	char   m;
	mode_t modus;
	uid_t  uid;
	gid_t  gid;
	size_t size;
	char   *name;

	size_t i;
	size_t c;
	
	buf = g_malloc(BUFSIZE + 1);
	bufp = buf;
	s = BUFSIZE;
	
	/* +33261 1000 1000 2885 /home/miekg/bin/tt */

	while ((s = getdelim(&buf, &t, '\n', stdin)) != -1) {
		if (t < LIST_MINSIZE) {
			/* corrupt */
		}
		buf[s - 1] = '\0';

		m = buf[0];  /* +/- */
		modus = (mode_t)atoi(buf + 1);
		if (modus == 0) {
			fprintf(stderr, "** Corrupt entry in filelist\n");
			continue;
		}
		p = strchr(buf + AFTER_MODUS + 1, ' '); *p = '\0';
		uid = (uid_t)atoi(buf + AFTER_MODUS);
		buf = p + 1;
		
		p = strchr(buf, ' '); *p = '\0';
		gid = (gid_t)atoi(buf);
		buf = p + 1;

		p = strchr(buf, ' '); *p = '\0';
		size = (size_t)atoi(buf);
		buf = p + 1;

		/* the rest is the name */
		name = strdup(buf);

		/* stdin is never closed here, so we keep reading the
		 * number of bytes (size) we should read
		 *
		 * buf is plundered and can be reused
		 */
		printf("%d,%d: %i %i\n", size, BUFSIZE, (size / BUFSIZE), (size % BUFSIZE));
		for (i = 0; i < size / BUFSIZE; i++) {
			c = fread(bufp, sizeof(char), BUFSIZE, stdin);
			if (c != BUFSIZE) {
				fprintf(stderr, "** Read too little from standard input\n");
				exit(EXIT_FAILURE);
			}
		}
		/* read in the remainder */
		c = fread(bufp, sizeof(char), size % BUFSIZE, stdin);
		if (c != size % BUFSIZE) {
			fprintf(stderr, "** Read too little from standard input\n");
			exit(EXIT_FAILURE);
		}
		
		printf("%c%d %d %d %d %s\n", m, modus, uid, gid, size, name);

		buf = bufp;
		g_free(name);
	}
	exit(EXIT_SUCCESS);
}
