/* 
 * Copyright (c) 2009,2010 Miek Gieben
 * crypt.c
 * encrypt/decrypt paths
 * struct r_entry
 */

#include "rdup-tr.h"
#include "base64.h"

#ifdef HAVE_LIBNETTLE
#include <nettle/aes.h>

extern guint opt_verbose;


/** 
 * init the cryto
 * with key  *key
 * and length length
 * lenght MUST be 16, 24 or 32
 * anything short will be zero padded to 
 * create a correct key
 * return aes context
 */
struct aes_ctx *
crypt_init(gchar *key, gboolean crypt)
{
	guint length = strlen(key);
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
		if (!isascii(*p))
			return FALSE;
		
	return TRUE;
}

/*
 * don't do anything with the strings .. and .
 */
gchar *
dot_dotdot(gchar *q, gchar *p, gboolean abs) 
{
	gchar *r = NULL;

	if (strcmp(q, "..") == 0) {
		if (p)
			r =  g_strdup_printf("%s/%s", p, "..");
		else
			abs ?  (r = g_strdup("/..")) :
				(r = g_strdup(".."));
	}

	if (strcmp(q, ".") == 0) {
		if (p)
			r =  g_strdup_printf("%s/%s", p, ".");
		else
			abs ?  (r = g_strdup("/.")) :
				(r = g_strdup("."));
	}
	return r;
}

/* encrypt and base64 encode path element
 * return the result
 */
gchar *
crypt_path_ele(struct aes_ctx *ctx, gchar *elem, GHashTable *tr)
{
	guint aes_size, len;
	guchar *source;
	guchar *dest;
	gchar *b64, *hashed;

	len    = strlen(elem);
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
	} else if (strlen(b64) > 255) {
		/* path ele too long. XXX 255 -> symbolic name please */
		msg(_("Encrypted base64 path length exceeds %d characters"), 255);
		return elem;
	} else {
		g_hash_table_insert(tr, elem, b64);
		return b64;
	}
}

/* decrypt and base64 decode path element
 * return the result
 */
gchar *
decrypt_path_ele(struct aes_ctx *ctx, char *b64, GHashTable *tr)
{
	guint aes_size, len;
	guchar *source;
	guchar *dest;
	gchar *crypt, *hashed;
	guint crypt_size;

	len    = strlen(b64);
	hashed = g_hash_table_lookup(tr, b64);
	if (hashed)
		return hashed;
	/* be safe and alloc 2 times what we need */
	crypt = g_malloc(len * 2);

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

	/* we could have gotten valid string to begin with
	 * if the result is now garbled instead of nice plain
	 * text assume this was the case. 
	 */
	if (!is_plain((char*) dest)) {
		if (opt_verbose > 2)
			msg(_("Returning original string `%s\'"), b64);

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
	gchar *q, *c, *t, *crypt, *xpath, d;
	gboolean abs;

	/* links might have relative targets */
	abs = g_path_is_absolute(p);

	xpath = NULL;
	for (q = (p + abs); (c = strchr(q, '/')); q++) {
		d = *c;
		*c = '\0';	

		/* don't decrypt '..' and '.' */
		if ( (t = dot_dotdot(q, xpath, abs)) ) {
			xpath = t;
			q = c;
			*c = d;
			continue;
		}
		crypt = crypt_path_ele(ctx, q, tr);

		if (xpath)
			xpath = g_strdup_printf("%s/%s", xpath, crypt);
		else 
			abs ? (xpath = g_strdup_printf("/%s", crypt)) :
				(xpath = g_strdup(crypt));
		q = c;
		*c = d;
	}
	crypt = crypt_path_ele(ctx, q, tr);
	if (xpath)
		xpath = g_strdup_printf("%s/%s", xpath, crypt);
	else 
		abs ? (xpath = g_strdup_printf("/%s", crypt)) :
			(xpath = g_strdup(crypt));
	return xpath;
}


/**
 * decrypt an entire path
 */
gchar *
decrypt_path(struct aes_ctx *ctx, gchar *x, GHashTable *tr) {

	gchar *path, *q, *c, *t, *plain, d;
	gboolean abs;

	/* links */
	abs = g_path_is_absolute(x);

	path = NULL;
	for (q = (x + abs); (c = strchr(q, '/')); q++) {
		d = *c;
		*c = '\0';	

		/* don't decrypt '..' and '.' */
		if ( (t = dot_dotdot(q, path, abs)) ) {
			path = t;
			q = c;
			*c = d;
			continue;
		}
		plain = decrypt_path_ele(ctx, q, tr);

		if (path) 
			path = g_strdup_printf("%s/%s", path, plain);
		else
			abs ? (path = g_strdup_printf("/%s", plain)) :
				(path = g_strdup(plain));
		q = c;
		*c = d;
	}
	plain = decrypt_path_ele(ctx, q, tr);
	if (path) 
		path = g_strdup_printf("%s/%s", path, plain);
	else
		abs ? (path = g_strdup_printf("/%s", plain)) :
			(path = g_strdup(plain));
	return path;
}

/**
 * Read the key from a file
 * Key must be 16, 24 or 32 octets
 * Check for this - if larger than 32 cut it off
 */
gchar *
crypt_key(gchar *file) 
{
	FILE *f;
	char *buf;
	size_t s;

	buf = g_malloc0(BUFSIZE);
	s = BUFSIZE;
	if (! (f = fopen(file, "r"))) {
		msg(_("Failed to open `%s\': %s"),
				file, strerror(errno));
		g_free(buf);
		return NULL;
	}
	
	if (rdup_getdelim(&buf, &s, '\n', f) == -1) {
		msg(_("Failed to read AES key from `%s\': %s"),
				file, strerror(errno));
		g_free(buf);
		return NULL;
	}

	/* BUGBUG hmm */
	if ( buf[strlen(buf) - 1] == '\n' ) {
		buf[strlen(buf) - 1] = '\0';		/* kill \n */
	}
	s = strlen(buf);
	if (s > 32) {
		msg(_("Maximum AES key size is 32 bytes, truncating!"));
		buf[32] = '\0';
		return buf;
	}
	if (s != 16 && s != 24 && s != 32) {
		msg(_("AES key must be 16, 24 or 32 bytes"));
		g_free(buf);
		return NULL;
	}
	return buf;
}
#endif /* HAVE_LIBNETTLE */
