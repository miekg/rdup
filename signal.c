/* 
 * Copyright (c) 2005 - 2008 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

extern sig_atomic_t sig;

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
        switch(signal) {
                case SIGPIPE:
                        msg(_("SIGPIPE received, exiting"));
                        break;
                case SIGINT:
                        msg(_("SIGINT received, exiting"));
                        break;
                default:
                        msg(_("Unhandled signal reveived, exiting"));
                        break;
        }
        exit(EXIT_FAILURE);
}
