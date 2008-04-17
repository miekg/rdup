/*
 * Copyright (c) 2005 - 2008 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

void
usage(FILE *f)
{
        fprintf(f, _("USAGE: %s [OPTION...] FILELIST DIR|FILE...\n"), PROGNAME);
        fprintf(f, _("%s generates a full or incremental file list, this\n"), PROGNAME);
        fprintf(f, _("list can be used to implement a (incremental) backup scheme.\n"));
        fprintf(f, _("\n   FILELIST\tfile to store filenames\n"));
        fprintf(f, _("   DIR\t\tdirectory or directories to dump\n"));
        fprintf(f, _("\nOPTIONS:\n"));
        fprintf(f, _("   -N FILE\tuse the timestamp of FILE for incremental dumps\n"));
        fprintf(f, _("   \t\tif FILE does not exist, a full dump is performed\n"));
        fprintf(f, _("   -F FORMAT\tuse specified format string\n"));
        fprintf(f, _("   \t\tdefaults to: \"%%p%%T %%b %%u %%g %%l %%s %%n\\n\"\n"));
	fprintf(f, _("   -E FILE\tuse FILE as an exclude list\n"));
        fprintf(f, _("   -0\t\tdelimit internal filelist with NULLs\n"));
        fprintf(f, _("   -V\t\tprint version (%s)\n"), VERSION);
#ifdef HAVE_ATTR_XATTR_H
        fprintf(f, _("   -a\t\tread the extended attributes: r_uid, r_gid\n"));
#endif /* HAVE_ATTR_XATTR_H */
#ifdef HAVE_ATTROPEN
        fprintf(f, _("   -a\t\tread the extended attributes: r_uid, r_gid\n"));
#endif /* HAVE_ATTROPEN */
        fprintf(f,
                _("   -c\t\tcat the contents (FORMAT=\"%%p%%T %%b %%u %%g %%l %%s\\n%%n%%C\")\n"));
        fprintf(f, _("   -h\t\tthis help\n"));
        fprintf(f, _("   -m\t\tonly print new/modified files (unsets -r)\n"));
        fprintf(f, _("   -l\t\tdon't check for file size changes\n"));
        fprintf(f, _("   -n\t\tignore %s files\n"), NOBACKUP);
        fprintf(f, _("   -r\t\tonly print removed files (unsets -m)\n"));
        fprintf(f, _("   -s SIZE\tonly output files smaller then SIZE bytes\n"));
        fprintf(f, _("   -v\t\tbe more verbose\n"));
        fprintf(f, _("   -x\t\tstay in local file system\n"));
        fprintf(f, _("\nFORMAT:\n"));
        fprintf(f, _("   The following escape sequences are recognized:\n"));
        fprintf(f, _("   \'%%p\': '+' if new, '-' if removed\n"));
        fprintf(f, _("   \'%%b\': permission bits\n"));
        fprintf(f, _("   \'%%m\': file mode bits\n"));
        fprintf(f, _("   \'%%u\': uid\n"));
        fprintf(f, _("   \'%%g\': gid\n"));
        fprintf(f, _("   \'%%l\': path length\n"));
        fprintf(f, _("   \'%%s\': file size\n"));
        fprintf(f, _("   \'%%n\': path\n"));
        fprintf(f, _("   \'%%t\': time of modification (epoch)\n"));
        fprintf(f, _("   \'%%H\': the sha1 hash of the file's contents\n"));
        fprintf(f, _("   \'%%T\': \'type\' (d, l or -: dir, link or file)\n"));
        fprintf(f, _("   \'%%C\': file contents\n"));
        fprintf(f, _("\nReport bugs to <miek@miek.nl>\n"));
        fprintf(f, _("Licensed under the GPL version 3.\nSee the file LICENSE in the "));
        fprintf(f, _("source distribution of rdup.\n"));
}
