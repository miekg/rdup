/*
 * A simple protocol for rdup, if you even want
 * to call it a protocol
 *
 * When rdup output file contents it will do this in blocks.
 *
 * This removes the filesize race conditions
 * and makes everything else somewhat simpler
 *
 * block header is VERSION BLOCK BLOCKSIZE as in
 * 01BLOCK08192
 * So a small 4 byte file becomes:
 *
 * 01BLOCK00004
 * <4 bytes of the file>01BLOCK00000
 *
 * A stop block is a block with length zero 01BLOCK00000, this
 * comes after each file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "protocol.h"

extern gint opt_verbose;
extern int sig;

/* signal.c */
void got_sig(int);
void signal_abort(int);

/**
 * output a block header
 */
gint
block_out_header(FILE *f, size_t size, int fp)
{
	char *p;
	p = g_strdup_printf("%c%c%s%05d\n", PROTO_VERSION_MAJOR,
			PROTO_VERSION_MINOR, PROTO_BLOCK,(int)size);
	if (sig != 0)
		signal_abort(sig);
	if (f) {
		if (fwrite(p, sizeof(char), strlen(p), f) != strlen(p))
			return -1;

	} else if (fp != -1) {
		if (write(fp, p, strlen(p)) == -1)
			return -1;
	}
	g_free(p);
	return 0;
}

/**
 * output a block
 */
gint
block_out(FILE *f, size_t size, char *buf, int fp)
{
	if (sig != 0)
		signal_abort(sig);
	if (f) {
		if (fwrite(buf, sizeof(char), size, f) != size)
			return -1;

	} else if (fp != -1) {
		if (write(fp, buf, size) == -1)
			return -1;
	}
	return 0;
}

/**
 * read a block from f
 */
gint
block_in(FILE *f, size_t size, char *buf)
{
	if (sig != 0)
		signal_abort(sig);
	if (fread(buf, sizeof(char), size, f) != size) {
		/* read less then expected */
		return -1;
	}
	return 0;
}

/**
 * parse a block header: 01BLOCK08192
 * return the number of bytes to read
 * 00000 signals the end (no more bytes to read
 * -1 for parse errors
 */
size_t
block_in_header(FILE *f)
{
	/* we are expecting a block header:
	 * 2 pos version; the word block; 5 digit number; newline
	 */
	char c[6];
	int bytes;

	/* I cannot think of anything smarter */
	/* version check */
	c[0] = fgetc(f);
	if (sig != 0) signal_abort(sig);
	c[1] = fgetc(f);
	if (sig != 0) signal_abort(sig);

	if (c[0] != PROTO_VERSION_MAJOR ||
			c[1] != PROTO_VERSION_MINOR) {
		msg(_("Wrong protocol version `%c%c\': want `%c%c'"),
				c[0], c[1],
				PROTO_VERSION_MAJOR,
				PROTO_VERSION_MINOR);
		return -1;
	}

	/* 'block' */
	c[0] = fgetc(f); /* B */
	if (sig != 0) signal_abort(sig);
	c[1] = fgetc(f); /* L */
	if (sig != 0) signal_abort(sig);
	c[2] = fgetc(f); /* O */
	if (sig != 0) signal_abort(sig);
	c[3] = fgetc(f); /* C */
	if (sig != 0) signal_abort(sig);
	c[4] = fgetc(f); /* K */
	if (sig != 0) signal_abort(sig);

	if (c[0] != 'B' || c[1] != 'L' || c[2] != 'O' ||
			c[3] != 'C' || c[4] != 'K') {
		msg(_("BLOCK protocol seperator not found: `%c%c%c%c%c\'"),
				c[0], c[1], c[2], c[3], c[4]);
		return -1;
	}

	/* the bytes */
	c[0] = fgetc(f);
	if (sig != 0) signal_abort(sig);
	c[1] = fgetc(f);
	if (sig != 0) signal_abort(sig);
	c[2] = fgetc(f);
	if (sig != 0) signal_abort(sig);
	c[3] = fgetc(f);
	if (sig != 0) signal_abort(sig);
	c[4] = fgetc(f);
	if (sig != 0) signal_abort(sig);
	c[5] = fgetc(f); /* \n */
	if (sig != 0) signal_abort(sig);

	if (!isdigit(c[0]) || !isdigit(c[1]) || !isdigit(c[2]) ||
			!isdigit(c[3]) || !isdigit(c[4])) {
		msg(_("Illegal block size"));
		return -1;
	}
	
	bytes = atoi(g_strdup_printf("%c%c%c%c%c",
			c[0], c[1], c[2], c[3], c[4]));

	if (bytes > BUFSIZE) {		/* out of bounds...? */
		msg(_("Block size larger then BUFSIZE"));
		return -1;
	}
	if (opt_verbose > 2)
		msg(_("Block seen, start read of %d bytes"), bytes);
	
	return bytes;
}
