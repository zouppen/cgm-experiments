#ifndef CGM_ERROR_H
#define CGM_ERROR_H   1

/**
 * Some nasty macros and other things to help error reporting in this cgm
 * library.
 */

#define has_errno 1
#define no_errno 0

/**
 * A shorthand function for setting cgm errors on one line.
 * Sets global error code to CODE and errno status value to ERRNO
 * (0 or 1) and returns from caller function with RET value.
 */
#define return_with_error(RET, CODE, ERRNO) { cgm_error.code = (CODE); cgm_error.see_errno = (ERRNO); return (RET); }

/**
 * Cleans error structure by setting it to successful state and returns from
 * caller function with RET value.
 */
#define return_success(RET) { cgm_error.code = cgm_no_error; return (RET); }

// Not a thread-safe solution. But without this the code becomes a total mess.
struct cgm_error_struct {
	int line; // Zero if not applicable.
	int see_errno; // Errno contains something important.
	enum cgm_error_code {
		cgm_no_error, // The default.
		cgm_err_file_open,
		cgm_err_file_close,
		cgm_err_invalid_header,
		cgm_err_garbage,
		cgm_err_invalid_byte,
		cgm_error_code_count
	} code;
};

extern struct cgm_error_struct cgm_error;

void cgm_err(int retval, const char *file);

#endif /* cgm_error.h */
