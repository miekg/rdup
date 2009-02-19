/*
 * Copyright (c) 2005 - 2008 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

extern char *PROGNAME;

void
usage(FILE *f)
{
        fprintf(f, _("USAGE: %s [OPTION]... FILELIST DIR|FILE...\n"), PROGNAME);
	fputs( _("\
Generate a full or incremental file list. This list can be used to\n\
implement a (incremental) backup scheme.\n\
\n\
	FILELIST\tfile to store filenames\n\
        DIR\t\tdirectory or directories to dump\n\
\n\
\n\
    OPTIONS:\n\
        -N FILE\t\tuse the timestamp of FILE for incremental dumps\n\
        \t\tif FILE does not exist, a full dump is performed\n\
        -F FORMAT\tuse specified format string\n\
        \t\tdefaults to: \"%p%T %b %u %g %l %s %n\\n\"\n\
	-R\t\treverse the output (depth first, first the dirs then the files)\n\
	-E FILE\t\tuse FILE as an exclude list\n\
        -0\t\tdelimit internal filelist with NULLs\n\
        -V\t\tprint version\n"), f);
	fputs( _("\
	-d FILE\t\tduplicate rdup output to FILE (in the default fmt)\n\
        -c\t\tcat the contents (FORMAT=\"%p%T %b %u %g %l %s\\n%n%C\")\n\
        -m\t\tonly print new/modified files (unsets -r)\n\
        -n\t\tignore .nobackup files\n\
        -r\t\tonly print removed files (unsets -m)\n\
        -s SIZE\t\tonly output files smaller then SIZE bytes\n\
        -x\t\tstay in local file system\n\
        -v\t\tbe more verbose\n\
        -h\t\tthis help\n\
\n\
    FORMAT:\n\
        The following escape sequences are recognized:\n\
        \'%p\': '+' if new, '-' if removed\n\
        \'%b\': permission bits\n\
        \'%m\': file mode bits\n\
        \'%u\': uid\n\
        \'%g\': gid\n\
        \'%l\': path length (for links: length of \'path -> target\')\n\
        \'%s\': file size\n\
        \'%n\': path (for links: \'path -> target\')\n\
        \'%N\': path (for links: \'path')\n\
        \'%t\': time of modification (epoch)\n\
        \'%H\': the sha1 hash of the file's contents\n\
        \'%T\': \'type\' (d, l, h, -, c, b, p or s: dir, symlink, hardlink, file, \n\
	      character device, block device, named pipe or socket)\n\
        \'%C\': file contents\n\
\n\
Report bugs to <miek@miek.nl>\n\
Licensed under the GPL version 3.\n\
See the file LICENSE in the source distribution of rdup.\n"), f);
}
