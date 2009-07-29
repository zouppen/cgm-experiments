/* UTF-8 getc and similar functions */

#ifndef UTF8_FGETC_H
#define UTF8_FGETC_H

#define UTF8_MAX_BYTES 4
#define UTF8_FGETC_NO_DATA (-1)
#define UTF8_FGETC_INVALID_BYTE (-2)
#define UTF8_FGETC_TRUNCATED_BYTE (-3)

/**
 * Reads UTF-8 character from a stream and writes it to buf, padding it to
 * UTF8_MAX_BYTES by writing zeroes to the end of buf. Returns the number of
 * bytes read.
 */
int utf8_fgetc(FILE *stream, char *buf);

/**
 * Returns an UTF-8 char length in bytes by analyzing the first byte.
 */
inline int char_length(char byte);

#endif //UTF8_FGETC_H
