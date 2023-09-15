#include <stdio.h>
#include <stdlib.h>
#include "ink_master.h"
#include "ink_widget.h"
#include "ink_hwr.h"
#include "ink_vabs.h"

/*
 *----------------------------------------------------------------------
 *
 * DllEntryPoint --
 *
 *	This wrapper function is used by Windows to invoke the
 *	initialization code for the DLL.  If we are compiling
 *	with Visual C++, this routine will be renamed to DllMain.
 *	routine.
 *
 * Results:
 *	Returns TRUE;
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#ifdef __WIN32__
BOOL APIENTRY
DllEntryPoint(hInst, reason, reserved)
    HINSTANCE hInst;		/* Library instance handle. */
    DWORD reason;		/* Reason this function is being called. */
    LPVOID reserved;		/* Not used. */
{
    return TRUE;
}
#endif

/*
 * The type record for ink images:
 */

static int ImgInkCreate _ANSI_ARGS_((Tcl_Interp *interp,
			    char *name, int argc, char **argv,
			    Tk_ImageType *typePtr, Tk_ImageMaster master,
			    ClientData *clientDataPtr));
static ClientData ImgInkGet _ANSI_ARGS_((Tk_Window tkwin,
			    ClientData clientData));
static void ImgInkDisplay _ANSI_ARGS_((ClientData clientData,
			    Display *display, Drawable drawable, 
			    int imageX, int imageY, int width, int height,
			    int drawableX, int drawableY));
static void ImgInkFree _ANSI_ARGS_((ClientData clientData,
			    Display *display));
static void ImgInkDelete _ANSI_ARGS_((ClientData clientData));

Tk_ImageType inkImageType = {
    "ink",                  /* name */
    ImgInkCreate,           /* createProc */
    ImgInkGet,              /* getProc */
    ImgInkDisplay,          /* displayProc */
    ImgInkFree,             /* freeProc */
    ImgInkDelete,           /* deleteProc */
    (Tk_ImageType *) NULL   /* nextPtr */
};

static Tk_ConfigSpec configSpecs[NINK_SPECS+NHWR_SPECS+NVABS_SPECS+1] = {

	inkconfigSpecs
	hwrconfigSpecs
	vabsconfigSpecs

	{TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
		(char *) NULL, 0, 0}

};


