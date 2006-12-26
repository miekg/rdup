/* 
 * Copyright (c) 2005 - 2007 Miek Gieben
 * See LICENSE for the license
 */

#include "rdup.h"

extern sig_atomic_t sig;

void
got_sig(int signal)
{
	sig = signal;
}
