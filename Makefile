CFLAGS=-Wall -Wextra -std=c99 -pedantic
LDFLAGS=`xml2-config --cflags --libs`

.PHONY: all clean

all: tester cgm2dom

utf8.o: utf8.c
	gcc $(CFLAGS) -c utf8.c

tester: utf8.o utf8_test.c
	gcc $(CFLAGS) -o utf8_test utf8.o utf8_test.c

cgm2dom: utf8.o cgm2dom.c
	gcc $(CFLAGS) $(LDFLAGS) -o cgm2dom utf8.o cgm2dom.c

clean:
	@rm -f utf8.o
	@rm -f utf8_test
	@rm -f cgm2dom