int Ink_Init(interp)
    Tcl_Interp *interp;
{
	if (Tcl_PkgRequire(interp,"Tk",EXPECTED_TK_VERSION,0)==NULL) {
		return TCL_ERROR;
	}
	Tk_CreateImageType(&inkImageType);
	Tcl_CreateCommand (interp,"ink_set_tablet_info",inkSetTabletInfo,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"ink_set_colors",inkSetColors,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"ink_set_fonts",inkSetFonts,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"ink_unset_min_max",inkUnSetMinMax,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"ink_set_min_max",inkSetMinMax,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	
	if (Tcl_PkgProvide (interp,"Ink",UNIPEN_VERSION)!=TCL_OK) {
		return TCL_ERROR;
	}
	
	return TCL_OK;
}

/*
 * Prototypes for procedures used only locally in this file:
 */

static int		ImgInkCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static void		ImgInkCmdDeletedProc _ANSI_ARGS_((
			    ClientData clientData));
static int ImgInkConfigureMaster (InkMaster *, int, char **, int flags);

/*

Local helpers

*/
#define HWR_BOX  0
#define SIG1_BOX 1
#define SIG2_BOX 2
#define SIG3_BOX 3
#define NBOXMAX  4 /* Huidig maximum */

static void make_layout(LayoutBox layoutbox[NBOXMAX]
                               , int showhwr
	                       , int showvabs
	                       , char *signam1
	                       , char *signam2
	                       , char *signam3)
{
	int sigbits[NBOXMAX], i, j, nsigs, any_x_annot;
	double xmarg, ymarg, xx, yy, ww, hh;
	
	for(i = 0; i < NBOXMAX; ++i) {
		sigbits[i] = 0;
		layoutbox[i].x_annot = 0;
		layoutbox[i].y_annot = 0;
		strcpy(layoutbox[i].signam,"NONE");
	}
	
	/* Fold the dedicated flags flat into sigbits[] */
	
	if(showhwr) sigbits[HWR_BOX] = 1;
	if(showvabs) {
		if(strcmp(signam1,"NONE") != 0)  sigbits[SIG1_BOX] = 1;
		if(strcmp(signam2,"NONE") != 0)  sigbits[SIG2_BOX] = 1;
		if(strcmp(signam3,"NONE") != 0)  sigbits[SIG3_BOX] = 1;
		
	}
	
	/* From here on generic layout arrays */
	
	nsigs = 0;
	for(i = 0; i < NBOXMAX; ++i) {
		if(sigbits[i]) ++nsigs;
	}
	if(nsigs < 1) nsigs = 1;
	
	xmarg = 0.02;
	ymarg = 0.02;
	
	xx = xmarg;
	yy = ymarg;
	
	hh = (1.0 - 2. * ymarg) / nsigs;
	ww = (1.0 - 2. * xmarg);

	any_x_annot = 0;
	j = 0;
	for(i = 0; i < NBOXMAX; ++i) {
		if(sigbits[i]) {
			if(i == 1) strcpy(layoutbox[i].signam, signam1);
			if(i == 2) strcpy(layoutbox[i].signam, signam2);
			if(i == 3) strcpy(layoutbox[i].signam, signam3);
	    	
				/* 
					We prefer x_annot in the 0-pos-ymax type graphs (not
					in the bipolar neg-0-pos graphs. However, if there is
					no x_annot in the end (at the last signal) we have to
					accept it 
				*/
	        
			if(!any_x_annot) {
				if((strcmp(layoutbox[i].signam,"P") == 0) ||
					(strcmp(layoutbox[i].signam,"VA") == 0) ||
					(strcmp(layoutbox[i].signam,"AA") == 0) 
					) {
					layoutbox[i].x_annot = 1;
					any_x_annot = 1;
				}
			}
			if(j == 3 && !any_x_annot) {
				layoutbox[i].x_annot = 1;
			}

			++j;
			layoutbox[i].x = xx;
			layoutbox[i].y = yy;
			layoutbox[i].w = ww;
			layoutbox[i].h = hh - ymarg;
			yy += hh;
		}
	}
}

/* ---------------- */

static int
ImgInkCreate(interp, name, argc, argv, typePtr, master, clientDataPtr)
    Tcl_Interp *interp;        /* Interpreter for application containing image. */
    char *name;                /* Name to use for image. */
    int argc;                  /* Number of arguments. */
    char **argv;               /* Argument strings for options (doesn't include image name or type). */
    Tk_ImageType *typePtr;     /* Pointer to our type record (not used). */
    Tk_ImageMaster master;     /* Token for image, to be used by us in later callbacks. */
    ClientData *clientDataPtr; /* Store manager's token for image here; it will be returned in later callbacks. */
{
	InkMaster *masterPtr;

	if (ncolors==0) {
		fprintf (stderr,"first set colors using 'ink_set_colors colors'!\n");
		Tcl_AppendResult (interp,"first set colors using 'ink_set_colors colors'!",NULL);
		return 1;
	}

	if (!fonts_initialized) {
		fprintf (stderr,"first set fonts using 'ink_set_fonts fnt1 fnt2 fnt3'!\n");
		Tcl_AppendResult (interp,"first set fonts using 'ink_set_fonts fnt1 col1 fnt2 col2 fnt3 col3'!",NULL);
		return 1;
	}

	masterPtr = (InkMaster *) ckalloc(sizeof(InkMaster));
	memset((char *)masterPtr,0,sizeof(InkMaster));
	masterPtr->tkMaster = master;
	masterPtr->interp = interp;
	masterPtr->imageCmd = Tcl_CreateCommand(interp, name, ImgInkCmd,
		(ClientData) masterPtr, ImgInkCmdDeletedProc);
	inkCreate  (interp,&masterPtr->ink);
	hwrCreate  (&masterPtr->hwr);
	vabsCreate (&masterPtr->vabs, interp);

	if (ImgInkConfigureMaster(masterPtr, argc, argv, 0) != TCL_OK) {
		ImgInkDelete((ClientData) masterPtr);
		return TCL_ERROR;
	}
	*clientDataPtr = (ClientData) masterPtr;
	return TCL_OK;
}

static int
ImgInkConfigureMaster(masterPtr, argc, argv, flags)
    InkMaster *masterPtr;	/* Pointer to data structure describing image */
    int argc;			/* Number of entries in argv. */
    char **argv;		/* Pairs of configuration options for image. */
    int flags;			/* Flags to pass to Tk_ConfigureWidget,
				 * such as TK_CONFIG_ARGV_ONLY. */
{
	char sbuf[16];
	int index;

	if (Tk_ConfigureWidget(masterPtr->interp, Tk_MainWindow(masterPtr->interp),
			configSpecs, argc, argv, (char *) masterPtr, flags)
			!= TCL_OK) {
		return TCL_ERROR;
	}

	if (masterPtr->ink.request_cursor_x==-2 && masterPtr->ink.request_cursor_y==-2) {
		masterPtr->ink.request_cursor_x = -1;
		masterPtr->ink.request_cursor_y = -1;
		return TCL_OK;
	}

	if (masterPtr->ink.request_cursor_x>=0 && masterPtr->ink.request_cursor_y>=0) {
		index = hwrGetCursorInSignal (&masterPtr->ink,&masterPtr->hwr
			,masterPtr->ink.request_cursor_x,masterPtr->ink.request_cursor_y);
		sprintf (sbuf,"%d  ",index);
		Tcl_AppendResult(masterPtr->interp,sbuf,NULL);
		return TCL_OK;
	}

	if (inkConfigure(&masterPtr->ink)!=TCL_OK) {
		Tcl_AddErrorInfo(masterPtr->interp, "\n    (while configuring image \"");
		Tcl_AddErrorInfo(masterPtr->interp, Tk_NameOfImage(masterPtr->tkMaster));
		Tcl_AddErrorInfo(masterPtr->interp, "\")");
		Tcl_BackgroundError(masterPtr->interp);
	}

	Tk_ImageChanged(masterPtr->tkMaster, 0, 0, masterPtr->ink.width/2,
		masterPtr->ink.height/2, masterPtr->ink.width, masterPtr->ink.height);

	/* generate result */

	sprintf (sbuf,"%d  ",masterPtr->ink.nsamples);
	if (masterPtr->ink.labelString!=NULL)
		Tcl_AppendResult(masterPtr->interp,sbuf,masterPtr->ink.labelString,NULL);
	else
		Tcl_AppendResult(masterPtr->interp,sbuf,NULL);

	return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * ImgInkCmd --
 *
 *	This procedure is invoked to process the Tcl command
 *	that corresponds to an image managed by this module.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *--------------------------------------------------------------
 */

static int
ImgInkCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Information about button widget. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
	InkMaster *masterPtr = (InkMaster *) clientData;
	int c, code;
	size_t length;

	if (argc < 2) {
		sprintf(interp->result,
			"wrong # args: should be \"%.50s option ?arg arg ...?\"",
			argv[0]);
		return TCL_ERROR;
	}
	c = argv[1][0];
	length = strlen(argv[1]);
	if ((c == 'c') && (strncmp(argv[1], "cget", length) == 0)
			&& (length >= 2)) {
		if (argc != 3) {
			Tcl_AppendResult(interp, "wrong # args: should be \"",
				argv[0], " cget option\"",
				(char *) NULL);
			return TCL_ERROR;
		}
		return Tk_ConfigureValue(interp, Tk_MainWindow(interp), configSpecs,
			(char *) masterPtr, argv[2], 0);
	} else if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)
			&& (length >= 2)) {
		if (argc == 2) {
			code = Tk_ConfigureInfo(interp, Tk_MainWindow(interp),
			configSpecs, (char *) masterPtr, (char *) NULL, 0);
		} else if (argc == 3) {
			code = Tk_ConfigureInfo(interp, Tk_MainWindow(interp),
			configSpecs, (char *) masterPtr, argv[2], 0);
		} else {
			code = ImgInkConfigureMaster(masterPtr, argc-2, argv+2,
				TK_CONFIG_ARGV_ONLY);
		}
		return code;
	} else {
		Tcl_AppendResult(interp, "bad option \"", argv[1],
			"\": must be cget or configure", (char *) NULL);
		return TCL_ERROR;
	}
}

