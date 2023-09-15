#include <stdio.h>
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif
#include <string.h>
#include <tk.h>

#include "ink_widget.h"
#include "ink_siglib.h"
#include "siglib.h"

int ncolors = 0;
XColor **color_ptr = NULL;
char **color_names = NULL;

static int fonts_reset = 0;
int fonts_initialized = 0;
Font ink_fonts[N_INK_FONTS];
GC ink_fontgcs[N_INK_FONTS];
char *font_names[N_INK_FONTS];
char *font_colors[N_INK_FONTS];

#define ck_strdup(s) strcpy(ckalloc(strlen(s)+1),s)

#ifndef FAKED_FSAMP
#define FAKED_FSAMP    100.0
#endif
#ifndef FAKED_RESOL_MM
#define FAKED_RESOL_MM  50.0
#endif
#ifndef FAKED_RESOL_G
#define FAKED_RESOL_G    1.0
#endif

double Xrange,Yrange;
static double fsample_of_data = FAKED_FSAMP;
static double resol_mm        = FAKED_RESOL_MM;
static double resol_g         = FAKED_RESOL_G;
int use_global_bounds = 0;

int inkSetTabletInfo (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i;

	if (argc!=4) {
		fprintf (stderr,"use: ink_set_tablet_info fsample_of_data resol_mm resol_g!");
		for (i=0;i<argc;i++)
			fprintf (stderr,"%d: %s\n",i,argv[i]);
		return TCL_ERROR;
	}
	i = 1;
	if (sscanf(argv[i],"%lf",&fsample_of_data)!=1) {
		fprintf (stderr,"ink_set_tablet_info: unable to get fsample_of_data from %s!\n",argv[i]);
		exit(1);
	}
	i++;
	if (sscanf(argv[i],"%lf",&resol_mm)!=1) {
		fprintf (stderr,"ink_set_tablet_info: unable to get resol_mm from %s!\n",argv[i]);
		exit(1);
	}
	i++;
	if (sscanf(argv[i],"%lf",&resol_g)!=1) {
		fprintf (stderr,"ink_set_tablet_info: unable to get resol_g from %s!\n",argv[i]);
		exit(1);
	}
	return TCL_OK;
}

int inkSetColors (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i;

	if (argc<2) {
		fprintf (stderr,"use: %s color [colors]!",argv[0]);
		return 1;
	}
	ncolors = argc-1;
	if (ncolors>MAX_SUBGC) {
		fprintf (stderr,"inkSetColors: only %d colors are allowed!!\n",MAX_SUBGC);
		fprintf (stderr,"you requested %d colors, TRUNCATING to %d\n",ncolors,MAX_SUBGC);
		ncolors = MAX_SUBGC;
	}
	color_names = (char **) ckalloc (ncolors*sizeof(char *));
	for (i=1;i<argc;i++)
		color_names[i-1] = ck_strdup(argv[i]);
	return TCL_OK;
}

int inkSetFonts (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i,j;

	if (argc!=1+N_INK_FONTS*2) {
		fprintf (stderr,"use: %s fnt0 col0",argv[0]);
		for (i=1;i<N_INK_FONTS;i++)
			fprintf (stderr," fnt%d col%d",i,i);
		fprintf (stderr,"!\n");
		fprintf (stderr,"got: %s\n",argv[0]);
		for (i=1;i<argc;i++)
			fprintf (stderr,"     %s\n",argv[i]);
		return 1;
	}
	for (j=0,i=1;i<argc;) {
		font_names[j] = ck_strdup(argv[i]);
		i++;
		font_colors[j] = ck_strdup(argv[i]);
		i++;
		j++;
	}
	fonts_initialized = 1;
	fonts_reset = 1;
	return TCL_OK;
}


int inkSetMinMax (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	if (argc!=3) {
		fprintf (stderr,"use: %s xrange yrange!",argv[0]);
		return 1;
	}
	use_global_bounds = 1;
	if (sscanf(argv[1],"%lf",&Xrange)!=1) {
		fprintf (stderr,"unable to determine Xrange from %s!\n",argv[1]);
		return 1;
	}
	if (sscanf(argv[2],"%lf",&Yrange)!=1) {
		fprintf (stderr,"unable to determine Yrange from %s!\n",argv[2]);
		return 1;
	}
	return TCL_OK;
}

