#include "rdup.h"
#include <openssl/sha.h>

int
sha1_stream(FILE *f, unsigned char *digest)
{
	guint buffer[BUFSIZE], done;
	SHA_CTX *c = g_malloc(sizeof(SHA_CTX));

	SHA1_Init(c);
	for (;;)
	{
		done = fread(buffer, 1, sizeof(buffer), f);
		SHA1_Update(c, buffer, done);
		if (done < sizeof(buffer))
			break;
	}
	if (ferror(f))
		return -1;

	SHA1_Final(digest, c);
	return 0;  
}
