/* sha1.c - copied from
   /usr/share/doc/libnettle-dev/examples/sha-example.c
   */

#include "rdup.h"

#ifdef HAVE_LIBNETTLE
#include <nettle/sha.h>
#endif				/* HAVE_LIBNETTLE */

#ifndef HAVE_LIBNETTLE
int sha1_stream( __attribute__ ((unused)) FILE * f, __attribute__ ((unused))
		unsigned char *digest)
#else
int sha1_stream(FILE * f, unsigned char *digest)
#endif
{

#ifdef HAVE_LIBNETTLE
	struct sha1_ctx ctx;
	uint8_t buffer[SHA1_DIGEST_SIZE];

	sha1_init(&ctx);
	for (;;) {
		guint done = fread(buffer, 1, sizeof(buffer), f);
		sha1_update(&ctx, done, buffer);
		if (done < sizeof(buffer))
			break;
	}
	if (ferror(f))
		return -1;

	sha1_digest(&ctx, SHA1_DIGEST_SIZE, digest);
#endif				/* HAVE_LIBNETTLE */
	return 0;
}
