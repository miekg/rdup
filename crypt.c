/* 
 * Copyright (c) 2009 Miek Gieben
 * crypt.c
 * encrypt/decrypt paths
 * struct r_entry
 */

#include "rdup-tr.h"
#include "base64.h"

#ifdef HAVE_LIBSSL
#include <openssl/blowfish.h>

extern gint opt_verbose;
extern sig_atomic_t sig;

/* signal.c */
void got_sig(int);
void signal_abort(int);

EVP_CIPHER_CTX *
crypt_init(gchar *key, gboolean crypt)
{
	/* see blowfish(3) */
	/* guint length = strlen(key); */
	EVP_CIPHER_CTX *ctx = g_malloc(sizeof(EVP_CIPHER_CTX));
	gint i;
	guchar iv[] = {1,2,3,4,5,6,7,8};
	EVP_CIPHER_CTX_init(ctx);
	if (crypt)
		i = EVP_EncryptInit_ex(ctx, EVP_bf_cbc(), NULL, (guchar*)key, iv);
	else 
		i = EVP_DecryptInit_ex(ctx, EVP_bf_cbc(), NULL, (guchar*)key, iv);

	if (i == 0) {
		msg(_("Failed to setup encryption"));
		return NULL;
	}
	return ctx;
}

static gboolean
is_plain(gchar *s) 
{
	char *p;
	for (p = s; *p; p++) {
		if (sig != 0) signal_abort(sig);
		if (!isascii(*p))
			return FALSE;
	}
		
	return TRUE;
}

/*
 * don't do anything with the strings .. and .
 */
static gchar *
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

static gint
bf_encrypt(EVP_CIPHER_CTX *ctx, gchar *dest, gchar *source, guint slen)
{
	int outlen, tmplen;
	if (! EVP_EncryptUpdate(ctx, (guchar*)dest, &outlen, (guchar*)source, slen))
		return -1;

	if (! EVP_EncryptFinal_ex(ctx, (guchar*)dest + outlen, &tmplen))
		return -1;

	outlen += tmplen;
	return outlen;
}

static gint
bf_decrypt(EVP_CIPHER_CTX *ctx, gchar *dest, gchar *source, guint slen)
{
	int outlen, tmplen;
	if (! EVP_DecryptUpdate(ctx, (guchar*)dest, &outlen, (guchar*)source, slen))
		return -1;

	if (! EVP_DecryptFinal_ex(ctx, (guchar*)dest + outlen, &tmplen))
		return -1;

	outlen += tmplen;
	return outlen;
}

/* encrypt and base64 encode path element
 * return the result
 */
gchar *
crypt_path_ele(EVP_CIPHER_CTX *ctx, gchar *elem, guint len, GHashTable *tr)
{
	gchar *dest;
	gchar *b64, *hashed;

	hashed = g_hash_table_lookup(tr, elem);
	if (hashed) 
		return hashed;
	/* BUGBUF the size here */
	dest = g_malloc0(BUFSIZE);

	if (bf_encrypt(ctx, dest, elem, len) == -1)
		return NULL;
	
	b64 = encode_base64(len * 2, (guchar*)dest);

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
decrypt_path_ele(EVP_CIPHER_CTX *ctx, gchar *b64, guint len, GHashTable *tr)
{
	gchar *dest;
	gchar *crypt, *hashed;

	hashed = g_hash_table_lookup(tr, b64);
	if (hashed)
		return hashed;

	/* BUGBUG sizes here */
	crypt = g_malloc0(BUFSIZE);
	dest  = g_malloc0(BUFSIZE);

	if (!decode_base64((unsigned char*)crypt, (char*)b64))
		return b64;
	
	if (bf_decrypt(ctx, dest, crypt, len) == -1)
		return NULL;
	
	g_free(crypt);

	/* we could have gotten valid string to begin with
	 * if the result is now garbled instead of nice plain
	 * text assume this was the case. 
	 */
	if (!is_plain(dest)) {
		if (opt_verbose > 2)
			msg(_("Returning original string `%s\'"), b64);

		g_free(dest);
		dest = g_strdup(b64);
	} 
	g_hash_table_insert(tr, b64, dest);
	return dest;
}

/** 
 * encrypt an entire path
 */
gchar *
crypt_path(EVP_CIPHER_CTX *ctx, gchar *p, GHashTable *tr) {
	gchar *q, *c, *t, *crypt, *xpath, d;
	gboolean abs;

	/* links might have relative targets */
	abs = g_path_is_absolute(p);

	xpath = NULL;
	for (q = (p + abs); (c = strchr(q, '/')); q++) {
		d = *c;
		*c = '\0';	

		if (sig != 0) signal_abort(sig);

		/* don't decrypt '..' and '.' */
		if ( (t = dot_dotdot(q, xpath, abs)) ) {
			xpath = t;
			q = c;
			*c = d;
			continue;
		}
		if (! (crypt = crypt_path_ele(ctx, q, strlen(q), tr)))
			return NULL;

		if (xpath)
			xpath = g_strdup_printf("%s/%s", xpath, crypt);
		else 
			abs ? (xpath = g_strdup_printf("/%s", crypt)) :
				(xpath = g_strdup(crypt));
		q = c;
		*c = d;
	}
	if (! (crypt = crypt_path_ele(ctx, q, strlen(q), tr)))
		return NULL;

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
decrypt_path(EVP_CIPHER_CTX *ctx, gchar *x, GHashTable *tr) {

	gchar *path, *q, *c, *t, *plain, d;
	gboolean abs;

	/* links */
	abs = g_path_is_absolute(x);

	path = NULL;
	for (q = (x + abs); (c = strchr(q, '/')); q++) {
		d = *c;
		*c = '\0';	

		if (sig != 0) signal_abort(sig);

		/* don't decrypt '..' and '.' */
		if ( (t = dot_dotdot(q, path, abs)) ) {
			path = t;
			q = c;
			*c = d;
			continue;
		}
		if (! (plain = decrypt_path_ele(ctx, q, strlen(q), tr)))
			return NULL;

		if (path) 
			path = g_strdup_printf("%s/%s", path, plain);
		else
			abs ? (path = g_strdup_printf("/%s", plain)) :
				(path = g_strdup(plain));
		q = c;
		*c = d;
	}
	if (! (plain = decrypt_path_ele(ctx, q, strlen(q), tr)))
		return NULL;

	if (path) 
		path = g_strdup_printf("%s/%s", path, plain);
	else
		abs ? (path = g_strdup_printf("/%s", plain)) :
			(path = g_strdup(plain));
	return path;
}

/**
 * Read the key from a file
 */
gchar *
crypt_key(gchar *file) 
{
	FILE *f;
	gchar *buf;
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

		if (sig != 0) signal_abort(sig);

		msg(_("Failed to read key from `%s\': %s"),
				file, strerror(errno));
		g_free(buf);
		return NULL;
	}

	buf[strlen(buf) - 1] = '\0';		/* kill \n */
	return buf;
}
#endif /* HAVE_LIBSSL */
