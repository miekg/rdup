/* 
 * Copyright (c) 2009 Miek Gieben
 * See LICENSE for the license
 * rdup-tr -- rdup translate, transform an
 * rdup filelist to an tar/cpio archive with 
 * per file compression and/or encryption
 */

#include "rdup-tr.h"

int
r_archive_open(__attribute__((unused)) struct archive *a, 
	__attribute__((unused))	void *data)
{
	/* I want to use stdout, so return ok always */
	return(ARCHIVE_OK);
}

int
r_archive_close(__attribute__((unused)) struct archive *a, 
	__attribute__((unused))	void *data)
{
	/* I want to use stdout, so return ok always */
	return(ARCHIVE_OK);
}

ssize_t
r_archive_write(__attribute__((unused)) struct archive *a, 
		__attribute__((unused)) void *data, 
		void *buff, size_t n) 
{
	return (write(1, buff, n));
}
