/* UTF-8 getc and similar functions */

#ifndef UTF8_H
#define UTF8_H

#define UTF8_MAX_BYTES 4
#define UTF8_ERR_NO_DATA (-1)
#define UTF8_ERR_INVALID_BYTE (-2)
#define UTF8_ERR_TRUNCATED_BYTE (-3)

#include <stdio.h>

/* yet another UTF-8 string */
typedef struct {
	unsigned char *data;
	int len;
} utf8_string;

/**
 * Reads UTF-8 character from a stream and writes it to buf. Does not pad
 * buf with anything. Returns the number of bytes read.
 */
int utf8_fgetc(FILE *stream, unsigned char *buf);

/**
 * Reads n UTF-8 characters from a stream and writes it to buf. Does not pad
 * buf with anything. Returns the number of bytes read.
 */
int utf8_fgets(FILE *stream, unsigned char *buf, int n);

/**
 * Returns an UTF-8 char length in bytes by analyzing the first byte.
 */
int utf8_chrlen(unsigned char byte);

/**
 * Checks if UTF-8 buffer starts with a given UTF-8 string.
 * Buffer size in bytes must be at least as long as given UTF-8 string.
 * This just makes a raw comparison and distinguishes similar characters with
 * diffent raw codes.
 */
int utf8_starts_with(unsigned char *buf, utf8_string *str);

#endif //UTF8_H
