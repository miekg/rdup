/*
 * Copyright (c) 2009 - 2011 Miek Gieben
 * License: GPLv3(+), see LICENSE for details
 * make a dir writable and later remove that
 * right again
 */

#include "rdup-up.h"

struct stat *dir_write(gchar * p)
{
	/* chmod +w . && rm $file && chmod -w # and hope for the best */
	if (!p)
		return NULL;

	struct stat *s = g_malloc(sizeof(struct stat));
#ifdef DEBUG
	msgd(__func__, __LINE__, _("Making directory writable `%s\'"), p);
#endif				/* DEBUG */

	if (stat(p, s) == -1)
		return NULL;

	/* make it writable, assume we are the OWNER */
	if (chmod(p, s->st_mode | S_IWUSR) == -1) {
		msgd(__func__, __LINE__,
		     _("Failed to make directory writeable `%s\': %s"), p,
		     strerror(errno));
	}
	return s;
}

void dir_restore(gchar * p, struct stat *s)
{
	if (!s || !p)
		return;
	/* restore perms - assumes *s has not be f*cked up */
	if (chmod(p, s->st_mode & 07777) == -1) {
		msgd(__func__, __LINE__,
		     _("Failed to restore permissions `%s\': %s"), p,
		     strerror(errno));
	}
}

/**
 * return parent dir string
 * p MUST not end in a /
 */
gchar *dir_parent(gchar * p)
{
	gchar *p2;
	gchar *n;
	gchar *copy;

	if (!p)
		return NULL;

	if (p[0] == '/' && p[1] == '\0')
		return p;

	copy = g_strdup(p);
	n = strrchr(copy, '/');
	if (n) {
		*(n + 1) = '\0';
		p2 = g_strdup(copy);
		g_free(copy);
		*n = '/';
		return p2;
	}
	return NULL;
}

/**
  * Make sure a path exists
  */

void dir_mkpath(gchar *p)
{
	gchar *parent;

        parent = dir_parent(p);

	if (parent && (parent[0] == 0)) {
		msgd(__func__, __LINE__, _("Reached / while trying to create path, bailing out."));
		g_free(parent);
		return;
	}

#if DEBUG
	msgd(__func__, __LINE__, _("Creating skeleton directory `%s\'"), parent);
#endif

	struct stat *st = g_malloc(sizeof(struct stat));

	if (stat(parent, st) != 0) {
		/* Directory does not exist. EEXIST for race condition */
		if (mkdir(parent, 0700) != 0 && errno != EEXIST) {
			dir_mkpath(parent);
			if (mkdir(parent, 0700) != 0 && errno != EEXIST)
				msgd(__func__, __LINE__, _("Failed to create `%s\'"), parent);
		}
	}
	else if (!S_ISDIR(st->st_mode)) {
		msgd(__func__, __LINE__, _("%s\': already exists and not a directory"), parent);
	}

	g_free(st);
	g_free(parent);
}
