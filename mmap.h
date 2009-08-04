#ifndef MMAP_H
#define MMAP_H   1

#include <sys/types.h>

enum mmap_state {
	mmap_state_not_opened,
	mmap_state_error,
	mmap_state_open,
	mmap_state_closed
};

enum mmap_mode {
	mmap_mode_readonly,       // read-only access to the file and memory
	mmap_mode_write,          // read and write access
	mmap_mode_volatile_write  // read-only access to the file but
                                  // altering the data buffer is allowed
};

struct mmap_info {
	int fd;                // fd of the mmap'd file
	void *data;            // actual data
	off_t length;          // length of the data
	enum mmap_state state; // errno is maybe set additionally.
};

/**
 * mmaps the given file at pathname and returns a struct containing pointer
 * to the data. Parameter 'mode' sets the access to the file and memory area.
 * In case of error, struct member state is set to mmap_state_error
 * and errno is set.
 */
struct mmap_info mmap_file(const char *pathname, enum mmap_mode mode);

/**
 * Closes the given mmapped file. Error can be read from info->state and errno.
 */
void mmap_close(struct mmap_info *info);

#endif /* mmap.h */
