/*
 * Copyright (c) 2005 - 2011 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

extern char *PROGNAME;

void usage(FILE * f)
{
	fprintf(f, _("USAGE: %s [OPTION]... FILELIST  [ DIR | FILE ]...\n"),
		PROGNAME);
	fputs(_("\
Generate a full or incremental file list. This list can be used to\n\
implement a (incremental) backup scheme.\n\
\n\
	FILELIST\tfile to store filenames\n\
        DIR\t\tdirectory or directories to dump, defaults to .\n\
\n\
\n\
    OPTIONS:\n\
        -N FILE\t\tuse the (c_time) timestamp of FILE for incremental dumps\n\
        \t\tif FILE does not exist, a full dump is performed\n\
	-M FILE\t\tas -N, but use the m_time\n\
        -F FORMAT\tuse specified format string\n\
        \t\tdefaults to: \"%p%T %b %u %g %l %s %n\\n\"\n\
	-R\t\treverse the output (depth first, first the dirs then the files)\n\
	-E FILE\t\tuse FILE as an exclude list\n\
	-P CMD\n\
	\t\tfilter file contents through CMD, will be called with 'sh -c CMD'\n\
	\t\tmay be repeated, output will be filtered through all commands\n\
        -V\t\tprint version\n"), f);
	fputs(_("\
        -a\t\treset atime\n\
        -c\t\tforce output to tty\n\
        -m\t\tonly print new/modified files (unsets -r)\n\
        -n\t\tignore .nobackup files\n\
        -r\t\tonly print removed files (unsets -m)\n\
        -s SIZE\t\tonly output files smaller then SIZE bytes\n\
        -u\t\tdisable the special handling of ._rdup_. files\n\
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
        \'%s\': original file size\n\
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
