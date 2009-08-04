/**
 * @file
 * @author Joel Lehtonen <joel.lehtonen ät jyu.fi>
 * @version 0.1
 *
 * @section LICENSE
 *
 * GNU GPL version 3 or (at your option) later.
 *
 * @section DESCRIPTION
 *
 * UTF-8 compatible fgetc.
 *
 * The character extractor is somewhat relaxed and it only checks errors which
 * prevent character extraction from a stream. It doesn't decode Unicode values.
 */

#include <string.h>
#include <stdio.h>
#include "utf8.h"

/**
 * Reads UTF-8 character from a stream and writes it to buf. Does not pad
 * buf with anything. Returns the number of bytes read.
 */
int utf8_fgetc(FILE *stream, unsigned char *buf)
{
	int got;

	// Reading the first byte of a character.
	got = fread(buf, 1, 1, stream);
	if (got != 1) return UTF8_ERR_NO_DATA;
  
	// Examining the first byte to determine character length.
	int bytes = utf8_chrlen(buf[0]);
	if (bytes == UTF8_ERR_INVALID_BYTE) return UTF8_ERR_INVALID_BYTE;

	// Reading other bytes of a multibyte char.
	got = fread(buf+1, 1, bytes-1, stream);
	if (got != bytes-1) return UTF8_ERR_TRUNCATED_BYTE;

	return bytes; // Number of bytes read.
}

/**
 * Reads n UTF-8 characters from a stream and writes it to buf. Does not pad
 * buf with anything. Returns the number of bytes read.
 */
int utf8_fgets(FILE *stream, unsigned char *buf, int n)
{
	int i, bytes, total_bytes=0;
	for( i=0; i<n; i++) {
		bytes = utf8_fgetc(stream, buf);
		if (bytes < 0) return bytes; // error

		buf += bytes; // moving the pointer
		total_bytes += bytes;
	}
	return total_bytes;
}

/**
 * Returns an UTF-8 char length in bytes by analyzing the first byte.
 */
inline int utf8_chrlen(unsigned char byte) {
	if ( (byte & 0x80) == 0x00 ) return 1; // 0xxxxxxx (plain ASCII)
	if ( (byte & 0xe0) == 0xc0 ) return 2; // 110xxxxx
	if ( (byte & 0xf0) == 0xe0 ) return 3; // 1110xxxx
	if ( (byte & 0xf8) == 0xf0 ) return 4; // 1110xxxx

	return UTF8_ERR_INVALID_BYTE;
}

/**
 * Checks if UTF-8 buffer starts with a given UTF-8 string.
 * Buffer size in bytes must be at least as long as given UTF-8 string.
 * This just makes a raw comparison and distinguishes similar characters with
 * diffent raw codes.
 */
int utf8_starts_with(unsigned char *buf, utf8_string *str)
{
	return memcmp(buf, str->data, str->bytes);
}

/**
 * Gets utf8 string from given buf. The string contains at most max_bytes and
 * n characters. The string may be shorter if there was not enough characters
 * between buf and buf+max_bytes.
 */
utf8_string utf8_as_string(unsigned char *buf, int n, int max_bytes)
{
	utf8_string str;
	str.data = buf; // starting address never changes
	str.bytes = 0;
	str.length = 0;

	while (1) {
		// Stop if enough characters has been read.
		if (str.length == n) break;

		// Stop if inter-character point is at max_bytes. Count all in.
		if (str.bytes == max_bytes) break;

		int bytes = utf8_chrlen(str.data[str.bytes]);

		// Stop if invalid byte
		if (bytes == UTF8_ERR_INVALID_BYTE) {
			str.length = -1;
			break;
		}

		// Stop if inter-character point is beyond the buffer.
		if (str.bytes+bytes > max_bytes) break;
		
		str.bytes += bytes;
		str.length++;
	}
	
	return str;
}

/**
 * Reads a C string literal to an utf8_string. Please note that this is
 * FIXME and is not indented to everyday use (has a fancy 9999 limit and
 * dirty casting). Rewrite if you mind.
 */
utf8_string utf8_literal_to_string(const char* literal)
{
	return utf8_as_string((unsigned char *)literal, 9999, strlen(literal));
}
