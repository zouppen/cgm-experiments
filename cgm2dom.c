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

struct cgm_special {
	int element_start;
	int element_end;
	int escape;
	int inline_separator;
	int newline;
	int tab;
	int space;
};

int main(int argc, char **argv)
{
	struct cgm_special special;
	// It's safe to put ASCII literals here, values map to unicodes
	special.newline = '\n'; 
	special.tab = '\t';
	special.space = ' ';

	xmlDocPtr doc = NULL;            // document pointer
	struct level levels[MAX_LEVELS];
	int cur_level_i = 0;
	struct level *cur_level = levels; // pointer to the first element 

	if (argc < 2 || argc > 3) 
		errx(1, "Usage: %s CGM_FILE [OUTPUT_FILE]", argv[0]);

	// Opening CGM file to memory
	struct mmap_info info =	mmap_fopen(argv[1], mmap_mode_volatile_write);
	if (info.state == mmap_state_error)
		err(1,"Can not open a file %s for reading", argv[1]);
     
	unsigned char *cgm_p = info.data;
	unsigned char *cgm_end = cgm_p + info.length;

	// Reading the CGM header.
	special.element_start = utf8_to_unicode(&cgm_p, cgm_end);
	
	if (!( utf8_to_unicode(&cgm_p, cgm_end) == 0x63 && // c
	       utf8_to_unicode(&cgm_p, cgm_end) == 0x67 && // g
	       utf8_to_unicode(&cgm_p, cgm_end) == 0x6d && // m
	       utf8_to_unicode(&cgm_p, cgm_end) == 0x20 )) // space
	{
		errx(2,"File does not contain a valid CGM header");
	}

	special.escape = utf8_to_unicode(&cgm_p, cgm_end);
	special.inline_separator = utf8_to_unicode(&cgm_p, cgm_end);
	special.element_end = utf8_to_unicode(&cgm_p, cgm_end);

	if (utf8_to_unicode(&cgm_p, cgm_end) != special.newline)
		errx(2,"Extra characters inside header line");

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
		int code = utf8_to_unicode(&cgm_p, cgm_end);

		if (code == UTF8_ERR_NO_DATA) { // End of file
			break;
		} else if (code < 0) {
			printf("Tragic scandal: %d\n", code);
			break;
		} else if (code == special.element_start) {
			printf("start\n");
		} else if (code == special.element_end) {
			printf("end\n");
		} else if (code == special.escape) {
			printf("escape\n");
		} else if (code == special.inline_separator) {
			printf("inline\n");
		} else if (code == special.newline) {
			printf("newline\n");
		} else if (code == special.tab) {
			printf("tab\n");
		} else if (code == special.space) {
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

	mmap_close(&info);
	if (info.state == mmap_state_error)
		err(1,"Can not close the file %s",  argv[1]);

	return(0);
}
#else
	int main(void) {
		errx(1, "Tree support not compiled in");
	}
#endif
