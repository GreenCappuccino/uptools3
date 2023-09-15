#ifndef _JUMP_THOUGH_
#define _JUMP_THOUGH_

#include <uplib.h>
#include <upsiglib.h>
#include <uphierarchy.h>

/* mark:     0 = less left
             1 = more left
             2 = less right
             3 = more right
	jumpsize: 0 = 1 sample
             1 = 10 samples
             1 = Y-extrema
             1 = penstreams
             >= 4 = take a segment from level l = (current level+jumpsize - 3)
*/

#define NBASIC_JUMPSIZES      6
#define JUMP_ONE_SAMPLE       0
#define JUMP_TEN_SAMPLES      1
#define JUMP_HUNDRED_SAMPLES  2
#define JUMP_THOUSEND_SAMPLES 3
#define JUMP_Y_EXTREMA        4
#define JUMP_PEN_STREAM       5

extern int do_get_child_markers (int, int, int, int, int
	,int *, int *
	,tUPLevel *
	,sigSignal *, tUPUnipen *,char *);

extern int do_set_segm_markers (int, int, int, int, int
	,tUPLevel *
	, tUPUnipen *);

extern sigSignal *do_get_edited_segment (tUPUnipen *, char *);
extern void initiate_segment_walk (tUPUnipen *, tUPLevel *, int, int);
extern sigDelineation *copy_current_delineation (void);
extern int do_get_segment_delineation (tUPUnipen *, int, int, char *);

#endif