int inkUnSetMinMax (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	if (argc!=1) {
		fprintf (stderr,"use: %s!",argv[0]);
		return 1;
	}
	use_global_bounds = 0;
	return TCL_OK;
}

static void inkDeleteSubsegments (InkWidgetResources *ink)
{
	InkSubSegments *sub;
	int i;

	sub = &ink->subsegments;
	if (sub->nsubsegments!=0) {
		for (i=0;i<sub->nsubsegments;i++) {
			ckfree(sub->m_color[i]);
			ckfree(sub->m_name[i]);
		}     
		ckfree(sub->m_width);
		ckfree(sub->m_start);
		ckfree(sub->m_end);
		ckfree(sub->m_color);
		ckfree(sub->m_name);
	}
	sub->nsubsegments = 0; 
}

static int inkTransformSubSegments (InkWidgetResources *ink)
{
	Tcl_Interp *interp;
	InkSubSegments *sub;
	int n,i,m_start,m_end,m_width;
	char *name,col[128],*string,*ptr,*ptr2,*newptr;

	inkDeleteSubsegments(ink);
	sub = &ink->subsegments;

	interp = ink->interp;
	string = ink->subsegmentsString;
	ptr = string;
	n = 0;
	while (1) {
		m_start = (int) strtol(ptr,&newptr,10);
		if (newptr==ptr)
			break;
		ptr = newptr;
		m_end = (int) strtol(ptr,&newptr,10);
		if (newptr==ptr) {
			fprintf (stderr,"error in subsegment string:\n%s\n",string);
			fprintf (stderr,"was looking for m_end\n");
			return TCL_ERROR;
		}
		newptr++;
		if ((ptr=strstr(newptr,"(<("))==NULL) {
			fprintf (stderr,"error in subsegment string:\n%s\n",string);
			fprintf (stderr,"expecting \"(<( label )>)\"!\n");
			return TCL_ERROR;
		}
		ptr += 3;
		if ((ptr2=strstr(newptr,")>)"))==NULL) {
			fprintf (stderr,"error in subsegment string:\n%s\n",string);
			fprintf (stderr,"expecting \"(<( label )>)\"!\n");
			return TCL_ERROR;
		}
		newptr = ptr2+4;
		name = (char *) malloc (ptr2-ptr+1);
		strncpy(name,ptr,ptr2-ptr);
		name[ptr2-ptr] = '\0';
		ptr2 --;
		i = 0;
		while (newptr[0]!=' ') {
			if (newptr[0]=='\0') {
				fprintf (stderr,"error in subsegment string:\n%s\n",string);
				fprintf (stderr,"when parsing color (%s) (%s)\n",newptr,col);
				return TCL_ERROR;
			}
			col[i++] = *newptr++;
		}
		col[i] = '\0';
		ptr = newptr;
		m_width = (int) strtol(ptr,&newptr,10);
		if (newptr==ptr) {
			fprintf (stderr,"error in subsegment string:\n%s\n",string);
			fprintf (stderr,"was looking for m_width, color = %s\n",col);
			return TCL_ERROR;
		}
		ptr = newptr;
		if (n==0) {
			sub->m_width  = (int *) ckalloc (sizeof(int));
			sub->m_start  = (int *) ckalloc (sizeof(int));
			sub->m_end    = (int *) ckalloc (sizeof(int));
			sub->m_color  = (Tk_Uid *) ckalloc (sizeof(Tk_Uid));
			sub->m_name   = (char **) ckalloc (sizeof(char *));
		}
		else {
			if ((sub->m_width=(int *)realloc(sub->m_width,(n+1)*sizeof(int)))==NULL) {
				fprintf (stderr,"unable to allocate %d m_width!\n",n+1);
				exit(1);
			}
			if ((sub->m_start=(int *)realloc(sub->m_start,(n+1)*sizeof(int)))==NULL) {
				fprintf (stderr,"unable to allocate %d m_start!\n",n+1);
				exit(1);
			}
			if ((sub->m_end=(int *)realloc(sub->m_end,(n+1)*sizeof(int)))==NULL) {
				fprintf (stderr,"unable to allocate %d m_end!\n",n+1);
				exit(1);
			}
			if ((sub->m_color=(char **)realloc(sub->m_color,(n+1)*sizeof(char *)))==NULL) {
				fprintf (stderr,"unable to allocate %d m_color!\n",n+1);
				exit(1);
			}
			if ((sub->m_name=(char **)realloc(sub->m_name,(n+1)*sizeof(char *)))==NULL) {
				fprintf (stderr,"unable to allocate %d m_name!\n",n+1);
				exit(1);
			}
		}
		sub->m_width[n]  = m_width;
		sub->m_start[n]  = m_start;
		sub->m_end[n]    = m_end;
		sub->m_color[n]  = (Tk_Uid) ck_strdup(col);
		sub->m_name[n]   = name;
		n++;
	}
	sub->nsubsegments = n;
	
	return TCL_OK;
}

