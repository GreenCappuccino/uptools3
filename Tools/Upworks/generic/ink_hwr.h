#ifndef _INK_HWR_
#define _INK_HWR_

#include "ink_widget.h"

/* define the config specs for HWR */

#define NHWR_SPECS 1
#define hwrconfigSpecs \
   {TK_CONFIG_INT, "-_showhwr", (char *) NULL, (char *) NULL,\
	      "1", Tk_Offset(InkMaster, hwr.showhwr), TK_CONFIG_NULL_OK},\

typedef struct HwrResources {
	double *X;
	double *Y;
	int bufsize;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	ContainerBox box;
	int showhwr;
} HwrResources;

extern void hwrCreate  (HwrResources *);
extern void hwrDisplay (InkWidgetResources *, HwrResources *);
extern void hwrDelete  (HwrResources *);

extern int hwrGetCursorInSignal(InkWidgetResources *, HwrResources *,int,int);

#endif
