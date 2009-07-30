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
	char *newline = "\n";

	if (argc < 2 || argc > 3) 
		errx(1, "Usage: %s CGM_FILE [OUTPUT_FILE]", argv[0]);
     
	FILE *file = fopen(argv[1], "rb");
	if ( file == NULL) err(1,"Can not open the file");

	xmlNsPtr nsCGM = xmlNewNs(NULL, "http://codegrove.org/2009/cgm", "cgm");

	// DOM startup
	{
		LIBXML_TEST_VERSION;
		doc = xmlNewDoc(BAD_CAST "1.0"); // XML 1.0
		
		// UNTESTED! FIXME.
		xmlNodePtr root = xmlNewNode(nsCGM, BAD_CAST "cgm");
		xmlDocSetRootElement(doc, root);

		cur_level->indentation = 0;
		cur_level->parent = root;
	}

	char buf[UTF8_MAX_BYTES];
		
	while (true) {
		int n = utf8_fgetc(file, buf);
		
		if (n == UTF8_FGETC_NO_DATA && feof(file) ) break; // normal EOF
		if (n < 0) errx(2,"Vika tiedostossa.");
		
		fwrite(buf, 1, n, stdout);
		printf("%d ",n);
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
