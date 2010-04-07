#include "rdup-up.h"

/* strip n components from pathname
 * sets path to  NULL when the path didn't contain enough
 * components to begin with
 *
 * Symlinks are handled as follows:
 * with path1 -> path2, only path1 is modified (shortened)
 * the pathlen is adjusted accordingly and the
 * and the filesize too (if that is overloaded which is
 * the case with links)
 *
 * For hardlinks we must also strip the part after the ->
 * as all hardlinks fall in the backed up directory part
 */

extern guint opt_strip;
extern gchar *opt_path_strip;
extern guint opt_path_strip_len;

/* Count the number of slashes in a string (=path) */
static guint pathlabel(struct rdup *e)
{
	gint i, j = 0;

	for(i = 0; i < (S_ISLNK(e->f_mode) || e->f_lnk == 1 ? e->f_size : (gint)e->f_name_size); i++) {
		if (e->f_name[i] == '/')
			j++;
	}
	return j;
}

/* this implements the -s option */
void
strippath(struct rdup *e)
{
	char *p;
	guint i;

	if (! e->f_name)
		return;

	for(i = 1, p = strchr(e->f_name, '/'); p; p = strchr(p + 1, '/'), i++) {
		if (i > opt_strip)
			break;
	}

	if (!p) {
		e->f_name = NULL;
		e->f_name_size = 0;
		return;
	} else {
		e->f_name = p;
	}
	e->f_name_size = strlen(p);

	if (e->f_lnk == 1) {
		for(i = 1, p = strchr(e->f_target, '/'); p; p = strchr(p + 1, '/'), i++) {
			if (i > opt_strip)
				break;
		}
		e->f_target = p;
		e->f_size = strlen(e->f_target);
	}
	return;
}

/* this implements the -r options, strip opt_strip_path from each name */
void
strippathname(struct rdup *e)
{
	gchar *where;
	guint len;

	if (! e->f_name)
		return;

	/* the other way around, if the path is a prefix of the prefix
	 * we should discard the entry. But only is the path we are looking
	 * at is LONGER than the prefix
	 */
	(void)pathlabel(e);
#ifdef DEBUG
	msgd(__func__, __LINE__,_("Label %d, strippathlen %d\n"), pathlabel(e), 
			opt_path_strip_len);
#endif /* DEBUG */
/*	if (pathlabel(e) > opt_path_strip_len) { */
		if (g_str_has_prefix(opt_path_strip, e->f_name)) {
			e->f_name = NULL;
			e->f_name_size = 0;
			return;
		}
/*	} */

	if (g_str_has_prefix(e->f_name, opt_path_strip) == FALSE)
		return;

	len = strlen(opt_path_strip) - 1; /* -1: discard the trailing slash */
	where = e->f_name + len;

	/* string starts with opt_path_strip, so we can just jump
	 * into e->f_name[strlen of opt_path_strip] and discard
	 * everything before
	 */
	memmove(e->f_name, where, e->f_name_size - len);
	e->f_name_size -= len;
	e->f_name[e->f_name_size] = '\0';
	
	if (S_ISLNK(e->f_mode) || e->f_lnk == 1)
		e->f_size -= len;

	/* hardlinks need to get the same treatment */
	if (e->f_lnk == 1) {
		if (g_str_has_prefix(e->f_target, opt_path_strip) == TRUE) {
			where = e->f_target + len;
			memmove(e->f_target, where, strlen(e->f_target) - len);
			e->f_size -= len;
			e->f_target[e->f_size] = '\0';
		}
	}
	return;
}
