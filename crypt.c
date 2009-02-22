/* 
 * Copyright (c) 2009 Miek Gieben
 * crypt.c
 * encrypt/decrypt paths
 * struct r_entry
 */

#include "rdup-tr.h"
#include "base64.h"
#include <nettle/aes.h>

extern GHashTable *trhash;

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

static gboolean
is_plain(gchar *s) {
	char *p;
	for (p = s; *p; p++)
		if (!isalnum(*p))
			return FALSE;
		
	return TRUE;
}

/* encrypt and base64 encode path element
 * return the result
 */
gchar *
crypt_path_ele(struct aes_ctx *ctx, gchar *elem, guint len, GHashTable *tr)
{
	guint aes_size;
	guchar *source;
	guchar *dest;
	gchar *b64, *hashed;

	hashed = g_hash_table_lookup(tr, elem);
	if (hashed) 
		return hashed;

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
		/* hash insert? */
		return elem; /* as if nothing happened */
	} else {
		g_hash_table_insert(tr, elem, b64);
		return b64;
	}
}

/* decrypt and base64 decode path element
 * return the result
 */
gchar *
decrypt_path_ele(struct aes_ctx *ctx, char *b64, guint len, GHashTable *tr)
{
	guint aes_size;
	guchar *source;
	guchar *dest;
	gchar *crypt, *hashed;
	guint crypt_size;

	hashed = g_hash_table_lookup(tr, b64);
	if (hashed)
		return hashed;
	crypt = g_malloc(len); /* is this large enough? XXX */

	crypt_size = decode_base64((guchar*)crypt, b64);
	if (!crypt_size)
		return b64;

	aes_size = ( (crypt_size / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;

	/* pad the string to be crypted */
	source = g_malloc0(aes_size);
	dest   = g_malloc0(aes_size);

	g_memmove(source, crypt, crypt_size);
	aes_decrypt(ctx, aes_size, dest, source);
	
	g_free(source);
	g_free(crypt);

	/* we could have been valid string to begin with
	 * if the result is now garbled instead of nice plain
	 * test assume this was the case. 
	 */
	if (!is_plain((char*) dest)) {
		g_free(dest);
		dest = (guchar*) g_strdup(b64);
	} 
	g_hash_table_insert(tr, b64, dest);
	return (gchar*) dest;
}

/** 
 * encrypt an entire path
 */
gchar *
crypt_path(struct aes_ctx *ctx, gchar *p, GHashTable *tr) {
	gchar *q, *c, *crypt, *xpath, d;

	if (!g_path_is_absolute(p))
		return NULL;

	xpath = NULL;
	/* p + 1, path should be absolute */
	for (q = p + 1; (c = strchr(q, DIR_SEP)); q++) {
		d = *c;
		*c = '\0';	
		
		/* don't encrypt '..' */
		if (strcmp(q, "..") == 0) {
			if (xpath)
				xpath = g_strdup_printf("%s%c%s", xpath, DIR_SEP, "..");
			else 
				xpath = g_strdup("/..");

			q = c;
			*c = d;
			continue;
		}
		crypt = crypt_path_ele(ctx, q, strlen(q), tr);

		if (xpath)
			xpath = g_strdup_printf("%s%c%s", xpath, DIR_SEP, crypt);
		else 
			xpath = g_strdup_printf("%c%s", DIR_SEP, crypt);

		q = c;
		*c = d;
	}
	crypt = crypt_path_ele(ctx, q, strlen(q), tr);
	if (xpath)
		xpath = g_strdup_printf("%s%c%s", xpath, DIR_SEP, crypt);
	else 
		xpath = g_strdup_printf("%c%s", DIR_SEP, crypt);

	return xpath;
}


/**
 * decrypt an entire path
 */
gchar *
decrypt_path(struct aes_ctx *ctx, gchar *x, GHashTable *tr) {

	gchar *path, *q, *c, *plain, d;

	if (!g_path_is_absolute(x))
		return NULL;

	path = NULL;
	/* x + 1, path should be absolute */
	for (q = x + 1; (c = strchr(q, DIR_SEP)); q++) {
		d = *c;
		*c = '\0';	

		/* don't decrypt '..' */
		if (strcmp(q, "..") == 0) {
			if (path)
				path = g_strdup_printf("%s%c%s", path, DIR_SEP, "..");
			else 
				path = g_strdup("/..");

			q = c;
			*c = d;
			continue;
		}
		plain = decrypt_path_ele(ctx, q, strlen(q), tr);

		if (path) 
			path = g_strdup_printf("%s%c%s", path, DIR_SEP, plain);
		else
			path = g_strdup_printf("%c%s", DIR_SEP, plain);

		q = c;
		*c = d;
	}
	plain = decrypt_path_ele(ctx, q, strlen(q), tr);
	if (path) 
		path = g_strdup_printf("%s%c%s", path, DIR_SEP, plain);
	else
		path = g_strdup_printf("%c%s", DIR_SEP, plain);

	return path;
}
