#ifndef _INK_VABS_
#define _INK_VABS_


#define TBOX_XORG(box)   (box->x_org + box->xfrac*box->width)
#define TBOX_YORG(box)   (box->y_org + box->yfrac*box->height)
#define TBOX_WIDTH(box)  (box->wfrac*box->width)
#define TBOX_HEIGHT(box) (box->hfrac*box->height)

/* define the config specs for VABS */

#define NVABS_SPECS 4
#define vabsconfigSpecs \
   {TK_CONFIG_INT, "-_Show_Signals", (char *) NULL, (char *) NULL,\
	  "1", Tk_Offset(InkMaster, vabs.showvabs), TK_CONFIG_NULL_OK},\
   {TK_CONFIG_STRING, "-signal1", (char *) NULL, (char *) NULL,\
	  "P", Tk_Offset(InkMaster, vabs.signam1), TK_CONFIG_NULL_OK},\
   {TK_CONFIG_STRING, "-signal2", (char *) NULL, (char *) NULL,\
	  "VA", Tk_Offset(InkMaster, vabs.signam2), TK_CONFIG_NULL_OK},\
   {TK_CONFIG_STRING, "-signal3", (char *) NULL, (char *) NULL,\
	  "DPHI", Tk_Offset(InkMaster, vabs.signam3), TK_CONFIG_NULL_OK},\

typedef struct VabsResources {
	int showvabs;
	char *signam1;
	char *signam2;
	char *signam3;
	int hide_vabs_penups;
	int n;
	double xmin;
	double xmax;
	double ymin;
	double ymax;

	double a_xmin;
	double a_xmax;
	double a_ymin;
	double a_ymax;
	double a_xzero;
	double a_yzero;
	double a_dx;
	double a_dy;
	
	double *sig_taxis; 
	double *sig_faxis; 
	int bufsize;
	ContainerBox box;
        Tcl_Interp *interp;
} VabsResources;


extern void vabsCreate (VabsResources *, Tcl_Interp *);
extern void vabsDisplay (InkWidgetResources *, VabsResources *);
extern void vabsDelete (VabsResources *);

#endif