/*
 *----------------------------------------------------------------------
 *
 * ImgInkGet --
 *
 *	This procedure is called for each use of a ink image in a
 *	widget.
 *
 * Results:
 *	The return value is a token for the instance, which is passed
 *	back to us in calls to ImgInkDisplay and ImgInkFree.
 *
 * Side effects:
 *	A data structure is set up for the instance (or, an existing
 *	instance is re-used for the new one).
 *
 *----------------------------------------------------------------------
 */

static ClientData
ImgInkGet(tkwin, masterData)
    Tk_Window tkwin;
    ClientData masterData;
{
	return masterData;
}

/*
 *----------------------------------------------------------------------
 *
 * ImgInkDisplay --
 *
 *	This procedure is invoked to draw a ink image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A portion of the image gets rendered in a pixmap or window.
 *
 *----------------------------------------------------------------------
 */
static void
ImgInkDisplay(clientData, display, drawable, imageX, imageY, width,
	height, drawableX, drawableY)
    ClientData clientData;	/* Pointer to InkInstance structure for
				 * for instance to be displayed. */
    Display *display;		/* Display on which to draw image. */
    Drawable drawable;		/* Pixmap or window in which to draw image. */
    int imageX, imageY;		/* Upper-left corner of region within image
				 * to draw. */
    int width, height;		/* Dimensions of region within image to draw. */
    int drawableX, drawableY;	/* Coordinates within drawable that
				 * correspond to imageX and imageY. */
{
	InkMaster *masterPtr = (InkMaster *) clientData;
	ContainerBox *box;
	
	LayoutBox layoutbox[NBOXMAX];

	if (masterPtr->ink.nsamples<=0 || masterPtr->ink.suspend) {
		return;
	}

	/*  a l w a y s   call inkDisplay */
	inkDisplay(drawable,&masterPtr->ink);
	
	make_layout(layoutbox  , masterPtr->hwr.showhwr
	                       , masterPtr->vabs.showvabs
	                       ,(masterPtr->vabs).signam1
	                       ,(masterPtr->vabs).signam2
	                       ,(masterPtr->vabs).signam3);

	/* fill box for hwr and call hwrDisplay */
	if (masterPtr->hwr.showhwr) {
		box = &masterPtr->hwr.box;
		box->x_org = drawableX-imageX;
		box->y_org = drawableY-imageY;
		box->width = width;
		box->height = height;
		box->xfrac = layoutbox[HWR_BOX].x;
		box->yfrac = layoutbox[HWR_BOX].y;
		box->wfrac = layoutbox[HWR_BOX].w;
		box->hfrac = layoutbox[HWR_BOX].h;
		box->showbox = 1;
		hwrDisplay (&masterPtr->ink,&masterPtr->hwr);
	}

	if (masterPtr->ink.nsamples<=2) {
		return;
	}
	if (masterPtr->vabs.showvabs) {
		/* fill box for SIGNAL1 and call vabsDisplay */

		box = &masterPtr->vabs.box;
		box->x_org = drawableX-imageX;
		box->y_org = drawableY-imageY;
		box->width = width;
		box->height = height;
		box->showbox = 1;
		box->xfrac = layoutbox[SIG1_BOX].x;
		box->yfrac = layoutbox[SIG1_BOX].y;
		box->wfrac = layoutbox[SIG1_BOX].w;
		box->hfrac = layoutbox[SIG1_BOX].h;
		box->x_annot = layoutbox[SIG1_BOX].x_annot;
		strcpy(box->signam, layoutbox[SIG1_BOX].signam);
		vabsDisplay (&masterPtr->ink,&masterPtr->vabs);

		/* fill box for SIGNAL2 and call vabsDisplay */

		box->showbox = 1;
		box->xfrac = layoutbox[SIG2_BOX].x;
		box->yfrac = layoutbox[SIG2_BOX].y;
		box->wfrac = layoutbox[SIG2_BOX].w;
		box->hfrac = layoutbox[SIG2_BOX].h;
		box->x_annot = layoutbox[SIG2_BOX].x_annot;
		strcpy(box->signam, layoutbox[SIG2_BOX].signam);
		vabsDisplay (&masterPtr->ink,&masterPtr->vabs);

		/* fill box for SIGNAL3 and call vabsDisplay */

		box->showbox = 1;
		box->xfrac = layoutbox[SIG3_BOX].x;
		box->yfrac = layoutbox[SIG3_BOX].y;
		box->wfrac = layoutbox[SIG3_BOX].w;
		box->hfrac = layoutbox[SIG3_BOX].h;
		box->x_annot = layoutbox[SIG3_BOX].x_annot;
		strcpy(box->signam, layoutbox[SIG3_BOX].signam);
		vabsDisplay (&masterPtr->ink,&masterPtr->vabs);
	}
}

