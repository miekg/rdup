/*
 * Copyright (c) 2009 - 2011 Miek Gieben
 * See LICENSE for the license
 * child.c handle child stuff
 */

#include "rdup.h"

extern int sig;

/* signal.c */
void got_sig(int signal);

/* close all pipes except n1 and n2 (-1 is not needed) */
void close_pipes(GSList * pipes, int n1, int n2)
{
	GSList *p;
	int *q;
	int j;

	for (j = 0, p = g_slist_nth(pipes, 0); p; p = p->next, j++) {
		q = p->data;
/*		msg("Pipe set %d fds %d %d", j, q[0], q[1]); */
		if (j == n1 || j == n2)
			continue;
		close(q[0]);
		close(q[1]);
	}
}

/* return 0 if all ok, -1 for trouble */
int wait_pids(GSList * pids, int flags)
{
	GSList *p;
	int status;

	for (p = g_slist_nth(pids, 0); p; p = p->next) {
		if (sig != 0)
			signal_abort(sig);

#ifdef DEBUG
		msgd(__func__, __LINE__, "Waiting for pid %d",
		     (int)*(pid_t *) (p->data));
#endif				/* DEBUG */

		/* -1 on error */
		waitpid(*(pid_t *) (p->data), &status, flags);	/* errno ECHILD is ok */
#if 0
		if (WIFEXITED(status)) {
			/* msg("Child exit %d", WEXITSTATUS(status));
			   if (WEXITSTATUS(status) != 0)
			   ret = -1;
			 */
			/* assume ok */
			ret = 0;
		} else {
			ret = -1;
		}
#endif
	}
	return 0;
}

/* create pipes and childs, return pids */
GSList *create_children(GSList * child, GSList ** pipes, int file)
{
	GSList *p;
	GSList *pids = NULL;
	GSList *cpipe = NULL;

	char **args;
	int *pips;
	int childs, j;
	pid_t *cpid;

	if (!child)
		return NULL;

	/* create all pipes before forking
	 * As a parent we read from the last pipe created
	 * We attach file to the input of the first child
	 * This eliminates to use of a tmp file
	 */
	childs = g_slist_length(child);
	for (j = 0; j < childs; j++) {
		pips = g_malloc(2 * sizeof(int));
		if (pipe(pips) == -1) {
			msg(_("Failure creating pipes"));
			exit(EXIT_FAILURE);
		}
		cpipe = g_slist_append(cpipe, pips);
	}

	for (j = 0, p = g_slist_nth(child, 0); p; p = p->next, j++) {
		if (sig != 0)
			signal_abort(sig);

		/* fork, exec child */
		args = (char **)p->data;
		cpid = g_malloc(sizeof(pid_t));
		pips = (g_slist_nth(cpipe, j))->data;

		if ((*cpid = fork()) == -1) {
			msg(_("Fork error"));
			return NULL;	/* more gracefull then exit */
			/* exit(EXIT_FAILURE); */
		}

		if (*cpid != 0) {	/* parent */
			/* save the pids */
			pids = g_slist_append(pids, cpid);
		} else {	/* child */
			if (j == 0) {
				/* dup f to stdin */
				if (dup2(file, 0) == -1)
					exit(EXIT_FAILURE);
				close(pips[0]);
				/* pips[1] still alive */
				if (dup2(pips[1], 1) == -1)
					exit(EXIT_FAILURE);

				close_pipes(cpipe, j, -1);
			} else {
				close(file);
				/* close read end */
				close(pips[0]);
				/* dup write to stdout */
				if (dup2(pips[1], 1) == -1)
					exit(EXIT_FAILURE);

				/* re-use pips for the previous pipe pair */
				pips = (g_slist_nth(cpipe, j - 1))->data;
				/* dup read to stdin */
				if (dup2(pips[0], 0) == -1)
					exit(EXIT_FAILURE);
				/* and close 1 */
				close(pips[1]);

				close_pipes(cpipe, j, j - 1);
			}

			/* finally ... exec */
			if (execvp(args[0], args) == -1) {
				msg(_("Failed to exec `%s\': %s"), args[0],
				    strerror(errno));
				exit(EXIT_FAILURE);
			}

			/* never reached */
			exit(EXIT_SUCCESS);
		}
	}
	/* all childeren created, close all pipes except the last one */
	close_pipes(cpipe, childs - 1, -1);
	/* close write end, we only need to read as parent */
	pips = (g_slist_last(cpipe))->data;
	close(pips[1]);

	*pipes = cpipe;
	return pids;
}