static int inkTransformData (InkWidgetResources *ink)
{
	char *ptr,*newptr;
	int i,n = 0;
	double x,y,z;
	double xmin,xmax,ymin,ymax;
	double *Xin, *Yin, *Zin;
	double *tmp_va, *tmp_t, *tmp_x, *tmp_y, *tmp_z;

	if (ink->samples[0] != NULL) {
		ckfree(ink->samples[0]);
		ink->samples[0] = NULL;
	}
	if (ink->samples[1] != NULL) {
		ckfree(ink->samples[1]);
		ink->samples[1] = NULL;
	}
	if (ink->samples[2] != NULL) {
		ckfree(ink->samples[2]);
		ink->samples[2] = NULL;
	}

	ptr = newptr = ink->dataString;

	x = strtod(ptr,&newptr);
	while (ptr!=newptr) {
		ptr = newptr;
		y = strtod(ptr,&newptr);
		if (ptr==newptr) {
			fprintf (stderr,"unable to determine Y coordinate!!\n");
			exit(1);
		}

		ptr = newptr;
		z = strtod(ptr,&newptr);
		if (ptr==newptr) {
			fprintf (stderr,"unable to determine Z coordinate!!\n");
			exit(1);
		}
		if (n==0) {
			ink->samples[0] = (double *) ckalloc (sizeof(double));
			ink->samples[1] = (double *) ckalloc (sizeof(double));
			ink->samples[2] = (double *) ckalloc (sizeof(double));
		}
		else {
			if ((ink->samples[0]=(double *)realloc(ink->samples[0],(n+1)*sizeof(double)))==NULL) {
				fprintf (stderr,"unable to allocate space for %d samples!\n",n+1);
				exit(1);
			}
			if ((ink->samples[1]=(double *)realloc(ink->samples[1],(n+1)*sizeof(double)))==NULL) {
				fprintf (stderr,"unable to allocate space for %d samples!\n",n+1);
				exit(1);
			}
			if ((ink->samples[2]=(double *)realloc(ink->samples[2],(n+1)*sizeof(double)))==NULL) {
				fprintf (stderr,"unable to allocate space for %d samples!\n",n+1);
				exit(1);
			}
		}
		ink->samples[0][n] = x * ink->resol_mm;
		ink->samples[1][n] = y * ink->resol_mm;
		ink->samples[2][n] = z * ink->resol_g;
		n++;
		ptr = newptr;
		x = strtod(ptr,&newptr);
	}


	ink->nsamples = n;
	
/* There are n UNIPEN coordinates now available in ink->samples in UPSIGLIB format */

        Xin = ink->samples[0];
        Yin = ink->samples[1];
        Zin = ink->samples[2]; 

        if(ink->lowpass_nsmooth != 0 && ink->lowpass_nrepeat != 0) {
        	siglib_repeated_smooth(Xin, Yin, n
        	                    , ink->lowpass_nsmooth
        	                    , ink->lowpass_nrepeat);
        }                     
        
        if(ink->spatial_resampling) {
        	tmp_va = (double *) malloc(n * sizeof(double));
        	tmp_t = (double *) malloc(n * sizeof(double));
        	tmp_x = (double *) malloc(n * sizeof(double));
        	tmp_y = (double *) malloc(n * sizeof(double));
        	tmp_z = (double *) malloc(n * sizeof(double));
       	
        	siglib_spatial_z_sampler(Xin, Yin, Zin, n,0,n-1
        	                      ,tmp_x
        	                      ,tmp_y
        	                      ,tmp_z
        	                      ,n,tmp_va,tmp_t, ink->minpres);
        	                      
	        for (i=0; i<n; i++) {
			Xin[i] = tmp_x[i];
			Yin[i] = tmp_y[i];
			Zin[i] = tmp_z[i];
        	}
        	
        	free(tmp_va);
        	free(tmp_t);
        	free(tmp_x);
        	free(tmp_y);
        	free(tmp_z);
        
        }  

/* Determine spatial extrema */

	xmin = ymin = 9999999.0;
	xmax = ymax = -9999999.0;
	for(i = 0; i < n; ++i) {
		if (Zin[i]<ink->minpres && ink->_ignore_penups)
			continue;
		if (xmin>Xin[i]) xmin = Xin[i];
		if (ymin>Yin[i]) ymin = Yin[i];
		if (xmax<Xin[i]) xmax = Xin[i];
		if (ymax<Yin[i]) ymax = Yin[i];	
	}
	if (xmin==xmax) xmax++;
	if (ymin==ymax) ymax++;

	ink->xmin = xmin;
	ink->xmax = xmax;
	ink->ymin = ymin;
	ink->ymax = ymax;

/* Ready */
	return TCL_OK;
}

