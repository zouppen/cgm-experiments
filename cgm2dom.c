/** 
 * Experiments for CGM DOM tree generation.
 * Based on the nice example DOM creation by
 * Lucas Brasilino <brasilino@recife.pe.gov.br>
 *
 * To compile this file using gcc you can type
 * gcc `xml2-config --cflags --libs` -o cgm2dom cgm2dom.c utf8_getc.c
 * 
 * @author Joel Lehtonen
 */

#include <stdio.h>
#include <err.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "utf8.h"
#include "mmap.h"

#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

const int MAX_LEVELS=10; // hard-wired indent levels... blame me.
const int true = 1;

struct level {
	int indentation;   // indentation of that level
	xmlNodePtr parent; // parent node of this level
};

struct cgm_unicode {
	int element_start;
	int element_end;
	int escape;
	int inline_separator;
	int newline;
	int tab;
	int space;
};

struct cgm_info {
	struct cgm_unicode unicode;
	unsigned char *p; // OK to alter.
	unsigned char *endptr; // End of the buffer. Do not alter.
	unsigned char *lineptr; // Helps printing line on error
	int line; // Line number for error reporting purposes
};

struct {
	int line; // Zero if not applicable.
	int see_errno; // Errno contains something important.
	enum {
		cgm_no_error, // The default.
		cgm_cannot_open, 
		cgm_invalid_header,
		cgm_garbage,
		cgm_invalid_byte,
		cgm_error_code_count
	} code;
} cgm_error;

// Prototypes are here temporarily
xmlDocPtr cgm_parse_file(char *filename);
void cgm_read_header(struct cgm_info *cgm);
void cgm_err(int retval, const char *file);

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3) 
		errx(1, "Usage: %s CGM_FILE [OUTPUT_FILE]", argv[0]);

	xmlDocPtr doc = cgm_parse_file(argv[1]);
	if (cgm_error.code) cgm_err(1, argv[0]);
	printf(":-)\n");

	/* 
	 * Dumping document to stdio or file
	 */
	xmlSaveFormatFileEnc(argc > 2 ? argv[2] : "-", doc, "UTF-8", 1);

	/*free the document */
	xmlFreeDoc(doc);

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();

}

xmlDocPtr cgm_parse_file(char *filename) {
	struct cgm_info cgm;

	xmlDocPtr doc = NULL;            // document pointer
	struct level levels[MAX_LEVELS];
	int cur_level_i = 0;
	struct level *cur_level = levels; // pointer to the first element 

	// Opening CGM file to memory
	struct mmap_info mmap_info = mmap_fopen(filename,
						mmap_mode_volatile_write);
	if (mmap_info.state == mmap_state_error) {
		cgm_error.code = cgm_cannot_open;
		cgm_error.see_errno = 1;
		return doc;
	}

	// Filling info from mmap struct to cgm parser struct
	cgm.p = mmap_info.data;
	cgm.endptr = cgm.p + mmap_info.length;

	// Some extra info for nicer errors
	cgm.lineptr = cgm.p;
	cgm.line = 1;

	// Filling trivial data to the unicode values
	// It's safe to put ASCII literals here, values map to unicodes
	cgm.unicode.newline = '\n'; 
	cgm.unicode.tab     = '\t';
	cgm.unicode.space   = ' ';

	// Parsing header
	cgm_read_header(&cgm);
	if (cgm_error.code) return doc; // ERROR

	// DOM startup
	
	LIBXML_TEST_VERSION;
	doc = xmlNewDoc(BAD_CAST "1.0"); // XML 1.0
	
	// Seems to be correct way to set root namespace. I found it by
	// trial and error. Libxml2 folks have skipped documentation.
	
	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "cgm");
	xmlNsPtr ns_CGM = xmlNewNs(root, 
				   BAD_CAST "http://codegrove.org/2009/cgm",
				   NULL);
	xmlDocSetRootElement(doc, root);
