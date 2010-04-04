/*
 * Copyright (c) 2009,2010 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup-tr.h"

extern char *PROGNAME;

void
usage_tr(FILE *f)
{
        fprintf(f, _("USAGE: %s [OPTION]... \n"), PROGNAME);
	fputs( _("\
Translate rdup output into something else.\n\
\n\
\n\
    OPTIONS:\n\
        -c\t\tforce output to tty\n\
	-X FILE\t\tencrypt all paths with AES and the key from FILE\n\
	-Y FILE\t\tdecrypt all paths with AES and the key from FILE\n\
	-h\t\tthis help\n\
	-V\t\tprint version\n\
        -O FMT\t\toutput format: pax, cpio, tar or rdup* (* = default)\n\
	\t\trdup uses format: \"%p%T %b %u %g %l %s\\n%n%C\"\n\
	-L\t\tset input format to a list of pathnames\n\
	-v\t\tbe more verbose and print processed files to stderr\n\
\n\
Report bugs to <miek@miek.nl>\n\
Licensed under the GPL version 3.\n\
See the file LICENSE in the source distribution of rdup.\n"), f);
}
