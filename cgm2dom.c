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
		 cgm_element_content_separator,
		 cgm_special_count};

int main(int argc, char **argv)
{
	utf8_string cgm_specials[cgm_special_count];

	xmlDocPtr doc = NULL;            // document pointer
	struct level levels[MAX_LEVELS];
	int cur_level_i = 0;
	struct level *cur_level = levels; // pointer to the first element 
	utf8_string newline = utf8_literal_to_string("\n");

	printf("pituus: %d, tavuja %d\nsisus: %s\n", newline.length,
	       newline.bytes, newline.data);

	if (argc < 2 || argc > 3) 
		errx(1, "Usage: %s CGM_FILE [OUTPUT_FILE]", argv[0]);

	// Opening CGM file to memory
	struct mmap_info info =	mmap_fopen(argv[1], mmap_mode_volatile_write);
	if (info.state == mmap_state_error)
		err(1,"Can not open a file %s for reading", argv[1]);
     

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
/*
int read_cgm_header(FILE *file,
		    cgm_specials[cgm_special_count][UTF8_MAX_BYTES]) {

	int cgm_magic_len = 4;
	unsigned char cgm_magic_correct = "cgm ";
	unsigned char cgm_magic_buf[cgm_magic_len];
	unsigned char *pos;
	int n;

	n = utf8_fgetc(file, cgm_specials[cgm_element_start]);
	if (n<0) return n;

	n = fread(cgm_magic_buf, 1, cgm_magic_len, file);
	if (n < cgm_magic_len) return -1;
	if ( memcmp(cgm_magic_buf, cgm_magic_correct, cgm_magic_len ) )
		return -1;

	//lue loput
	n = utf8_fgets(file, cgm_magic, 4);
	int utf8_starts_with(unsigned char *buf, utf8_string *str);

	cgm_element_end,
		 cgm_escape,
		 cgm_element_content_separator,}
*/
#else
	int main(void) {
		errx(1, "Tree support not compiled in");
	}
#endif
