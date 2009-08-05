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
	int length;
	int bytes;
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

/**
 * Gets utf8 string from given buf. The string contains at most max_bytes and
 * n characters. The string may be shorter if there was not enough characters
 * between buf and buf+max_bytes.
 */
utf8_string utf8_as_string(unsigned char *buf, int n, int max_bytes);

/**
 * Reads a C string literal to an utf8_string. Please note that this is
 * FIXME and is not indented to everyday use (has a fancy 9999 limit and
 * dirty casting). Rewrite if you mind.
 */
utf8_string utf8_literal_to_string(const char* literal);

/**
 * Converts next UTF-8 character from the byte buffer to Unicode number.
 * After decoding, *buf is updated to the end of the character, ready to
 * the next call of this function. Endptr is a pointer to the next byte after
 * the last character in the buffer (buf_start_pointer+buf_length). If an
 * error occurs, UTF8_ERR_* is returned and *buf is at the next character
 * after the errorneous byte. 
 */
int utf8_to_unicode(unsigned char **buf, unsigned char *endptr);


#endif //UTF8_H
