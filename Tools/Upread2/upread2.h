#ifndef _UPREAD2_
#define _UPREAD2_

#include <uplib.h>
#include <upsiglib.h>
#include <up_segment_io.h>

#ifndef SUN_SOL
#include <dirent.h>
#define DIR_DEFINE dirent
#else
#include <sys/dir.h>
#define DIR_DEFINE direct
#endif
 

#ifndef UR_VERSION
#define UR_VERSION "V3.0 June 1997 - L.Schomaker, L.Vuurpijl, G.Abbink  - NICI"
#endif

#define ANY_HIERARCHY "PAGE PARAGRAPH SENTENCE LINE WORD CHARACTER DIGIT"
/* Each output function has as input OFunction_Info *oinfo, containing
   fields required by the different output functions. If you add your own,
	and if you require extra fields, just put them in this struct.
*/

#define NOUTPUT_FUNCTIONS   4
#define _OUTPUT_UNIPEN   0
#define _OUTPUT_FEATCHAR 1
#define _OUTPUT_IMAGE    2
#define _OUTPUT_PPM      3

typedef struct {
	char outfile[512];
	char index_file[512];
	int output_function;
	int m;             /* number of points to resample to -1 = don't resample       */
	int do_add_sincos; /* indicates if running angles (dx,dy) must be added         */
	int do_add_phi;    /* indicates if angle velocities (dphix,dphiy) must be added */
	double dfact;      /* factor for adjusting resampling (between [0,1]) */
	int ncols;         /* for output image: the number of columns */
	int width;         /* width of image */
	int height;        /* height of image */
	int brush;         /* for line generator */
	int margin;        /* for output_image */
	int im_frmt;       /* either O_XBM or O_PPM */
	int writer_code;   /* corresponding to file currently opened */
	int add_comments;  /* for also adding comments in case of UNIPEN */
	int char_only;     /* for output featchar (full_name or not) */
	int determine_wc;  /* does not determine writer code (if given as arg) */
	int use_Z;
	
	int use_same_scale;
} OFunction_Info;

extern void init_output_unipen (tUPUnipen *, FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo);
extern void init_output_featchar (FILE *fp_out, upsegQueryInfo *info
    , OFunction_Info *oinfo);
extern int output_unipen (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams);
extern int output_image (upsegQueryInfo *info, OFunction_Info *oinfo
    , FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams);
extern int output_ppm (upsegQueryInfo *info, OFunction_Info *oinfo
    , FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams);
extern int output_featchar (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams);

#endif
