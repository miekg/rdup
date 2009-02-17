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
 * 01BLOCK8192
 */

#include "rdup.h"
#include "protocol.h"

/**
 * output a block header
 */
gint
block_out_header(FILE *f, size_t size, int fp) {
	char *p;
	p = g_strdup_printf("%c%c%s%05d\n", PROTO_VERSION_MAJOR, 
			PROTO_VERSION_MINOR, PROTO_BLOCK,(int)size);
	if (f) {
		if (fwrite(p, sizeof(char), strlen(p), f) != strlen(p))
			return -1;

	} else if (fp != -1) {
		if (write(fp, p, strlen(p)) == -1)
			return -1;
	}
	return 0;
}

/**
 * output a block
 */
gint
block_out(FILE *f, size_t size, char *buf, int fp) {
/*	if (fwrite(buf, sizeof(char), size, f) != size) { */
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
block_in(FILE *f, size_t size, char *buf) {
	if (fread(buf, sizeof(char), size, f) != size) {
		/* read less then expected */
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
block_in_header(FILE *f) {
	/* we are expecting an block header:
	 * 2 pos version; the word block; 5 digit number; newline
	 */
	char c[6];
	int bytes;

	/* I cannot think of anything smarter */
	/* version check */
	c[0] = fgetc(f);
	c[1] = fgetc(f);
	if (c[0] != PROTO_VERSION_MAJOR &&
			c[1] != PROTO_VERSION_MAJOR) {
		msg("Wrong protocol version");
		return -1;
	}

	/* 'block' */
	c[0] = fgetc(f); /* B */
	c[1] = fgetc(f); /* L */
	c[2] = fgetc(f); /* O */
	c[3] = fgetc(f); /* C */
	c[4] = fgetc(f); /* K */

	if (c[0] != 'B' || c[1] != 'L' || c[2] != 'O' ||
			c[3] != 'C' || c[4] != 'K') {
		msg("BLOCK protocol seperator not found");
		return -1;
	}

	/* the bytes */
	c[0] = fgetc(f); 
	c[1] = fgetc(f); 
	c[2] = fgetc(f); 
	c[3] = fgetc(f); 
	c[4] = fgetc(f); 
	c[5] = fgetc(f); /* \n */
	if (!isdigit(c[0]) || !isdigit(c[1]) || !isdigit(c[2]) ||
			!isdigit(c[3]) || !isdigit(c[4])) {
		msg("Illegal block size");
		return -1;
	}
	
	bytes = atoi(g_strdup_printf("%c%c%c%c%c",
			c[0], c[1], c[2], c[3], c[4]));

	if (bytes > BUFSIZE) {		/* out of bounds...? */
		msg("Block size larger then BUFSIZE");
		return -1;
	}
	/* file pointer should now be correctly positioned */
	return bytes;
}
