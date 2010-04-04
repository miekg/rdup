/**
 * Copyright (c) 2005 - 2010 Miek Gieben
 * See LICENSE for the license
 *
 * Not used anymore - this is a hack that I don't want to
 * support anymore. It stinks
 *
 * It was used in saving the uid/gid information when doing
 * a remote dump as a no root user.
 *
 * All xattr functions are grouped here
 */

#include "rdup.h"

extern gint opt_verbose;

uid_t
read_attr_uid(__attribute__((unused))
	char *path, uid_t u)
{
#ifdef HAVE_ATTR_XATTR_H
	/* linux */
	char buf[ATTR_SIZE + 1];
	uid_t x;
	int r;

	if ((r = lgetxattr(path, "user.r_uid", buf, ATTR_SIZE)) > 0) {
		x = (uid_t)atoi(buf);
		buf[r - 1] = '\0';
		if (x > R_MAX_ID) {
			msg(_("Too large uid `%zd\' for `%s\', truncating"), (size_t)x,
				path);
			return R_MAX_ID;
		}
		return x;
	} else {
		if (opt_verbose > 0) {
			msg(_("No uid xattr for `%s\'"), path);
		}
		return u;
	}
#elif HAVE_ATTROPEN
	/* solaris */
	char buf[ATTR_SIZE + 1];
	uid_t x;
	int attfd;
	int r;
	if ((attfd = attropen(path, "r_uid", O_RDONLY)) == -1) {
		if (opt_verbose > 0) {
			msg(_("No uid xattr for `%s\'"), path);
		}
		return u;
	}
	if ((r = read(attfd, buf, ATTR_SIZE)) == -1) {
		return u;
	}
	close(attfd);
	buf[r - 1] = '\0';
	x = (uid_t)atoi(buf);
	if (x > R_MAX_ID) {
		msg(_("Too large gid `%zd\' for `%s\', truncating"), (size_t)x, path);
		return R_MAX_ID;
	}
	return x;
#else
	return u;
#endif /* HAVE_ATTR_XATTR_H, HAVE_ATTROPEN */
}

gid_t
read_attr_gid(__attribute__((unused))
	char *path, gid_t g)
{
#ifdef HAVE_ATTR_XATTR_H
	char buf[ATTR_SIZE + 1];
	gid_t x;
	int r;

	if ((r = lgetxattr(path, "user.r_gid", buf, ATTR_SIZE)) > 0) {
		buf[r - 1] = '\0';
		x = (gid_t)atoi(buf);
		if (x > R_MAX_ID) {
			msg(_("Too large gid `%zd\' for `%s\', truncating"), (size_t)x,
					path);
			return R_MAX_ID;
		}
		return x;
	} else {
		if (opt_verbose > 0) {
			msg(_("No gid xattr for `%s\'"), path);
		}
		return g;
	}
#elif HAVE_ATTROPEN
	/* solaris */
	char buf[ATTR_SIZE + 1];
	gid_t x;
	int attfd;
	int r;
	if ((attfd = attropen(path, "r_gid", O_RDONLY)) == -1) {
		if (opt_verbose > 0) {
			msg(_("No gid xattr for `%s\'"), path);
		}
		return g;
	}
	if ((r = read(attfd, buf, ATTR_SIZE)) == -1) {
		return g;
	}
	close(attfd);
	buf[r - 1] = '\0';
	x = (uid_t)atoi(buf);
	if (x > R_MAX_ID) {
		msg(_("Too large gid `%zd\' for `%s\', truncating"), (size_t)x, path);
		return R_MAX_ID;
	}
	return x;
#else
	return g;
#endif /* HAVE_ATTR_XATTR_H, HAVE_ATTROPEN */
}
