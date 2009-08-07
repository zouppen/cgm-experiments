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
#include "cgm_error.h"

#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

const int MAX_LEVELS=10; // hard-wired indent levels... blame me.
const int true = 1;

struct level {
	int indent;        // indentation of that level
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
	int preformatted;
};

struct cgm_info {
	struct cgm_unicode unicode;
	unsigned char *p; // OK to alter.
	unsigned char *endptr; // End of the buffer. Do not alter.
	unsigned char *lineptr; // Helps printing line on error
	int line; // Line number for error reporting purposes
};

const int tab_width = 8; // May be nice if configurable
const int cgm_empty_line = -1;

// Prototypes are here temporarily
xmlDocPtr cgm_parse_file(char *filename);
int cgm_read_header(struct cgm_info *cgm);
int cgm_read_indent(struct cgm_info *cgm);
int cgm_read_text(struct cgm_info *cgm);
int cgm_dummy_dumper(struct cgm_info *cgm);

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3) 
		errx(1, "Usage: %s CGM_FILE [OUTPUT_FILE]", argv[0]);

	xmlDocPtr doc = cgm_parse_file(argv[1]);
	if (cgm_error.code) cgm_err(1, argv[1]);
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
	if (mmap_info.state == mmap_state_error)
		return_with_error(doc, cgm_err_file_open, 1);

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
	xmlNewProp(root, BAD_CAST "original", BAD_CAST filename);

	// Set indentation level 
	cur_level->indent = 0;
	cur_level->parent = root;

	// Line parser.

	xmlNodePtr last_node = NULL;

	while (1) {

		// Determining line indent
		int indent = cgm_read_indent(&cgm);
		if (cgm_error.code) return doc; // error occurred

		// Indentation
		if (indent == cgm_empty_line) {
			printf("empty\n");
			continue;
		}

		if ( indent == cur_level->indent ) {
			// Just like previous line
		} else if ( indent > cur_level->indent ) {
			// Indent has increased.
			// FIXME check if we need to allocate more memory
			cur_level++;
			cur_level->indent = indent;
			cur_level->parent = last_node;
		} else {
			// Indent has decreased
			// Search for matching indentation level
			while (indent < cur_level->indent) {
				cur_level--;
			}
			
			// If it's not matching then we have a syntax error
			if ( indent != cur_level->indent )
				return_with_error(doc, cgm_err_indentation, 1);
		}
		
		printf("Indent: %d\n",indent); // debug

		// Put the element into the DOM tree

		unsigned char *text_p = cgm.p;
		int text_length = cgm_read_text(&cgm);
		if (cgm_error.code) return doc; // error occurred
		
		printf("Bytes in that line: %d\n",text_length);

		// Add new element to the tree.
		xmlNodePtr new_text = xmlNewTextLen(text_p, text_length);
		xmlNodePtr new_el = xmlNewChild(cur_level->parent, NULL,
						BAD_CAST "line", NULL);
		xmlAddChild(new_el, new_text);
		
		last_node = new_el;

		// Take the newline out.
		utf8_to_unicode(&cgm.p, cgm.endptr); // FIXME doesn't check...
		
		if (cgm.p >= cgm.endptr) break; // EOF
	}
	
	/*
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
		return_with_error(doc, cgm_err_file_close, has_errno);

	return_success(doc);
}

/**
 * Reads CGM header and fills the given cgm struct with all the important stuff.
 * Always returns 0. Errors are passed with return_with_error().
 */
