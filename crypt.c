/* 
 * Copyright (c) 2009 Miek Gieben
 * crypt.c
 * encrypt/decrypt paths
 * struct r_entry
 */

#include "rdup-tr.h"

#include <nettle/aes.h>

/** init the cryto
 * with key  *key
 * and length length
 * lenght MUST be 16, 24 or 32
 * return aes context
 */
struct aes_ctx *
init(gchar *key, guint length, gboolean crypt)
{
	struct aes_ctx *ctx = g_malloc(sizeof(struct aes_ctx));
	if (crypt)
		aes_set_encrypt_key(ctx, length, (uint8_t*)key);
	else 
		aes_set_decrypt_key(ctx, length, (uint8_t*)key);
	return ctx;
}
