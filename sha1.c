/* sha1.c - copied from
   /usr/share/doc/libnettle-dev/examples/sha-example.c
   */

#include "rdup.h"
#include <nettle/sha.h>

int
sha1_stream(FILE *f, unsigned char *digest)
{
	struct sha1_ctx ctx;
	uint8_t buffer[BUFSIZE];
	/*  uint8_t digest[SHA1_DIGEST_SIZE]; */

	sha1_init(&ctx);
	for (;;)
	{
		guint done = fread(buffer, 1, sizeof(buffer), f);
		sha1_update(&ctx, done, buffer);
		if (done < sizeof(buffer))
			break;
	}
	if (ferror(f))
		return -1;

	sha1_digest(&ctx, SHA1_DIGEST_SIZE, digest);
	return 0;  
}
