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
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "utf8_getc.h"

#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

const int MAX_LEVELS=10; // hard-wired indent levels... blame me.
const int true = 1;

struct level {
	int indentation;   // indentation of that level
	xmlNodePtr parent; // parent node of this level
};

int main(int argc, char **argv)
{
	xmlDocPtr doc = NULL;            // document pointer
	struct level levels[MAX_LEVELS];
	int cur_level_i = 0;
	struct level *cur_level = levels; // pointer to the first element 
	utf8_string newline;
	newline.data = "\n";
	newline.len = 1;

	if (argc < 2 || argc > 3) 
		errx(1, "Usage: %s CGM_FILE [OUTPUT_FILE]", argv[0]);
     
	FILE *file = fopen(argv[1], "rb");
	if ( file == NULL) err(1,"Can not open the file");

	// DOM startup
	
	LIBXML_TEST_VERSION;
	doc = xmlNewDoc(BAD_CAST "1.0"); // XML 1.0
	
	// Seems to be correct way to set root namespace. I found it by
	// trial and error. Libxml2 folks have skipped documentation.
	
	xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "cgm");
	xmlNsPtr ns_CGM = xmlNewNs(root, "http://codegrove.org/2009/cgm", NULL);
	xmlDocSetRootElement(doc, root);
	
	// Set level indicators	
	cur_level->indentation = 0;
	cur_level->parent = root;
	

	//unsigned char buf[UTF8_MAX_BYTES];
	unsigned char *helper[2048]; // FIXME: hard coded
	utf8_string text;
	text.data = helper;
	text.len = 0;
	unsigned char *buf = text.data; // moving pointer	

	while (true) {
		int n = utf8_fgetc(file, buf);
		
		if (n == UTF8_FGETC_NO_DATA && feof(file) ) break; // normal EOF
		if (n < 0) errx(2,"Vika tiedostossa.");
		
		if (utf8_compare_char(buf, &newline)) {
			// other char
			// FIXME check
			buf += n; // Char ok for text buffer, taking that one.
			text.len += n;

			fwrite(buf, 1, n, stdout);
			printf("%d ",n);
		} else {
			*buf = '\0'; // temp.
			//FIXME counter
			xmlNewChild(root, ns_CGM, BAD_CAST "line", text.data);

			// Text node is done, rolling back
			buf=text.data;
			text.len = 0;
		}
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
	return(0);
}
#else
	int main(void) {
		errx(1, "Tree support not compiled in");
	}
#endif
