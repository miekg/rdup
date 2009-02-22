/* 
 * Copyright (c) 2009 Miek Gieben
 * crypt.c
 * encrypt/decrypt paths
 * struct r_entry
 */

#include "rdup-tr.h"
#include "base64.h"
#include <nettle/aes.h>

/** init the cryto
 * with key  *key
 * and length length
 * lenght MUST be 16, 24 or 32
 * return aes context
 */
struct aes_ctx *
crypt_init(gchar *key, guint length, gboolean crypt)
{
	struct aes_ctx *ctx = g_malloc(sizeof(struct aes_ctx));
	if (crypt)
		aes_set_encrypt_key(ctx, length, (uint8_t*)key);
	else 
		aes_set_decrypt_key(ctx, length, (uint8_t*)key);
	return ctx;
}

/* encrypt and base64 encode path element
 * return the result
 */
gchar *
crypt_path(struct aes_ctx *ctx, gchar *elem, guint len)
{
	guint aes_size;
	guchar *source;
	guchar *dest;
	gchar *b64;

	aes_size = ( (len / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;

	/* pad the string to be crypted */
	source = g_malloc0(aes_size);
	dest   = g_malloc0(aes_size);
	
	g_memmove(source, elem, len);
	aes_encrypt(ctx, aes_size, dest, source);
	
	b64 = encode_base64(aes_size, dest);
	g_free(source);
	g_free(dest);
	if (!b64) {
		return elem; /* as if nothing happened */
	} else {
		return b64;
	}
}
