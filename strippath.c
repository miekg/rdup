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

/*
 * for hardlinks we must also strip the part after the ->
 * as all hardlinks fall in the backed up directory part
 */

extern guint opt_strip;
extern gchar *opt_path_strip;
extern guint opt_path_strip_len;

/* Count the number of slashes in a string (=path) */
static guint pathlabel(struct r_entry *e) {
	gint i, j = 0;

	for(i = 0; i < (S_ISLNK(e->f_mode) || e->f_lnk == 1 ? e->f_size : e->f_name_size); i++) {
		if (e->f_name[i] == '/')
			j++;
	}
	return j;
}

/* this implements the -s option */
void
strippath(struct r_entry *e)
{
	char *p;
	guint i;

	/* links */
	if (S_ISLNK(e->f_mode) || e->f_lnk == 1)
		e->f_name[e->f_size] = '\0';

	if (e->f_lnk == 1) {
		/* hardlinks... mangle the part after -> also */
#if 0
		fprintf(stderr, "orig %d %d %s\n",
				e->f_name_size, (int)e->f_size, e->f_name);
#endif
		for(i = 1, p = strchr(e->f_name + e->f_size + 1 , '/');
				p; p = strchr(p + 1, '/'), i++) {
			if (i > opt_strip)
				break;
		}
		/* p == NULL - we should get the same below. ie. entry
		 * is discarded
		 */
		if (p) {
			/* how much shorter are we */
			guint shorter;
			i = strlen(p); 
			shorter = (e->f_name_size - e->f_size - i - 4);
			memmove(e->f_name + e->f_size + 4, p, i + 1);
			/* make f_name_size shorter too */
			e->f_name_size -= shorter;
		}
	}

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
	return;
}

/* this implements the -r options, strip opt_strip_path from each name */
void
strippathname(struct r_entry *e)
{
	gchar *where;
	guint len;

	/* the other way around, if the path is a prefix of the prefix
	 * we should discard the entry. But only is the path we are looking
	 * at is LONGER than the prefix
	 */
	(void)pathlabel(e);
/*	fprintf(stderr, "label %d strippath len %d\n", pathlabel(e), opt_path_strip_len); */
/*	if (pathlabel(e) > opt_path_strip_len) { */
		if (g_str_has_prefix(opt_path_strip, e->f_name)) {
			e->f_name = NULL;
			return;
		}
/*	} */

	if (g_str_has_prefix(e->f_name, opt_path_strip) == FALSE) 
		return;

//fprintf(stderr, "orig: %s\n", e->f_name);

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

	/* everything is ok - except for hardlinks where the part after the ->
	 * needs to get the same treatment */
	if (e->f_lnk == 1) {
		where = e->f_name + e->f_size + 4 + len;
		memmove(e->f_name + e->f_size + 4, where, strlen(where));
		e->f_name_size -= len;
		e->f_name[e->f_name_size] = '\0';
		//fprintf(stderr, "%s hard stripped: %s %d %d\n", where, e->f_name, (int)e->f_name_size, (int)e->f_size);

	}
//fprintf(stderr, "stripped: %s %d %d\n", e->f_name, (int)e->f_name_size, (int)e->f_size);
	return;
}
