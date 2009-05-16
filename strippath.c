#include "rdup-up.h"

/* strip n components from pathname
 * sets path to  NULL when the path didn't contain enough
 * components to begin with 
 *
 * sym- and hardlinks are handled as follows:
 * with path1 -> path2, only path1 is modified (shortened)
 * the pathlen is adjusted accordingly and the
 * and the filesize too (if that is overloaded which is
 * the case with links)
 */

extern guint opt_strip;

void
strippath(struct r_entry *e)
{
	char *p;
	guint i;

	fprintf(stderr, "%zd %zd %s\n", 
			e->f_name_size, (guint) e->f_size, e->f_name);

	/* links */
	if (S_ISLNK(e->f_mode) || e->f_lnk == 1) 
		e->f_name[e->f_size] = '\0';

	for(i = 1, p = strchr(e->f_name, '/'); p; p = strchr(p + 1, '/'), i++) {
		if (i > opt_strip)
			break;
	}

	if (S_ISLNK(e->f_mode) || e->f_lnk == 1) 
		e->f_name[e->f_size] = ' ';

	if (!p) {
		e->f_name = NULL;
		return;
	} else {
		e->f_name = p;
	}

	/* how much shorter are we? */
	i = e->f_name_size - strlen(p);
	e->f_name_size -= i; 
	/* for links also shorten the start of the '->' */
	if (S_ISLNK(e->f_mode) || e->f_lnk == 1)
		e->f_size -= i;

	fprintf(stderr, "%zd %zd %s\n", 
			e->f_name_size, (guint) e->f_size, e->f_name);
	return;
}
