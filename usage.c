/*
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

void
usage(FILE *f)
{
        fprintf(f, "USAGE: %s [OPTION...] FILELIST DIR|FILE...\n", PROGNAME);
        fprintf(f, "%s generates a full or incremental file list, this\n", PROGNAME);
        fprintf(f, "list can be used to implement a (incremental) backup scheme.\n");
        fprintf(f, "\n   FILELIST\tfile to store filenames\n");
        fprintf(f, "   DIR\t\tdirectory or directories to dump\n");
        fprintf(f, "\nOPTIONS:\n");
        fprintf(f, "   -N FILE\tuse the timestamp of FILE for incremental dumps\n");
        fprintf(f, "   \t\tif FILE does not exist, a full dump is performed\n");
        fprintf(f, "   -F FORMAT\tuse specified format string\n");
        fprintf(f, "   \t\tdefaults to: \"%%p%%T %%b %%u %%g %%l %%s %%n\\n\"\n");
        fprintf(f, "   -0\t\tdelimit internal filelist with NULLs\n");
        fprintf(f, "   -V\t\tprint version\n");
#ifdef HAVE_ATTR_XATTR_H
        fprintf(f, "   -a\t\tread the extended attributes: r_uid, r_gid\n");
#endif /* HAVE_ATTR_XATTR_H */
#ifdef HAVE_OPENAT
        fprintf(f, "   -a\t\tread the extended attributes: r_uid, r_gid\n");
#endif /* HAVE_OPENAT */
        fprintf(f,
                "   -c\t\tcat the contents (FORMAT=\"%%p%%T %%b %%u %%g %%l %%s\\n%%n%%C\")\n");
        fprintf(f, "   -h\t\tthis help\n");
        fprintf(f, "   -m\t\tonly print new/modified files (unsets -r)\n");
        fprintf(f, "   -l\t\tdon't check for file size changes\n");
        fprintf(f, "   -n\t\tignore " NOBACKUP " files\n");
        fprintf(f, "   -r\t\tonly print removed files (unsets -m)\n");
        fprintf(f, "   -s SIZE\tonly output files smaller then SIZE bytes\n");
        fprintf(f, "   -v\t\tbe more verbose\n");
        fprintf(f, "   -x\t\tstay in local file system\n");
        fprintf(f, "\nFORMAT:\n");
        fprintf(f, "   The following escape sequences are recognized:\n");
        fprintf(f, "   \'%%p\': '+' if new, '-' if removed\n");
        fprintf(f, "   \'%%b\': permission bits\n");
        fprintf(f, "   \'%%m\': file mode bits\n");
        fprintf(f, "   \'%%u\': uid\n");
        fprintf(f, "   \'%%g\': gid\n");
        fprintf(f, "   \'%%l\': path length\n");
        fprintf(f, "   \'%%s\': file size\n");
        fprintf(f, "   \'%%n\': path\n");
        fprintf(f, "   \'%%t\': time of modification (epoch)\n");
        fprintf(f, "   \'%%H\': the sha1 hash of the file's contents\n");
        fprintf(f, "   \'%%T\': \'type\' (d, l or -: dir, link or file)\n");
        fprintf(f, "   \'%%C\': file contents\n");
        fprintf(f, "\nReport bugs to <miek@miek.nl>\n");
        fprintf(f, "Licensed under the GPL. See the file LICENSE in the\n");
        fprintf(f, "source distribution of rdup.\n");
}
