/* sha1.c - copied from
   /usr/share/doc/libnettle-dev/examples/sha-example.c
   */

#include "rdup.h"
#include <nettle/sha.h>

int
sha1_stream(__attribute__((unused)) FILE *f, unsigned char *digest)
{
#ifdef HAVE_LIBNETTLE
	uint8_t buffer[SHA1_DIGEST_SIZE + 1];
	struct sha1_ctx ctx;

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
#else
	int i;
	for(i = 0; i < SHA1_DIGEST_SIZE; i++) {
		digest[i] = '\0';
	}
#endif /* HAVE_LIBNETTLE */
	return 0;  
}
