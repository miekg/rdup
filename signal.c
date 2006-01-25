/* 
 * Copyright (c) 2005, 2006 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

extern sig_atomic_t sig;

void
sigpipe(__attribute__((unused)) int signal)
{
	sig = SIGPIPE;
}

void
sigint( __attribute__((unused)) int signal)
{
	sig = SIGINT;
}
