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

enum cgm_special{cgm_element_start,
		 cgm_element_end,
		 cgm_escape,
		 cgm_inline_separator,
		 cgm_special_count};

int main(int argc, char **argv)
{
	int specials[cgm_special_count];

	xmlDocPtr doc = NULL;            // document pointer
	struct level levels[MAX_LEVELS];
	int cur_level_i = 0;
	struct level *cur_level = levels; // pointer to the first element 
	int newline = 0x0a;

	if (argc < 2 || argc > 3) 
		errx(1, "Usage: %s CGM_FILE [OUTPUT_FILE]", argv[0]);

	// Opening CGM file to memory
	struct mmap_info info =	mmap_fopen(argv[1], mmap_mode_volatile_write);
	if (info.state == mmap_state_error)
		err(1,"Can not open a file %s for reading", argv[1]);
     
	unsigned char *cgm_p = info.data;
	unsigned char *cgm_end = cgm_p + info.length;

	// Reading the CGM header.
	specials[cgm_element_start] = utf8_to_unicode(&cgm_p, cgm_end);
	if (!( utf8_to_unicode(&cgm_p, cgm_end) == 0x63 && // c
	       utf8_to_unicode(&cgm_p, cgm_end) == 0x67 && // g
	       utf8_to_unicode(&cgm_p, cgm_end) == 0x6d && // m
	       utf8_to_unicode(&cgm_p, cgm_end) == 0x20 )) // space
	{
		errx(2,"File does not contain a valid CGM header");
	}

	specials[cgm_escape] = utf8_to_unicode(&cgm_p, cgm_end);
	specials[cgm_inline_separator] = utf8_to_unicode(&cgm_p, cgm_end);
	specials[cgm_element_end] = utf8_to_unicode(&cgm_p, cgm_end);

	if (utf8_to_unicode(&cgm_p, cgm_end) != newline)
		errx(2,"Extra characters inside header line");

	// Starting the parser.

	while (1) {
		int code = utf8_to_unicode(&cgm_p, cgm_end);
		if (code < 0) {
			printf("Loppu tuli, vikakoodi %d", code);
			break;
		}
		printf("U+%x\n", code);
	}

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
	
	unsigned char helper[2048]; // FIXME: hard coded
	utf8_string text;
	text.data = helper;
	text.len = 0;
	unsigned char *buf = text.data; // moving pointer	

	while (true) {
		int n = utf8_fgetc(file, buf);
		
		if (n == UTF8_ERR_NO_DATA && feof(file) ) break; // normal EOF
		if (n < 0) errx(2,"Vika tiedostossa.");
		
		if (utf8_starts_with(buf, &newline)) {
			fwrite(buf, 1, n, stdout);
			printf("%d ",n);

			// other char
			// FIXME check
			buf += n; // Char ok for text buffer, taking that one.
			text.len += n;
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

	enum cgm_special joopajoo;
	printf("sizeof: %d %d\n", cgm_element_end, cgm_special_count);
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