int cgm_read_header(struct cgm_info *cgm)
{
	cgm_error.line = 1;
	
	cgm->unicode.element_start = utf8_to_unicode(&cgm->p, cgm->endptr);

	if (!( utf8_to_unicode(&cgm->p, cgm->endptr) == 'c' &&
	       utf8_to_unicode(&cgm->p, cgm->endptr) == 'g' &&
	       utf8_to_unicode(&cgm->p, cgm->endptr) == 'm' &&
	       utf8_to_unicode(&cgm->p, cgm->endptr) == '1' ))
	{
		return_with_error(0, cgm_err_invalid_header, no_errno);
	}

	cgm->unicode.inline_separator = utf8_to_unicode(&cgm->p, cgm->endptr);
	cgm->unicode.escape           = utf8_to_unicode(&cgm->p, cgm->endptr);
	cgm->unicode.preformatted     = utf8_to_unicode(&cgm->p, cgm->endptr);
	cgm->unicode.element_end      = utf8_to_unicode(&cgm->p, cgm->endptr);

	if (utf8_to_unicode(&cgm->p, cgm->endptr) != cgm->unicode.newline)
		return_with_error(0, cgm_err_garbage, no_errno);

	return_success(0);
}

/**
 * Count indentation level. If the line has no content, this function returns
 * -1 and cgm->p is at the beginning of the following line.
 */
int cgm_read_indent(struct cgm_info *cgm)
{
	unsigned char *prev_p;
	int indent = 0;

	while (1) {
		prev_p = cgm->p;
		int code = utf8_to_unicode(&cgm->p, cgm->endptr);
		
		if (code == UTF8_ERR_NO_DATA ||
		    code == cgm->unicode.newline ) {
			// Line has no content
			return_success(cgm_empty_line);
		} else if (code < 0) {
			// Unexcepted error.
			return_with_error(0, cgm_err_invalid_byte, no_errno);
		} else if (code == cgm->unicode.space) {
			indent++;
		} else if (code == cgm->unicode.tab) {
			// Rounding towards next tab (allows spaces to be mixed)
			indent += tab_width - (indent % tab_width);
		} else {
			// Content starts. "Unget" last character
			cgm->p = prev_p;
			return_success(indent);
		}
	}
}

/**
 * Dumps a line as tokens and Unicode values to standard output.
 * Used for debugging purposes.
 */
int cgm_dummy_dumper(struct cgm_info *cgm)
{
	while (1) {

		int code = utf8_to_unicode(&cgm->p, cgm->endptr);
				
		if (code == UTF8_ERR_NO_DATA) { // End of file
			return_success(0);
		} else if (code < 0) {
			// Unexcepted error.
			return_with_error(0, cgm_err_invalid_byte, no_errno);
		} else if (code == cgm->unicode.element_start) {
			printf("start\n");
		} else if (code == cgm->unicode.element_end) {
			printf("end\n");
		} else if (code == cgm->unicode.escape) {
			printf("escape\n");
		} else if (code == cgm->unicode.inline_separator) {
			printf("inline\n");
		} else if (code == cgm->unicode.newline) {
			printf("newline\n");
			return_success(0);
		} else if (code == cgm->unicode.tab) {
			printf("tab\n");
		} else if (code == cgm->unicode.space) {
			printf("space\n");
		} else {
			printf("U+%x\n", code);
		}
	}
}

/**
 * This function reads content until next character is non-text like element
 * boundary, escape character or newline. This function returns text block
 * length IN BYTES. At the end of this call cgm->p points to the start of the
 * next non-text character.
 * FIXME. Now it just takes everything it gets except newlines.
 */
int cgm_read_text(struct cgm_info *cgm)
{
	const unsigned char *start = cgm->p;
	unsigned char *p = cgm->p; // Current position in file.

	while (1) {
		int code = utf8_to_unicode(&p, cgm->endptr);
		
		if (code == UTF8_ERR_NO_DATA ||
		    code == cgm->unicode.newline ) {
			// End has came
			int length = cgm->p - start;
			return_success(length);
		} else if (code < 0) {
			// Unexcepted error.
			return_with_error(0, cgm_err_invalid_byte, no_errno);
		}
		cgm->p = p; // Keeping cgm->p always one char before p.
	}
}


#else
int main(void) {
	errx(1, "Please reinstall or recompile libxml2 "
	     "with tree support");
}
#endif
