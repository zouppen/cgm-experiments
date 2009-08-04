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
 * Codegrove's mmapping library. 
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mmap.h"

/**
 * mmaps the given file at pathname and returns a struct containing pointer
 * to the data. Parameter 'mode' sets the access to the file and memory area.
 * In case of error, struct member state is set to mmap_state_error
 * and errno is set.
 */
struct mmap_info mmap_fopen(const char *pathname, enum mmap_mode mode)
{
	int ret;
	int open_flags, mmap_prot, mmap_flags; // Flags depending on mode.
	struct mmap_info info;
	info.state = mmap_state_error; // Reset if everything is ok.
	
	// Determining options for open(2) and mmap(2).
	switch (mode) {
	case (mmap_mode_readonly):
		open_flags = O_RDONLY;
		mmap_prot = PROT_READ;
		mmap_flags = MAP_PRIVATE;
		break;
	case(mmap_mode_write):
		open_flags = O_RDWR;
		mmap_prot = PROT_READ | PROT_WRITE;
		mmap_flags = MAP_SHARED;
		break;
	case (mmap_mode_volatile_write):
		open_flags = O_RDONLY;
		mmap_prot = PROT_READ | PROT_WRITE;
		mmap_flags = MAP_PRIVATE;
		break;
	}

	// Opening a file as we do normally.
	info.fd = open(pathname, open_flags);
	if (info.fd == -1) return info;
	
	// Determining the length of this file.
	struct stat stats;
	ret = fstat(info.fd, &stats);
	if (ret == -1) return info;
	info.length = stats.st_size;

	// Doing some black bit magic with mmap.
	info.data = mmap(NULL, info.length, mmap_prot, mmap_flags, info.fd, 0);
	if (info.data == MAP_FAILED) return info;
	
	info.state = mmap_state_open;
	return info;
}

/**
 * Closes the given mmapped file. Error can be read from info->state and errno.
 */
void mmap_close(struct mmap_info *info)
{
	int ret;

	// Setting state to success at first.
	info->state = mmap_state_closed;
	
	// Un-mmap.
	ret = munmap(info->data, info->length);
	if (ret == -1) info->state = mmap_state_error;
	// In case of fail, continue anyway try to close the file.
	// In that case close errno will overwrite this errno.

	// Close the file.
	ret = close(info->fd);
	if (ret == -1) info->state = mmap_state_error;
}
