#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define PROTO_BLOCK	"BLOCK"
#define PROTO_VERSION_MAJOR	'0'
#define	PROTO_VERSION_MINOR	'1'

#include <glib.h>
#include <stdio.h>
#include "config.h"

#ifdef HAVE_GETTEXT
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#else
#define _(String) String
#endif /* HAVE_GETTEXT */


/* protocol.c */
gint	block_out_header(FILE *, size_t, int);
gint	block_out(FILE *, size_t, char *, int);
size_t	block_in_header(FILE *);
gint	block_in(FILE *, size_t, char *);
void	msg(const char *, ...);
#endif  /* _PROTOCOL_H */
