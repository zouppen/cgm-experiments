CFLAGS=-Wall -Wextra -std=c99 -pedantic
LDFLAGS=`xml2-config --cflags --libs`

.PHONY: all clean

all: utf8_tester mmap_tester cgm2dom

utf8.o: utf8.c
	gcc $(CFLAGS) -c utf8.c

cgm_error.o: cgm_error.c
	gcc $(CFLAGS) -c cgm_error.c

utf8_tester: utf8.o utf8_test.c
	gcc $(CFLAGS) -o utf8_test utf8.o utf8_test.c

mmap.o: mmap.c
	gcc $(CFLAGS) -c mmap.c

mmap_tester: mmap.o mmap_test.c
	gcc $(CFLAGS) -o mmap_test mmap.o mmap_test.c

cgm2dom: utf8.o mmap.o cgm_error.o cgm2dom.c
	gcc $(CFLAGS) $(LDFLAGS) -o cgm2dom utf8.o mmap.o cgm_error.o cgm2dom.c

clean:
	@rm -f utf8.o mmap.o cgm_error.o
	@rm -f utf8_test cgm2dom mmap_test

