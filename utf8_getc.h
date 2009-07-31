/* UTF-8 getc and similar functions */

#ifndef UTF8_H
#define UTF8_H

#define UTF8_MAX_BYTES 4
#define UTF8_ERR_NO_DATA (-1)
#define UTF8_ERR_INVALID_BYTE (-2)
#define UTF8_ERR_TRUNCATED_BYTE (-3)

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
 * Returns an UTF-8 char length in bytes by analyzing the first byte.
 */
int utf8_chrlen(unsigned char byte);

int utf8_compare_char(unsigned char *buf, utf8_string *str);

#endif //UTF8_H
