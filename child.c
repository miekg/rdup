/* 
 * Copyright (c) 2009 Miek Gieben
 * child-tr.c handle child stuff
 */

#include "rdup-tr.h"

extern sig_atomic_t sig;
extern gint opt_verbose;

/* signal.c */
void got_sig(int signal);

/* close all pipes except no1 and no2 (if not -1) */
void
close_pipes(GSList *pipes, int no1, int no2)
{
	GSList *p;
	int *q;
	int j;

	for (j = 0, p = g_slist_nth(pipes, 0); p; p = p->next, j++) { 
		q = p->data;
		if ( (j != -1 && j != no1) && (j != -1 && j != no2) ) {
			if (opt_verbose > 0) 
				msg("closing pipe %d\n", j);
			close(q[0]);
			close(q[1]);
		}
	}
}

void
wait_pids(GSList *pids)
{
	GSList *p;

	if (!pids)
		return;

	for (p = g_slist_nth(pids, 0); p; p = p->next) { 
                if (sig != 0)
                        signal_abort(sig);

		if (opt_verbose > 0)
			msg("waiting for child %d", *(pid_t*)(p->data));

		waitpid(*(pid_t* )(p->data), NULL, 0);
	}
}

/* create pipes and childs, return pids */
GSList *
create_childeren(GSList *child, GSList **pipes, int tmpfile) 
{
	GSList  *p;
	GSList	*pids	= NULL;
	GSList	*cpipe  = NULL;

	char	**args;
	int	*pips;
	int	childs, j;
	pid_t	*cpid;

	if (!child)
		return NULL;

	/* create ALL pipes before forking and one more
	 * for the parent child communication
	 */
	childs = g_slist_length(child);
	for (j = 0; j < childs; j++) { 
		pips = g_malloc(2 * sizeof(int));
		if (pipe(pips) == -1) {
			msg("Failure creating pipes");
			exit(EXIT_FAILURE);
		}
		cpipe = g_slist_append(cpipe, pips);
	}	

	for (j = 0, p = g_slist_nth(child, 0); p; p = p->next, j++) { 
                if (sig != 0)
                        signal_abort(sig);

		/* fork, exec, child */
                args = (char**) p->data;
		cpid = g_malloc(sizeof(pid_t));
		pips = (g_slist_nth(cpipe, j))->data;

		if ( (*cpid = fork()) == -1) {
			msg("Error forking");
			exit(EXIT_FAILURE);
		}

		if (*cpid != 0) {	/* parent */
			/* save the pids */
			pids = g_slist_append(pids, cpid);
		} else {
			/* child */
			if (j != (childs - 1)) {
				/* not the last one */
				close(tmpfile);

				/* close write end, connect read to stdin */
				close(pips[1]);
				if (dup2(pips[0], 0) == -1) {
					exit(EXIT_FAILURE);
				}

				/* re-use pips */
				pips = (g_slist_nth(cpipe, j + 1))->data;

				/* close read end, connect write to stdout */
				close(pips[0]);
				if (dup2(pips[1], 1) == -1) {
					exit(EXIT_FAILURE);
				}

				close_pipes(cpipe, j, j + 1);
			} else {
				/* last one, conn stdout to tmpfile */
				if (dup2(tmpfile, 1) == -1) {
					exit(EXIT_FAILURE);
				}

				/* close write end, connect read to stdin */
				close(pips[1]);
				if (dup2(pips[0], 0) == -1) {
					exit(EXIT_FAILURE);
				}
				close_pipes(cpipe, j, -1);
			}

			/* finally ... exec */
			if ( execvp(args[0], args) == -1) {
				msg("Failed to exec `%s\': %s\n", args[0], strerror(errno));
				exit(EXIT_SUCCESS);
			}
			/* never reached */
			exit(EXIT_SUCCESS);
		}
        }
	/* all childeren created, close all pipes except 0 */
	close_pipes(cpipe, 0, -1);
	/* close read end, we only need to write as parent */
	pips = (g_slist_nth(cpipe, 0))->data;
	close(pips[0]);

	*pipes = cpipe;
	return pids;
}
