/*
 * Copyright (c) 2005 - 2008 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

void
usage(FILE *f)
{
        fprintf(f, _("USAGE: %s [OPTION...] FILELIST DIR|FILE...\n"), PROGNAME);
	fputs( _("\
Generate a full or incremental file list. This list can be\n\
implement a (incremental) backup scheme.\n\
\n\
	FILELIST\tfile to store filenames\n\
        DIR\t\tdirectory or directories to dump\n\
\n\
\n\
        OPTIONS:\n\
        -N FILE\tuse the timestamp of FILE for incremental dumps\n\
        \t\tif FILE does not exist, a full dump is performed\n\
        -F FORMAT\tuse specified format string\n\
        \t\tdefaults to: \"%%p%%T %%b %%u %%g %%l %%s %%n\\n\"\n\
	-E FILE\tuse FILE as an exclude list\n\
        -0\t\tdelimit internal filelist with NULLs\n\
        -V\t\tprint version\n"), f);
#ifdef HAVE_ATTR_XATTR_H
	fputs( _("-a\t\tread the extended attributes: r_uid, r_gid\n"), f);
#endif /* HAVE_ATTR_XATTR_H */
#ifdef HAVE_ATTROPEN
        fputs( _("-a\t\tread the extended attributes: r_uid, r_gid\n"), f);
#endif /* HAVE_ATTROPEN */
	fputs( _("\
        -c\t\tcat the contents (FORMAT=\"%%p%%T %%b %%u %%g %%l %%s %%n%%C\")\n\
        -h\t\tthis help\n\
        -m\t\tonly print new/modified files (unsets -r)\n\
        -l\t\tdon't check for file size changes\n\
        -n\t\tignore .nobackup files\n\
        -r\t\tonly print removed files (unsets -m)\n\
        -s SIZE\tonly output files smaller then SIZE bytes\n\
        -v\t\tbe more verbose\n\
        -x\t\tstay in local file system\n\
\n\
        FORMAT:\n\
        The following escape sequences are recognized:\n\
        \'%%p\': '+' if new, '-' if removed\n\
        \'%%b\': permission bits\n\
        \'%%m\': file mode bits\n\
        \'%%u\': uid\n\
        \'%%g\': gid\n\
        \'%%l\': path length\n\
        \'%%s\': file size\n\
        \'%%n\': path\n\
        \'%%t\': time of modification (epoch)\n\
        \'%%H\': the sha1 hash of the file's contents\n\
        \'%%T\': \'type\' (d, l or -: dir, link or file)\n\
        \'%%C\': file contents\n\
\n\
Report bugs to <miek@miek.nl>\n\
Licensed under the GPL version 3.\n\
See the file LICENSE in the source distribution of rdup.\n"), f);
}
