CFLAGS=-Wall -Wextra -Werror -ansi -pedantic

.PHONYT=all clean

all: tester

utf8_fgetc.o: utf8_fgetc.c
	gcc ${CFLAGS} -c utf8_fgetc.c

tester: utf8_fgetc.o utf8_fgetc_test.c
	gcc ${CFLAGS} -o utf8_fgetc_test utf8_fgetc.o utf8_fgetc_test.c

clean:
	@rm -f utf8_fgetc.o
	@rm -f utf8_fgetc_test
