#include "rdup-up.h"

/* strip n components from pathname
 * and return the name
 * returns NULL when the path didn't contain enough
 * components to begin with otherwise a modified entry
 * is returned!
 * sym- and hardlinks are handled as follows:
 * with path1 -> path2, only path1 is modified (shortened)
 * the pathlen is adjusted accordingly and the
 * and the filesize too (if that is overloaded which is
 * the case with links)
 */

struct r_entry *
strippath(struct r_entry *e, guint strip)
{
	char *p;
	guint i;

	fprintf(stderr, "%zd %zd %s", 
			e->f_name_size, (guint) e->f_size, e->f_name);

	for(i = 1, p = strchr(e->f_name, '/'); p; p = strchr(p + 1, '/'), i++) {
		if (i > strip)
			break;
	}
	if (!p)
		return NULL;

	/* how much shorter are we? */
	i = e->f_name_size - strlen(p);
	/* shorten the name */
	e->f_name_size =- i;
	/* for links also shorten the start of the '->' */
	if (S_ISLNK(e->f_mode)|| e->f_lnk == 1)
		e->f_size -= i;

	fprintf(stderr, "%zd %zd %s", 
			e->f_name_size, (guint) e->f_size, e->f_name);
	return e;
}
