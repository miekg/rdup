/*
 * Copyright (c) 2005 - 2011 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

extern int sig;

void
got_sig(int signal)
{
	sig = signal;
}

/**
 * we received a signal
 */
void
signal_abort(int signal)
{
	int status;
        switch(signal) {
                case SIGPIPE:
                        msg(_("SIGPIPE received, exiting"));
                        break;
                case SIGINT:
                        msg(_("SIGINT received, exiting"));
                        break;
		case SIGCHLD:
			(void)wait(&status);
#ifdef DEBUG
			msg("Wait stat: %d\n", status);
#endif /* DEBUG */
			return;
                default:
                        msg(_("Unhandled signal reveived, exiting"));
                        break;
        }
        exit(EXIT_FAILURE);
}
