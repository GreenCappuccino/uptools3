#ifndef _INK_WIDGET_
#define _INK_WIDGET_

/* define the config specs for InkWidget */

#define NINK_SPECS 35
#define inkconfigSpecs \
	{TK_CONFIG_INT, "-request_cursor_x", (char *) NULL, (char *) NULL,\
		"-1", Tk_Offset(InkMaster, ink.request_cursor_x), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-request_cursor_y", (char *) NULL, (char *) NULL,\
		"-1", Tk_Offset(InkMaster, ink.request_cursor_y), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_suspend", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink.suspend), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_UID, "-foreground", (char *) NULL, (char *) NULL,\
		"#000000", Tk_Offset(InkMaster, ink.fgUid), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_UID, "-background", (char *) NULL, (char *) NULL,\
		"#0000FF", Tk_Offset(InkMaster, ink.bgUid), 0},\
	{TK_CONFIG_STRING, "-data", (char *) NULL, (char *) NULL,\
		(char *) NULL, Tk_Offset(InkMaster, ink.dataString), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_STRING, "-label", (char *) NULL, (char *) NULL,\
		(char *) NULL, Tk_Offset(InkMaster, ink.labelString), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-nsamples", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink.nsamples), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-cursor", (char *) NULL, (char *) NULL,\
		"-1", Tk_Offset(InkMaster, ink.cursor), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_DOUBLE, "-minpres", (char *) NULL, (char *) NULL,\
		"1.0", Tk_Offset(InkMaster, ink.minpres), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-width", (char *) NULL, (char *) NULL,\
		"500", Tk_Offset(InkMaster, ink.width), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-height", (char *) NULL, (char *) NULL,\
		"500", Tk_Offset(InkMaster, ink.height), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_STRING, "-subsegments", (char *) NULL, (char *) NULL,\
		(char *) NULL, Tk_Offset(InkMaster, ink.subsegmentsString), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_UID, "-ccolor", (char *) NULL, (char *) NULL,\
		"deeppink1", Tk_Offset(InkMaster, ink.ccolor), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_UID, "-scolor", (char *) NULL, (char *) NULL,\
		"yellow", Tk_Offset(InkMaster, ink.scolor), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_UID, "-pd_color", (char *) NULL, (char *) NULL,\
		"black", Tk_Offset(InkMaster, ink.pd_color), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_UID, "-pu_color", (char *) NULL, (char *) NULL,\
		"magenta", Tk_Offset(InkMaster, ink.pu_color), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_UID, "-sel_color", (char *) NULL, (char *) NULL,\
		"purple", Tk_Offset(InkMaster, ink.sel_color), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_DOUBLE, "-margin", (char *) NULL, (char *) NULL,\
		"0.03", Tk_Offset(InkMaster, ink.margin), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-cwidth", (char *) NULL, (char *) NULL,\
		"1", Tk_Offset(InkMaster, ink.cwidth), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-swidth", (char *) NULL, (char *) NULL,\
		"2", Tk_Offset(InkMaster, ink.swidth), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-uwidth", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink.uwidth), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-dwidth", (char *) NULL, (char *) NULL,\
		"1", Tk_Offset(InkMaster, ink.dwidth), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-sel_width", (char *) NULL, (char *) NULL,\
		"3", Tk_Offset(InkMaster, ink.sel_width), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-text_offset", (char *) NULL, (char *) NULL,\
		"-200", Tk_Offset(InkMaster, ink.text_offset), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_show_sample", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink._show_sample), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_show_subsegs", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink._show_subsegs), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_only_subsegs", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink._only_subsegs), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_mark_penstreams", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink._mark_penstreams), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_use_same_scale", (char *) NULL, (char *) NULL,\
		"1", Tk_Offset(InkMaster, ink._use_same_scale), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_remember_bounds", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink._remember_bounds), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_ignore_penups", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink._ignore_penups), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-lowpass_nsmooth", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink.lowpass_nsmooth), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-lowpass_nrepeat", (char *) NULL, (char *) NULL,\
		"3", Tk_Offset(InkMaster, ink.lowpass_nrepeat), TK_CONFIG_NULL_OK},\
	{TK_CONFIG_INT, "-_Spatial_Resampling", (char *) NULL, (char *) NULL,\
		"0", Tk_Offset(InkMaster, ink.spatial_resampling), TK_CONFIG_NULL_OK},\

/* BELOW IS THE DEFINITION OF A ContainerBox */
/* NOTE (0,0) is upper-left corner */

typedef struct {
	int x_org;   /* x0 of the total container */
	int y_org;   /* y0 of the total container */
	int width;   /* width of the total container */
	int height;  /* height of the total container */
	float wfrac; /* width fraction of box */
	float hfrac; /* height fraction of box */
	float xfrac; /* horizontal offset fraction */
	float yfrac; /* vertical offset fraction */
	int showbox; /* whether to show the bounding box or not */
	int x_annot; /* whether to show the x-axis annotation or not */
	int y_annot; /* whether to show the y-axis annotation or not */
	char signam[10]; /* Name of the signal (aka box title) */
} ContainerBox;