/*
 *----------------------------------------------------------------------
 *
 * ImgInkFree --
 *
 *	This procedure is called when a widget ceases to use a
 *	particular instance of an image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Internal data structures get cleaned up.
 *
 *----------------------------------------------------------------------
 */

static void
ImgInkFree(clientData, display)
    ClientData clientData;	/* Pointer to InkInstance structure for
				 * for instance to be displayed. */
    Display *display;		/* Display containing window that used image. */
{
}

/*
 *----------------------------------------------------------------------
 *
 * ImgInkDelete --
 *
 *	This procedure is called by the image code to delete the
 *	master structure for an image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Resources associated with the image get freed.
 *
 *----------------------------------------------------------------------
 */

static void ImgInkDelete(ClientData masterData)
{
	InkMaster *masterPtr = (InkMaster *) masterData;

	inkDelete(&masterPtr->ink);
	hwrDelete(&masterPtr->hwr);
	vabsDelete(&masterPtr->vabs);

	masterPtr->tkMaster = NULL;
	if (masterPtr->imageCmd != NULL) {
		Tcl_DeleteCommand(masterPtr->interp,
		Tcl_GetCommandName(masterPtr->interp, masterPtr->imageCmd));
	}
	Tk_FreeOptions(configSpecs, (char *) masterPtr, (Display *) NULL, 0);
	ckfree((char *) masterPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ImgInkCmdDeletedProc --
 *
 *	This procedure is invoked when the image command for an image
 *	is deleted.  It deletes the image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The image is deleted.
 *
 *----------------------------------------------------------------------
 */

static void
ImgInkCmdDeletedProc(clientData)
    ClientData clientData;	/* Pointer to InkMaster structure for
				 * image. */
{
	InkMaster *masterPtr = (InkMaster *) clientData;

	masterPtr->imageCmd = NULL;
	if (masterPtr->tkMaster != NULL) {
		Tk_DeleteImage(masterPtr->interp, Tk_NameOfImage(masterPtr->tkMaster));
	}
}
