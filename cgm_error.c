/**
 * Some nasty macros and other things to help error reporting in this cgm
 * library.
 */

#include <err.h>
#include "cgm_error.h"

struct cgm_error_struct cgm_error;

/**
 * Displays error with CGM parsing in a user friendly form. Exits the program
 * with retval and puts file name 'file' to the error message.
 */
void cgm_err(int retval, const char *file)
{
	static const char *msgs[] = {
		/* cgm_err_no_error */ "No error",
		/* cgm_err_file_open */ "Cannot open file for reading",
		/* cgm_err_file_close */ "Cannot close the file",
		/* cgm_err_invalid_header */ "Invalid header. Not a CGM file?",
		/* cgm_err_garbage */ "Garbage on line",
		/* cgm_err_invalid_byte */ "Invalid encoding in file",
		/* cgm_err_indentation */ "Obscure indentation"
	};
	
	if ( cgm_error.see_errno)
		err(retval, "At file %s:%d: %s", file, cgm_error.line,
		    msgs[cgm_error.code]);
	else
		errx(retval, "At file %s:%d: %s", file, cgm_error.line,
		    msgs[cgm_error.code]);
}
