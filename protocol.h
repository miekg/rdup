#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define PROTO_BLOCK	"BLOCK"
#define PROTO_VERSION_MAJOR	0
#define	PROTO_VERSION_MINOR	1

/* protocol.c */
gint block_out_header(FILE *f, size_t size);
gint block_out(FILE *f, size_t size, char *buf);
size_t block_in_header(FILE *f);
gint block_in(FILE *f, size_t size, char *buf);
#endif  /* _PROTOCOL_H */
