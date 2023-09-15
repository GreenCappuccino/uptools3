#ifndef _INK_MASTER
#define _INK_MASTER

#include <stdio.h>
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif
#include <string.h>
#include <tk.h>

#include "ink_widget.h"
#include "ink_hwr.h"
#include "ink_vabs.h"

#if defined(__WIN32__)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   undef WIN32_LEAN_AND_MEAN

/*
 * VC++ has an alternate entry point called DllMain, so we need to rename
 * our entry point.
 */

#   if defined(_MSC_VER)
#  define EXPORT(a,b) __declspec(dllexport) a b
#  define DllEntryPoint DllMain
#   else
#  if defined(__BORLANDC__)
#      define EXPORT(a,b) a _export b
#  else
#      define EXPORT(a,b) a b
#  endif
#   endif
#else
#   define EXPORT(a,b) a b
#endif

typedef struct {

	/* MASTER RESOURCES */

    Tk_ImageMaster tkMaster; /* Tk's token for image master.  NULL means
                              * the image is being deleted. */
    Tcl_Interp *interp;      /* Interpreter for application that is
                              * using image. */
    Tcl_Command imageCmd;    /* Token for image command (used to delete
									   * it when the image goes away).  NULL means
									   * the image command has already been
									   * deleted. */

	/* INK WIDGET RESOURCES */

	InkWidgetResources ink;

	/* HWR RESOURCES  */

	HwrResources hwr;

	/* VABS RESOURCES */

	VabsResources vabs;

} InkMaster;

typedef struct {
	double x,y,w,h;
	int x_annot;
	int y_annot;
	char signam[10];
} LayoutBox;

#endif
