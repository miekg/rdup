/*
 * Copyright (c) 2009 - 2011 Miek Gieben
 * See LICENSE for the license
 */
#include "rdup.h"

extern char *PROGNAME;

void msg_va_list(const char *fmt, va_list args)
{
	fprintf(stderr, "** %s: ", PROGNAME);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

/* only with debugging enabled will this call be different */
void msgd_va_list( __attribute__ ((unused))
		  const char *func, __attribute__ ((unused))
		  int line, const char *fmt, va_list args)
{
#ifdef DEBUG
	fprintf(stderr, "** %s: %s():%d  ", PROGNAME, func, line);
#else
	fprintf(stderr, "** %s: ", PROGNAME);
#endif				/* DEBUG */
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

void msg(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_va_list(fmt, args);
	va_end(args);
}

void msgd(const char *func, int line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msgd_va_list(func, line, fmt, args);
	va_end(args);
}
