#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define PROTO_BLOCK	"BLOCK"
#define PROTO_VERSION_MAJOR	'0'
#define	PROTO_VERSION_MINOR	'1'

/* protocol.c */
gint	block_out_header(FILE *, size_t, int);
gint	block_out(FILE *, size_t, char *, int);
size_t	block_in_header(FILE *);
gint	block_in(FILE *, size_t, char *);
#endif  /* _PROTOCOL_H */
