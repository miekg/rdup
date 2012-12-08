/*
 * lookup the user/group names associated with the uid/gid
 * Use hashes to speed things up
 */

#include "rdup.h"

/* lookup the uid belonging to username
 * if the uid is not found, return uid_given
 * otherwise return the uid belonging to the username
 * ON THIS SYSTEM
 */
uid_t
lookup_uid(GHashTable *u, gchar *user, uid_t uid_given)
{
	uid_t uid, *uid_tmp;
	struct passwd *p;

	if (uid_given == 0)
		return 0;

	uid_tmp = (uid_t*)g_hash_table_lookup(u, user);
	if (uid_tmp)
		return *uid_tmp;

	p = getpwnam(user);
	if (!p) /* user does not exist on this system */
		return uid_given;

	uid = p->pw_uid;
	uid_tmp = g_malloc(sizeof(uid_t));
	*uid_tmp = uid;
	g_hash_table_insert(u, g_strdup(user), (gpointer)uid_tmp);

	uid_tmp = (uid_t *)g_hash_table_lookup(u, user);

	return *uid_tmp;
}

/* see lookup_uid, but now for groups */
gid_t
lookup_gid(GHashTable *g, gchar *group, gid_t gid_given)
{
	gid_t gid, *gid_tmp;
	struct group *p;

	if (gid_given == 0)
		return 0;

	gid_tmp = (gid_t*)g_hash_table_lookup(g, group);
	if (gid_tmp)
		return *gid_tmp;

	p = getgrnam(group);
	if (!p) /* grp does not exist on this system */
		return gid_given;

	gid = p->gr_gid;
	gid_tmp = g_malloc(sizeof(gid_t));
	*gid_tmp = gid;

	g_hash_table_insert(g, g_strdup(group), (gpointer)gid_tmp);
	
	gid_tmp = (gid_t *)g_hash_table_lookup(g, group);

	return *gid_tmp;
}

gchar *
lookup_user(GHashTable *u, uid_t uid)
{
	gchar *n;
	struct passwd *p;

	n = (gchar *)g_hash_table_lookup(u, (gpointer)&uid);
	if (n)
		return n;

	/* if nothing found also add to hash? */
	p = getpwuid(uid);
	if (!p) /* user only has ID */
		return NULL;

	/* don't return the string as it might be overwritten in
	 * subsequent calls to getpwnam. Use the pointer stored
	 * in the hash. This is also the case for getgrgid()
	 */
	n = g_strdup(p->pw_name);
	g_hash_table_insert(u, (gpointer)&uid, n);
	return (gchar *)g_hash_table_lookup(u, (gpointer)&uid);
}

gchar *
lookup_group(GHashTable *g, gid_t gid)
{
	gchar *n;
	struct group *p;

	n = (gchar *)g_hash_table_lookup(g, (gpointer)&gid);
	if (n)
		return n;

	p = getgrgid(gid);
	if (!p) /* group only has ID */
		return NULL;

	n = g_strdup(p->gr_name);
	g_hash_table_insert(g, (gpointer)&gid, n);
	return (gchar *)g_hash_table_lookup(g, (gpointer)&gid);
}