/*	
	// Set level indicators	
	cur_level->indentation = 0;
	cur_level->parent = root;
	
*/

	// Starting the parser.

	while (1) {
		int code = utf8_to_unicode(&cgm.p, cgm.endptr);

		if (code == UTF8_ERR_NO_DATA) { // End of file
			break;
		} else if (code < 0) {
			cgm_error.line = cgm.line;
			cgm_error.code = cgm_invalid_byte;
			cgm_error.see_errno = 0;
			return doc;
		} else if (code == cgm.unicode.element_start) {
			printf("start\n");
		} else if (code == cgm.unicode.element_end) {
			printf("end\n");
		} else if (code == cgm.unicode.escape) {
			printf("escape\n");
		} else if (code == cgm.unicode.inline_separator) {
			printf("inline\n");
		} else if (code == cgm.unicode.newline) {
			cgm.line++;
			printf("newline\n");
		} else if (code == cgm.unicode.tab) {
			printf("tab\n");
		} else if (code == cgm.unicode.space) {
			printf("space\n");
		} else {
			printf("U+%x\n", code);
		}
	}
		
/*
		} else {
			//FIXME counter
			xmlNodePtr newtext = xmlNewTextLen(text.data, text.len);
			xmlNodePtr new_el = xmlNewChild(root, NULL,
							BAD_CAST "line", NULL);
			xmlAddChild(new_el, newtext);			

			// Text node is done, rolling back
			buf=text.data;
			text.len = 0;
		}
	}

	printf("\n");

*/	/*
	xmlNewChild(root_node, NULL, BAD_CAST "node1",
		    BAD_CAST "content of node 1");

	node =
		xmlNewChild(root_node, NULL, BAD_CAST "node3",
			    BAD_CAST "this node has attributes");
	xmlNewProp(node, BAD_CAST "attribute", BAD_CAST "yes");
	xmlNewProp(node, BAD_CAST "foo", BAD_CAST "bar");

	node = xmlNewNode(NULL, BAD_CAST "node4");
	node1 = xmlNewText(BAD_CAST

			   */

	mmap_close(&mmap_info);
	if (mmap_info.state == mmap_state_error)
		err(1,"Can not close the file %s", filename);
	// FIXME this error as cgm_error

	return(0);
}

void cgm_read_header(struct cgm_info *cgm)
{
	cgm_error.line = 1;
	cgm_error.see_errno = 0;
	
	cgm->unicode.element_start = utf8_to_unicode(&cgm->p, cgm->endptr);

	if (!( utf8_to_unicode(&cgm->p, cgm->endptr) == 'c' &&
	       utf8_to_unicode(&cgm->p, cgm->endptr) == 'g' &&
	       utf8_to_unicode(&cgm->p, cgm->endptr) == 'm' ))
	{
		cgm_error.code = cgm_invalid_header;
		return;
	}

	cgm->unicode.inline_separator = utf8_to_unicode(&cgm->p, cgm->endptr);
	cgm->unicode.escape = utf8_to_unicode(&cgm->p, cgm->endptr);
	cgm->unicode.element_end = utf8_to_unicode(&cgm->p, cgm->endptr);

	if (utf8_to_unicode(&cgm->p, cgm->endptr) != cgm->unicode.newline) {
		cgm_error.code = cgm_garbage;
		return;
	}

	cgm_error.code = cgm_no_error;
	return;
}

void cgm_err(int retval, const char *file)
{
	static const char *msgs[] = {
		/* cgm_no_error */ "No error",
		/* cgm_cannot_open */ "Cannot open file for reading",
		/* cgm_invalid_header */ "Invalid header",
		/* cgm_garbage */ "Garbage on line",
		/* cgm_invalid_byte */ "Invalid encoding in file"
	};
	
	if ( cgm_error.see_errno)
		err(retval, "At file %s:%d: %s", file, cgm_error.line,
		    msgs[cgm_error.code]);
	else
		errx(retval, "At file %s:%d: %s", file, cgm_error.line,
		    msgs[cgm_error.code]);
}
#else
int main(void) {
	errx(1, "Please reinstall or recompile libxml2 "
	     "with tree support");
}
#endif