static XColor *InkGetCol (char *col, Tcl_Interp *interp, Tk_Window tkwin)
{
	int i;

	if (color_ptr==NULL) {
		color_ptr = (XColor **) ckalloc(ncolors*sizeof(XColor *));
		for (i=0;i<ncolors;i++) {
			color_ptr[i] = Tk_GetColor(interp, tkwin, color_names[i]);
		}
	}
	for (i=0;i<ncolors;i++)
		if (strcmp(col,color_names[i])==0) {
			return (color_ptr[i]);
		}
	fprintf (stderr,"I HAVE NOT ALLOCATED COLOR '%s'!\n",col);
	return NULL;
}

static void InkGetColGc (Display *display, GC *loe_gc, Tk_Uid col, int width
	,Tcl_Interp *interp, Tk_Window tkwin, XColor *bg)
{
	XGCValues gcValues;
	unsigned int mask;
	GC newgc;
	XColor *loe_col;

	loe_col = InkGetCol(col,interp,tkwin);
	gcValues.foreground = loe_col->pixel;
	gcValues.graphics_exposures = False;
	gcValues.line_width = width;
	gcValues.join_style = JoinRound;
	gcValues.cap_style = CapRound;
	mask = GCForeground|GCGraphicsExposures|GCLineWidth|GCJoinStyle|GCCapStyle;
	if (bg != NULL) {
		gcValues.background = bg->pixel;
		mask |= GCBackground;
	}
	newgc = Tk_GetGC(tkwin,mask,&gcValues);
	if (*loe_gc != None) {
		Tk_FreeGC(display, *loe_gc);
	}
	*loe_gc = newgc;
}

static void InkGetGc (Display *display, XColor **loe_col, GC *loe_gc, Tk_Uid col, int width
	,Tcl_Interp *interp, Tk_Window tkwin, XColor *bg)
{
	XColor *colorPtr;
	XGCValues gcValues;
	unsigned int mask;
	GC newgc;

	colorPtr = Tk_GetColor(interp, tkwin, col);
	if (colorPtr == NULL) {
		fprintf (stderr,"error while getting color '%s'\n",col);
	}
	gcValues.foreground = colorPtr->pixel;
	gcValues.graphics_exposures = False;
	gcValues.line_width = width;
	gcValues.join_style = JoinRound;
	gcValues.cap_style = CapRound;
	mask = GCForeground|GCGraphicsExposures|GCLineWidth|GCJoinStyle|GCCapStyle;
	if (bg != NULL) {
		gcValues.background = bg->pixel;
		mask |= GCBackground;
	}
	newgc = Tk_GetGC(tkwin,mask,&gcValues);
	if (*loe_gc != None) {
		Tk_FreeGC(display, *loe_gc);
	}
	if (*loe_col!=NULL) {
		Tk_FreeColor(*loe_col);
	}
	*loe_gc = newgc;
	*loe_col = colorPtr;
}

