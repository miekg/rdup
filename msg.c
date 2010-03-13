#include "rdup.h"

extern char *PROGNAME;

void
msg_va_list(const char *fmt, va_list args)
{
#ifdef DEBUG
	/* put calling function in here */
        fprintf(stderr, "** %s: ", PROGNAME);
#else
        fprintf(stderr, "** %s: ", PROGNAME);
#endif
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
}

void
msg(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        msg_va_list(fmt, args);
        va_end(args);
}
