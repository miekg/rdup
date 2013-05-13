/*
 * Copyright (c) 2009 - 2011 Miek Gieben
 * License: GPLv3(+), see LICENSE for details
 */

#include "rdup-up.h"

extern char *PROGNAME;

void usage_up(FILE * f)
{
	fprintf(f, _("USAGE: %s [OPTION]... DIRECTORY\n"), PROGNAME);
	fputs(_("\
Update a directory tree with an rdup archive.\n\
\n\
        DIRECTORY\twhere to unpack the archive\n\
\n\
\n\
    OPTIONS:\n\
	-t\t\tcreate DIRECTORY if it does not exist\n\
	-s NUM\t\tstrip NUM leading path components\n\
	-r PATH\t\tstrip PATH from each pathname\n\
	-n\t\tdry run, do not touch the filesystem\n\
	-V\t\tprint version\n\
	-T\t\tshow table of contents. DIRECTORY is optional\n\
        -u\t\tdo not create ._rdup_. with user/group information\n\
        -q\t\tsilence \'chown\' failures even when \'root\'\n\
	-h\t\tthis help\n\
	-v\t\tbe more verbose and print processed files to stdout\n\
\n\
Report bugs to <miek@miek.nl>\n\
Licensed under the GPL version 3.\n\
See the file LICENSE in the source distribution of rdup.\n"), f);
}
