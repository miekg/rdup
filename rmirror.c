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
#define SUFFIX_SIZE 12


gboolean 
rmkdir(char *name, uid_t u, gid_t g)
{
	printf("%d %d %s/\n", u, g, name);
	return TRUE;
}


gboolean
rmklnk(char *name, char *target, uid_t u, gid_t g)
{
	printf("%d %d %s->%s\n", u, g, name, target);
	return TRUE;
}

gboolean
exist(char *path)
{
	struct stat s;

	return lstat(path, &s);
}


int
main(int argc, char **argv)
{
	/* readline 
	 * read contents
	 * until eof */
	ssize_t 	s;
	size_t 		t;
	char   		*buf;
	char   		*bufp;
	char   		*p;
	char 		*date_suf;
	time_t  	epoch;

	char   		m;
	char   		*name;
	mode_t 		modus;
	uid_t  		uid;
	gid_t  		gid;
	size_t 		size;
	size_t 		i;
	size_t 		c;
	
	buf      = g_malloc(BUFSIZE + 1);
	date_suf = g_malloc(SUFFIX_SIZE);
	bufp     = buf;
	s        = BUFSIZE;
	
	(void)time(&epoch);
	strftime(date_suf, SUFFIX_SIZE, ".%m%d.%H:%M", localtime(&epoch));
	
	/* Read line like this: 
	 * +33261 1000 1000 2885 /home/miekg/bin/tt 
	 */
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

		/*
		 * buf is plundered and can be reused
		 */
		if (S_ISDIR(modus)) {
			rmkdir(name, uid, gid);
		}

		if (S_ISLNK(modus)) {
			/* we will be able to read this in one swoop */
			c = fread(bufp, sizeof(char), size, stdin);
			if (c != size) {
				fprintf(stderr, "** Read too little from standard input\n");
				exit(EXIT_FAILURE);
			}
			rmklnk(name, bufp, uid, gid);
		}


		if (S_ISREG(modus)) {
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
			printf("%d %d %s\n", uid, gid, name);
		}
		buf = bufp; /* reset the pointer */
		g_free(name);
	}
	exit(EXIT_SUCCESS);
}
