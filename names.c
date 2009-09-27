/* 
 * lookup the user/group names associated with the uid/gid
 * Use hashes to speed things up
 */

#include "rdup.h"

gchar *
lookup_user(GHashTable *u, uid_t uid)
{
	gchar *n;
	struct passwd *p;

	n = (gchar *)g_hash_table_lookup(u, (gpointer)&uid);
	if (n) 
		return n;

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

	fprintf(stderr, "Slow look up for %d\n", (int) gid);
	p = getgrgid(gid);
	if (!p) /* group only has ID */
		return NULL;

	n = g_strdup(p->gr_name);
	g_hash_table_insert(g, (gpointer)&gid, n);
	return (gchar *)g_hash_table_lookup(g, (gpointer)&gid);
}