#define BOX_XORG(box)   (box->x_org + box->xfrac*box->width)
#define BOX_YORG(box)   (box->y_org + box->yfrac*box->height)
#define BOX_WIDTH(box)  (box->wfrac*box->width)
#define BOX_HEIGHT(box) (box->hfrac*box->height)
#define BOX_XTORG(box)   (box->x_org + (1.3*box->xfrac)*box->width)

/* sub-segment syntax {n s_0 e_0 c_0 w_0 ... s_n-1 e_n-1 c_n-1 w_n-1}

	s: start of sub-segment
	e: end of sub-segment
	c: color of sub-segment
	w: linewidth of sub-segment
*/

typedef struct {
	int     nsubsegments;
	char    **m_name;
	Tk_Uid  *m_color;
	int     *m_width;
	int     *m_start;
	int     *m_end;
} InkSubSegments;

#define MAX_SUBGC 15
typedef struct InkWidgetResources {

	/* if an image is suspended, each Display event is ignored */

	int suspend;

	/* if an image is configured with request_cursor_x and
	   request_cursor_y set, handle this with ink_return_cursor_pos */
	
	int request_cursor_x;
	int request_cursor_y;

	/* resources dealing width UNIPEN, INK etc. */

	char *dataString;
	char *labelString;
	int nsamples;
	int cursor;
	double *samples[3];
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double minpres;

	double fsample_of_data;
	double resol_mm;
	double resol_g;
	
        /* resources dealing with filtering and resampling */
        
        int lowpass_nsmooth;
        int lowpass_nrepeat;
        int spatial_resampling;

	/* Tcl/Tk and X resources */

	Tcl_Interp *interp;
	Tk_Window tkwin;
	Tk_Uid fgUid;             /* Value of -foreground option (malloc'ed). */
	Tk_Uid bgUid;             /* Value of -background option (malloc'ed). */
	XColor *fg;
	XColor *bg;
	Display *display;
	Drawable drawable;
	int width;
	int height;

	/* and other graphical resources *
	 * which also may be used by signal display routines */

	Tk_Uid ccolor;       /* color of cursor           */
	Tk_Uid scolor;       /* color of sample points    */
	Tk_Uid pd_color;     /* color of pen-down streams */
	Tk_Uid pu_color;     /* color of pen-up streams   */
	Tk_Uid sel_color;    /* color of marked ink       */
	XColor *cursor_col;
	XColor *sample_col;
	XColor *pd_col;
	XColor *bd_col;
	XColor *pu_col;
	XColor *sel_col;
	GC cursor_gc;
	GC sample_gc;
	GC pd_gc;
	GC pu_gc;
	GC sel_gc;
	GC m_gc[MAX_SUBGC];
	GC bd_gc;           /* graphical context for borders */
	char *subsegmentsString;
	double margin;
	InkSubSegments subsegments;
	int cwidth;
	int swidth;
	int uwidth;
	int dwidth;
	int sel_width;
	int text_offset;
	int _show_sample;
	int _show_subsegs;
	int _only_subsegs;
	int _mark_penstreams;
	int _use_same_scale;
	int _remember_bounds;
	int _ignore_penups;
} InkWidgetResources;

/* something about colors and fonts: global because only to be allocated once */

extern int ncolors;
extern XColor **color_ptr;
extern char **color_names;

#define N_INK_FONTS   3
#define LABEL_FONT    ink_fonts[0]
#define SUBSEG_FONT   ink_fonts[1]
#define GRAPHICS_FONT ink_fonts[2]
#define GRAPHICS_GC   ink_fontgcs[2]
#define LABEL_GC      ink_fontgcs[0]
extern int fonts_initialized;
extern Font ink_fonts[N_INK_FONTS];
extern GC ink_fontgcs[N_INK_FONTS];
extern char *font_names[N_INK_FONTS];
extern char *font_colors[N_INK_FONTS];

extern int use_global_bounds;
extern double Xrange,Yrange;

extern int inkSetMinMax      (ClientData, Tcl_Interp *, int, char **);
extern int inkUnSetMinMax    (ClientData, Tcl_Interp *, int, char **);
extern int inkSetColors      (ClientData, Tcl_Interp *, int, char **);
extern int inkSetFonts       (ClientData, Tcl_Interp *, int, char **);
extern int inkSetTabletInfo  (ClientData, Tcl_Interp *, int, char **);

extern int inkConfigure      (InkWidgetResources *);
extern void inkCreate        (Tcl_Interp *, InkWidgetResources *);
extern void inkDisplay       (Drawable, InkWidgetResources *);
extern void inkDelete        (InkWidgetResources *);

extern void DrawCursor       (InkWidgetResources *, int, int, ContainerBox *, int);

#endif