static void inkCreateColors (InkWidgetResources *ink)
{
	int i;
	InkSubSegments *subsegments;

	InkGetGc (ink->display,&ink->cursor_col,&ink->cursor_gc
		,ink->ccolor,ink->cwidth,ink->interp,ink->tkwin,ink->bg);
	InkGetGc (ink->display,&ink->sample_col,&ink->sample_gc
		,ink->scolor,ink->swidth,ink->interp,ink->tkwin,ink->bg);
	InkGetGc (ink->display,&ink->bd_col,&ink->bd_gc
		,"black",1,ink->interp,ink->tkwin,ink->bg);
	InkGetGc (ink->display,&ink->pd_col,&ink->pd_gc
		,ink->pd_color,ink->dwidth,ink->interp,ink->tkwin,ink->bg);
	InkGetGc (ink->display,&ink->pu_col,&ink->pu_gc
		,ink->pu_color,ink->uwidth,ink->interp,ink->tkwin,ink->bg);
	InkGetGc (ink->display,&ink->sel_col,&ink->sel_gc
		,ink->sel_color,ink->sel_width,ink->interp,ink->tkwin,ink->bg);
	XSetFont (ink->display,ink->sel_gc,SUBSEG_FONT);

	subsegments = &ink->subsegments;
	if (subsegments->nsubsegments!=0) {
		for (i=0;i<subsegments->nsubsegments&&i<ncolors;i++) {
			if (strcmp(subsegments->m_color[i],"selected_color")==0) {
				ink->m_gc[i] = None;
			} else {
				InkGetColGc (ink->display
					,&(ink->m_gc[i])
					,subsegments->m_color[i]
					,subsegments->m_width[i]
					,ink->interp,ink->tkwin,ink->bg);
				XSetFont (ink->display,ink->m_gc[i],SUBSEG_FONT);
			}
		}
	} else if (ink->_mark_penstreams) {
		for (i=0;i<ncolors;i++) {
			InkGetColGc (ink->display
				,&(ink->m_gc[i])
				,color_names[i]
				,ink->uwidth
				,ink->interp,ink->tkwin,ink->bg);
		}
	}
}

static void inkCreateFonts (InkWidgetResources *ink)
{
	int i;
	XColor *colorPtr;
	XGCValues gcValues;
	unsigned int mask;
	Font newfont;
	GC newgc;

	gcValues.graphics_exposures = False;
	gcValues.line_width = 1;
	gcValues.join_style = JoinRound;
	gcValues.cap_style = CapRound;
	mask = GCForeground|GCGraphicsExposures|GCLineWidth|GCJoinStyle|GCCapStyle;

	for (i=0;i<N_INK_FONTS;i++) {
		newgc = None;
		colorPtr = NULL;
		InkGetGc (ink->display,&colorPtr,&newgc
			,font_colors[i],1,ink->interp,ink->tkwin,ink->bg);
		fprintf (stderr,"loading font [%s]\n",font_names[i]);
		newfont = XLoadFont(ink->display,font_names[i]);
		XSetFont (ink->display,newgc,newfont);
		ink_fonts[i] = newfont;
		ink_fontgcs[i] = newgc;
	}
}

void inkDisplay (Drawable drawable, InkWidgetResources *ink)
{
	ink->drawable = drawable;
	if (fonts_reset) {
		fonts_reset = 0;
		inkCreateFonts (ink);
	}
}

void inkCreate (Tcl_Interp *interp, InkWidgetResources *ink)
{
	static int first = 1;

	memset ((char *)ink,0,sizeof(InkWidgetResources));

	ink->interp = interp;               /* given by INKMASTER */
	ink->tkwin = Tk_MainWindow(interp); /* interp contains everything */
	if (ink->tkwin==NULL) {
		fprintf (stderr,"there is no main window attached to interp, BAILING OUT\n");
		exit(1);
	}
	ink->display = Tk_Display(ink->tkwin);
	ink->request_cursor_x = -1;
	ink->request_cursor_y = -1;
	ink->fsample_of_data = fsample_of_data;
	ink->resol_mm = resol_mm;
	ink->resol_g = resol_g;

	if (first||fonts_reset) {
		fonts_reset = first = 0;
		inkCreateFonts (ink);
	}
}


