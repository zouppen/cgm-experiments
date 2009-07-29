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

#include <stdio.h>
#include <string.h>
#include "utf8_getc.h"

/**
 * Reads UTF-8 character from a stream and writes it to buf, padding it to
 * UTF8_MAX_BYTES by writing zeroes to the end of buf. Returns the number of
 * bytes read.
 */
int utf8_fgetc(FILE *stream, char *buf)
{
  int got;

  // Reading the first byte of a character
  got = fread(buf, 1, 1, stream);
  if (got != 1) return UTF8_FGETC_NO_DATA;
  
  // Examining the first byte to determine character length
  int bytes = char_length(buf[0]);
  if (bytes == -1) return UTF8_FGETC_INVALID_BYTE;

  // Reading other bytes of a multibyte char
  got = fread(buf+1, 1, bytes-1, stream);
  if (got != bytes-1) return UTF8_FGETC_TRUNCATED_BYTE;

  // Padding the char with zero bytes
  memset(buf+bytes, 0, UTF8_MAX_BYTES-bytes); 
  return bytes; // number of bytes read
}

/**
 * Returns an UTF-8 char length in bytes by analyzing the first byte.
 */
inline int char_length(char byte) {
  if ( (byte & 0x80) == 0x00 ) return 1; // 0xxxxxxx (plain ASCII)
  if ( (byte & 0xe0) == 0xc0 ) return 2; // 110xxxxx
  if ( (byte & 0xf0) == 0xe0 ) return 3; // 1110xxxx
  if ( (byte & 0xf8) == 0xf0 ) return 4; // 1110xxxx

  return -1; // invalid byte
}
