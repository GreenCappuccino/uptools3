#ifdef USE_WID

#ifndef _DIR_WRITER_IDS_
#define _DIR_WRITER_IDS_

#include <uplib.h>

typedef struct {
	char level[64];
	int seg_idx;
	char seg_name[64];
	int idx;
	char *writer_name;
	int dataset;
	char *path;
} UnipenIdentification;

extern void parse_writercode (char *unipen_code, UnipenIdentification *unipen_id);
extern char *writerid_2_upfile (UnipenIdentification *unipen_id);

extern int  read_unipen_identifications (char *fname);
extern int updirFilename2UnipenCode (char *fname);
extern char *updirEntry2UnipenCode (tUPEntry *entry, tUPEntry **entries
   , int nentries, int wcode, int char_only);

#endif

#endif