int inkConfigure (InkWidgetResources *ink)
{
	XColor *colorPtr;

	if (ink->fgUid[0] != '\0') {
		colorPtr = Tk_GetColor(ink->interp, ink->tkwin, ink->fgUid);
		if (colorPtr == NULL)
			return TCL_ERROR;
	}
	else
		colorPtr = NULL;
	if (ink->fg!=NULL)
		Tk_FreeColor(ink->fg);
	ink->fg = colorPtr;
	if (ink->bgUid[0] != '\0') {
		colorPtr = Tk_GetColor(ink->interp, ink->tkwin, ink->bgUid);
		if (colorPtr == NULL)
			return TCL_ERROR;
	}
	else
		colorPtr = NULL;
	if (ink->bg!=NULL)
		Tk_FreeColor(ink->bg);
	ink->bg = colorPtr;

	if (ink->dataString != NULL) {
		if (inkTransformData(ink)!=TCL_OK) {
				return TCL_ERROR;
		}
	}
	if (ink->subsegmentsString != NULL && ink->_show_subsegs) {
		if (inkTransformSubSegments(ink)!=TCL_OK) {
			return TCL_ERROR;
		}
	} else {
		inkDeleteSubsegments(ink);
	}
	inkCreateColors(ink);
	return TCL_OK;
}

void inkDelete (InkWidgetResources *ink)
{
	InkSubSegments *sub;
	int i;

	if (ink->fg!=NULL) {
		Tk_FreeColor(ink->fg);
	}
	if (ink->bg!=NULL) {
		Tk_FreeColor(ink->bg);
	}
	if (ink->pu_gc != None) {
		Tk_FreeGC(ink->display, ink->pu_gc);
	}
	if (ink->bd_gc != None) {
		Tk_FreeGC(ink->display, ink->bd_gc);
	}
	if (ink->pd_gc != None) {
		Tk_FreeGC(ink->display, ink->pd_gc);
	}
	if (ink->sample_gc != None) {
		Tk_FreeGC(ink->display, ink->sample_gc);
	}
	for (i=0;i<MAX_SUBGC;i++) {
		if ( (ink->m_gc[i] != None) && (ink->m_gc[i] != ink->sel_gc ) ) {
			Tk_FreeGC(ink->display, ink->m_gc[i]);
		}
	}

	sub = &ink->subsegments;
	if (sub->nsubsegments!=0) {
		for (i=0;i<sub->nsubsegments;i++) {
			ckfree(sub->m_color[i]);
			ckfree(sub->m_name[i]);
		}
		ckfree(sub->m_width);
		ckfree(sub->m_start);
		ckfree(sub->m_end);
		ckfree(sub->m_color);
		ckfree(sub->m_name);
	}
	sub->nsubsegments = 0;

	if (ink->samples[0] != NULL) {
		ckfree(ink->samples[0]);
		ink->samples[0] = NULL;
	}  
	if (ink->samples[1] != NULL) {
		ckfree(ink->samples[1]);
		ink->samples[1] = NULL;
	}  
	if (ink->samples[2] != NULL) {
		ckfree(ink->samples[2]);
		ink->samples[2] = NULL;
	}  
	ink->nsamples = 0;
}

void DrawCursor(InkWidgetResources *ink, int x, int y
	, ContainerBox *box, int draw_text)
{
	int x0,x1,y0,y1;
	char cursor_buf[128];

	x0 = x;
	y0 = y - 9;
	if (y0<0) y0=0;
	y1 = y0 + 18;
	XDrawLine(ink->display,ink->drawable,ink->cursor_gc,x0,y0,x0,y1);
	y0 = y;
	x0 -= 9;
	if (x0<0) x0=0;
	x1 = x0 + 18;
	XDrawLine(ink->display,ink->drawable,ink->cursor_gc,x0,y0,x1,y0);
	y0 -= 9;
	XDrawArc(ink->display,ink->drawable,ink->cursor_gc
		,x0+5,y0+5,8,8,0,360*64);
	XDrawArc(ink->display,ink->drawable,ink->cursor_gc
		,x0,y0,18,18,0,360*64);

	if (draw_text) {
		sprintf (cursor_buf,"%d",ink->cursor);
		x = BOX_XORG(box)+4;
		y = BOX_YORG(box)+14;
		XDrawString(ink->display,ink->drawable,GRAPHICS_GC
			,x,y,cursor_buf,strlen(cursor_buf));
	}
}
