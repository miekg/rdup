#include "rdup.h"
#include <openssl/sha.h>

int
sha1_stream(FILE *f, unsigned char *digest)
{
	SHA_CTX *c = NULL;
	guint buffer[BUFSIZE];
/*	SHA_DIGEST_LENGTH  */

	SHA1_Init(c);
	for (;;)
	{
		guint done = fread(buffer, 1, sizeof(buffer), f);
		SHA1_Update(c, buffer, done);
/*		sha1_update(&ctx, done, buffer);*/
		if (done < sizeof(buffer))
			break;
	}
	if (ferror(f))
		return -1;

	SHA1_Final(digest, c);
	return 0;  
}
