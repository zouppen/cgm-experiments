#include <stdio.h>
#include <err.h>
#include "utf8.h"

const int true = 1;

int main(int argc, char **argv)
{
  if (argc != 2) errx(1, "Usage: %s UTF-8_TEXT_FILE", argv[0]);

  FILE *file = fopen(argv[1], "rb");
  if ( file == NULL) err(1,"Tiedostoa ei saanut avattua");
  
  unsigned char buf[UTF8_MAX_BYTES];

  while (true) {
    int n = utf8_fgetc(file, buf);
    
    if (n == UTF8_ERR_NO_DATA && feof(file) ) break; // normal EOF
    if (n < 0) errx(2,"Vika tiedostossa.");

    fwrite(buf, 1, n, stdout);
    putchar(' ');
  }

  return 0;

}
