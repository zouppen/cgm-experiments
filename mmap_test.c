#include <stdio.h>
#include <err.h>
#include "mmap.h"

int main(int argc, char **argv)
{
	if (argc != 2) errx(1, "Usage: %s FILE_TO_READ", argv[0]);

	struct mmap_info info =	mmap_file(argv[1], mmap_mode_volatile_write);

	if (info.state == mmap_state_error) err(1,"Can not open a file %s "
						"for reading and mmapping",
						argv[1]);
	char *data = info.data;

	int width = 0;
	for (width = 0; width < info.length; width++) {
		if ( data[width] == '\n') {
			break;
		}
	}

	// The file stays untouched even if changing contents
	data[width] = '\0'; 		

	printf("File length: %d\nFirst line: %s\n", (int)info.length, data);

	void mmap_close(struct mmap_info *info);
	if (info.state == mmap_state_error) err(1,"Can not close the file %s",
						argv[1]);

	return 0;
}
