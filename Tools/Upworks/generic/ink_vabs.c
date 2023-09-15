#include <stdio.h>
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif
#include <string.h>
#include <math.h>
#include <tk.h>

#include "ink_master.h"
#include "ink_widget.h"
#include "ink_vabs.h"
#include "siglib.h"

#define CHAR_XOFF  4
#define CHAR_YOFF  5

#define XPOINT(x) ((short)(x+xorg))
#define YPOINT(y) ((short)(y+yorg))

double pretty_scale_up(xx)
double xx;
{
	double x, e, r, ie, near, try, s;
	
	if(xx > 0.) {
		s = 1;
		x = xx;
	} else if ( xx < 0.) {
		s = -1;
		x = -xx;
	} else {
		return(0.0);
	}
	
	e = log10(x);
	ie = (double) ((int) e);
	r = e - ie;
	near = (double) ((int) (pow(10., r) + 0.5));
	try = pow(10., ie) * near;
	if(try < x) {
		try = pow(10., ie) * (near + 0.5);
	}
	try = try * s;
        return(try);
}

double pretty_scale_down(xx)
double xx;
{
	double x, e, r, ie, near, try, s;
	
	if(xx > 0.) {
		s = 1;
		x = xx;
	} else  if(xx < 0.) {
		s = -1;
		x = -xx;
	} else {
		return(0.0);
	}
	
	e = log10(x);
	ie = (double) ((int) e);
	r = e - ie;
	near = (double) ((int) (pow(10., r) + 0.5));
	try = pow(10., ie) * near;
	if(try > x) {
		try = pow(10., ie) * (near - 0.5);
	}
	try = try * s;
        return(try);
}



void make_sig_value(Drawable drawable
      ,InkWidgetResources *ink
      ,Font a_font
      ,GC a_gc
      ,double xorg
      ,double yorg
      ,double width
      ,double height
      ,double y)
{
	XCharStruct c_struct;
	char sig_value[20];
	int dir, ascent, descent;

	sprintf(sig_value,"%+9.2f", y);
	
	XQueryTextExtents(ink->display,a_font,sig_value,strlen(sig_value)
					,&dir,&ascent,&descent,&c_struct);
	XDrawString(ink->display,drawable,a_gc
	,xorg + width - c_struct.width - CHAR_XOFF
	,yorg + c_struct.ascent + CHAR_YOFF
	,sig_value,strlen(sig_value));
}

void make_sig_title(Drawable drawable
      ,InkWidgetResources *ink
      ,Font a_font
      ,GC a_gc
      ,double xorg
      ,double yorg
      ,double width
      ,double height
      ,char *signam)
{
	XCharStruct c_struct;
	char sig_title[20];
	int dir, ascent, descent;
	double x,y;

	sprintf(sig_title, "%s %s"
	      , siglib_sig_title(signam)
	      , siglib_sig_units(signam));
	
	XQueryTextExtents(ink->display,a_font,sig_title,strlen(sig_title)
					,&dir,&ascent,&descent,&c_struct);
				
	if(c_struct.width < 0.25 * width) {	
	    x = xorg + CHAR_XOFF;
	    y = yorg + c_struct.ascent + CHAR_YOFF;
	    XDrawString(ink->display,drawable,a_gc
	                ,x,y, sig_title,strlen(sig_title));
	}
}

void make_annot_yaxis(Drawable drawable
      ,InkWidgetResources *ink
      ,VabsResources *vabs
      ,Font a_font
      ,GC a_gc
      ,double xorg
      ,double yorg
      ,double width
      ,double height
      ,double xpos_yaxis
      ,char *signam
      ,int    nsteps)
{
    XCharStruct c_struct;
    char yax_annot[20];
    int dir, ascent, descent;
    int i;
    double d, x, y, dy, ydata_tick;
    int ix1,iy1,ix2,iy2;
    double pretty_min, pretty_max;
    

    pretty_min = pretty_scale_up(vabs->ymin);
    pretty_max = pretty_scale_down(vabs->ymax);
/* WAS:
    pretty_min = vabs->ymin;
    pretty_max = vabs->ymax;
    */
    
#define FP(x) (vabs->a_ymin + ((vabs->a_ymax - vabs->a_ymin) * (x - vabs->ymin))/(vabs->ymax - vabs->ymin))
	
    d = (pretty_max - pretty_min) / nsteps;
    dy = d * vabs->a_dy;

    for(i = 0; i <= nsteps; ++i) {	
    
        /* the Y-annot. label */ 
  
        ydata_tick =  pretty_min + (double) i * d;
        if(ydata_tick < 1000. && ydata_tick > -1000.) {
        	sprintf(yax_annot, "%.1f", ydata_tick);
        } else {
		sprintf(yax_annot, "%.1g", ydata_tick);
	}
	
	XQueryTextExtents(ink->display,a_font,yax_annot,strlen(yax_annot)
					,&dir,&ascent,&descent,&c_struct);
				
	x = XPOINT(xpos_yaxis) - c_struct.width - CHAR_XOFF - 2;
	y = YPOINT(FP(pretty_min)) - (double) i * dy +  c_struct.ascent/2;	
	
	if(c_struct.width < 0.25 * width) {	
	    XDrawString(ink->display,drawable,a_gc,x,y
	                ,yax_annot,strlen(yax_annot));
	}
	
	/* the ytick */
	
	x = XPOINT(xpos_yaxis);
	y = YPOINT(FP(pretty_min)) - (double) i * dy;	
	ix1 = x;
	iy1 = y;
	ix2 = x - 3;
	iy2 = y;
	XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
	
    }

}

void make_annot_xaxis(Drawable drawable
      ,InkWidgetResources *ink
      ,VabsResources *vabs
      ,Font a_font
      ,GC a_gc
      ,double xorg
      ,double yorg
      ,double width
      ,double height
      ,double ypos_xaxis
      ,char *signam)
{
    XCharStruct c_struct;
    char xax_annot[20];
    int dir, ascent, descent;
    int i;
    double x, y, t, ut, x_rightmost;
    int ix1,iy1,ix2,iy2;
    char *eop;
	
    t = vabs->a_xmin;
    ut = vabs->xmin;
    x_rightmost = -99999.;
    i = 0;
    while(t < vabs->a_xmax) {
    	
        /* the X-annot. label */

	sprintf(xax_annot, "%.0f", ut);
	eop = strchr(xax_annot,' ');
	if(eop != NULL) {
		*eop = (char) 0;
	}
	
	XQueryTextExtents(ink->display,a_font,xax_annot,strlen(xax_annot)
					,&dir,&ascent,&descent,&c_struct);
				
	x = XPOINT(t) - c_struct.width/2.;
	y = YPOINT(ypos_xaxis) + c_struct.ascent + CHAR_YOFF + 2;
	
	if(i > 0 && c_struct.width < 0.25 * width) {	
	  if(x > x_rightmost) {
	    XDrawString(ink->display,drawable,a_gc,x,y
	                ,xax_annot,strlen(xax_annot));
	    x_rightmost = x + c_struct.width + CHAR_XOFF;
	  }
	}

        /* the xtick */

	ix1 = XPOINT(t);
	iy1 = YPOINT(ypos_xaxis);
	ix2 = XPOINT(t);
	iy2 = YPOINT(ypos_xaxis) + 3;
	XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
	t = t + 10.* vabs->a_dx; /* steps of 10 samples */
	ut = ut + 10.;
	++i;
    }

}

void make_sig_axes(Drawable drawable
      ,InkWidgetResources *ink
      ,Font a_font
      ,VabsResources *vabs
      ,double xorg
      ,double yorg
      ,double width
      ,double height
      ,char *signam
      ,int x_annot)
{
	int ix1,iy1,ix2,iy2;
	double xpos_yaxis, ypos_xaxis;

	if(strcmp(signam,"VA") == 0 || strcmp(signam,"AA") == 0 ) { 
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_yzero);
		ix2 = XPOINT(vabs->a_xmax);
		iy2 = YPOINT(vabs->a_yzero);
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		                        
		                       
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_yzero);
		ix2 = XPOINT(vabs->a_xmin);
		iy2 = YPOINT(vabs->a_ymax);
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		xpos_yaxis = vabs->a_xmin;
		ypos_xaxis = vabs->a_yzero;

        } else if(strcmp(signam,"P") == 0) {
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_yzero);
		ix2 = XPOINT(vabs->a_xmax);
		iy2 = YPOINT(vabs->a_yzero);
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		                        
		                       
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_ymin);
		ix2 = XPOINT(vabs->a_xmin);
		iy2 = YPOINT(vabs->a_ymax);
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		xpos_yaxis = vabs->a_xmin;
		ypos_xaxis = vabs->a_yzero;

        } else if(strcmp(signam,"DP") == 0 ||
                  strcmp(signam,"DPHI") == 0 || 
                  strcmp(signam,"PHI") == 0 || 
                  strcmp(signam,"VX") == 0 || 
                  strcmp(signam,"VY") == 0 || 
                  strcmp(signam,"SINF") == 0 || 
                  strcmp(signam,"COSF") == 0 || 
                  strcmp(signam,"CURV") == 0
               ) {
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_yzero);
		ix2 = XPOINT(vabs->a_xmax);
		iy2 = YPOINT(vabs->a_yzero);
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		                        
		                       
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_ymin);
		ix2 = XPOINT(vabs->a_xmin);
		iy2 = YPOINT(vabs->a_ymax);
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		xpos_yaxis = vabs->a_xmin;
		ypos_xaxis = vabs->a_yzero;

	} else {
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_ymin);
		ix2 = XPOINT(vabs->a_xmax);
		iy2 = iy1;
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		                        
		                       
		ix1 = XPOINT(vabs->a_xmin);
		iy1 = YPOINT(vabs->a_ymin);
		ix2 = ix1;
		iy2 = YPOINT(vabs->a_ymax);
		XDrawLine(ink->display,drawable, ink->pd_gc,ix1,iy1,ix2,iy2);
		xpos_yaxis = vabs->a_xmin;
		ypos_xaxis = vabs->a_ymin;
	} 
	
	/* xaxis ticks and annot */
	
	if(x_annot) {
           make_annot_xaxis(drawable, ink, vabs, GRAPHICS_FONT, GRAPHICS_GC
                                    , xorg, yorg, width, height, ypos_xaxis
                                    , signam);
        }
	
        /* yaxis ticks and annot */

#define NSTEPS 3
        make_annot_yaxis(drawable, ink, vabs, GRAPHICS_FONT, GRAPHICS_GC
                                 , xorg, yorg, width, height, xpos_yaxis
                                 , signam, NSTEPS);

}

/* something to scale points */

static void vabs_scale_points (int n, double *x, double *y, double*z,
	VabsResources *vabs, InkWidgetResources *ink,
	double w, double h, double xoffset, double yoffset, 
	double margin, int same_scale, double minpres, char *signam)
{
	double xmin, ymin, xmax, ymax;
	double xrange;
	double yrange;
	double xoff,yoff;
	double height = h;
	int i;
	
	if (same_scale) {
		xrange = Xrange;
		yrange = Yrange;
		ymin   = vabs->ymin;
	} else {
		xmin = 0.0;
		xmax = (double) n - 1;
		
		if(strcmp(signam,"VA") == 0) {
			ymin = 0.0; 
		   ymax = y[0];
		
	           /* Option: ignore_penups
	              The velocity around penups is considered
	              unreliable, since there are UNIPEN files
	              in which the PENUP trajectory is unknown.
	              Because the Lagrange differentiator has a 
	              window of five points, we take a zone of
	              three samples in which the Va(t) is not
	              considered in ymin/ymax determination and
	              the value is not plotted visibly (but _is_
	              followed by the cursor as long as it is
	              within vertical panel limits).
	           */
	              
		   for (i=1;i<(n-1);i++) {
		      if(ink->_ignore_penups) {
			if((y[i] > ymax)  
			   && (z[i-1] > minpres)  
			   && (z[i] > minpres)  
			   && (z[i+1] > minpres)  
			) {
				if(i >= 2 && i <= n-2) {
				    if(  z[i-2] > minpres 
				      && z[i+2] > minpres) {
				        ymax = y[i];
				    }
				} else {
					ymax = y[i];
				}
			}
		      } else {
		        if(y[i] > ymax) {
		           ymax = y[i];
		        }
		      }
		   }
		   xrange = (double) n;
		   yrange = ymax - ymin;

		} else {
			ymin = y[0]; 
			ymax = y[0];
		
		   for (i=1;i<n;i++) {
			if(y[i] > ymax) {
				ymax = y[i];
			}
			if(y[i] < ymin) {
				ymin = y[i];
			}
		   }
		   xrange = (double) n;
		   yrange = ymax - ymin;
		}

		vabs->xmin = xmin;
		vabs->xmax = xmax;
		vabs->ymin = ymin;
		vabs->ymax = ymax;

	}
	if(xrange <= 0.0) xrange = 1.0;
	if(yrange <= 0.0) yrange = 0.01;

	xoff = w/2 + 2.*margin; /* %%% */
	yoff = h/2;
	
	w -= 4. * margin;
	h -= 3. * margin;

	for (i=0;i<n;i++) {
		x[i] = xoff + (((double) i)/xrange-.5)*w;
		y[i] = height - (yoff+((y[i]-ymin)/yrange-.5)*h);
	}
	
	vabs->a_xmin = xoff + (((double) 0)/xrange-.5)*w;
	vabs->a_xmax = xoff + (((double) (n-1))/xrange-.5)*w;
	vabs->a_dx = w/xrange;
	vabs->a_ymin = height - (yoff+(0.-.5)*h);
	vabs->a_ymax = height - (yoff+(1.-.5)*h);
	vabs->a_dy = h/yrange;
	vabs->a_xzero = xoff + ((0.0)/xrange-.5)*w;
	vabs->a_yzero = height - (yoff+(-ymin/yrange-.5)*h);
	
}

static int vabs_compress_line (XPoint *points, int n)
{
	int i,j;

	j = 0;
	for (i=0;i<n;i++) {
		if (points[j].x==points[i].x && points[j].y==points[i].y) {
			points[j].x = points[i].x;
			points[j].y = points[i].y;
		}
		else {
			j++;
			points[j].x = points[i].x;
			points[j].y = points[i].y;
		}
	}
	return j+1;
}

static void vabs_copy_points (double fsamp, double *Xi, double *Yi, double *Zi, int n
                           ,VabsResources *vabs, char *signam, double *Ycopy)
{
	int i;
	char *err;

	if (vabs->bufsize==0) {
		vabs->sig_taxis = (double *) ckalloc(n*sizeof(double));
		vabs->sig_faxis = (double *) ckalloc(n*sizeof(double));
		vabs->bufsize = n;
		
	} else if (vabs->bufsize<n) {
		vabs->sig_taxis = (double *) realloc(vabs->sig_taxis,n*sizeof(double));
		vabs->sig_faxis = (double *) realloc(vabs->sig_faxis,n*sizeof(double));
		vabs->bufsize = n;
	}

/*
 static char *stypes[] = {       "SX"   x displacement
                                ,"SY"   y displacement
                                ,"SC"   cumulative displacement
                                ,"P"    pressure
                                ,"VX"   horizontal velocity
                                ,"VY"   vertical velocity
                                ,"VA"   absolute (tangential) velocity
                                ,"DP"   dP/dt
                                ,"AX"   horizontal acceleration
                                ,"AY"   vertical  acceleration
                                ,"AA"   absolute  acceleration
                                ,"PHI"  running angle
                                ,"DPHI" angular speed
                                ,"SINF" sine of running angle
                                ,"COSF" cosine of running angle
                                ,"CURV" curvature
*/


	err = siglib_get_signal(Xi,Yi,Zi, signam
	                             , vabs->sig_faxis, n, fsamp);
	   
	if(err != NULL) {
		fprintf(stderr,"%s\n", err);
		/* Tcl_AppendResult(vabs->interp,err,NULL); */
		for (i=0;i<n;i++) {
			 vabs->sig_faxis[i] = ((double) (i % 2) - 0.5) * 2.;
		}
	}    
	                            
	for (i=0;i<n;i++) {
		vabs->sig_taxis[i] = (double) i;
		Ycopy[i] = vabs->sig_faxis[i];
	}

	
}


void USE_THE_DISPLAY_FROM_HWR_BUT_WITH_DIFFERENT_SCALING
(InkWidgetResources *ink, VabsResources *vabs)
{
	Display *disp;
	Drawable drwbl;
	int i,color,s,ok;
	int x0,y0,x1;
	int c_left,xmin,xmax;
	int n,nsamples;
	int mstart,mend,swidth,text_offset;
	int _show_sample,_only_subsegs, _mark_penstreams;
	XPoint *points;
	InkSubSegments *subsegments;
	XCharStruct c_struct;
	ContainerBox *box = &vabs->box;
	double xorg,yorg,width,height;
	int same_scale = ink->_use_same_scale;
	double *X, *Y, *Z, minpres;
	double *SIG;
	int do_ignore_penups;
	char *signam;
	int x_annot;
	GC m_gc;

	signam   = box->signam;
	x_annot  = box->x_annot;

	disp     = ink->display;
	drwbl    = ink->drawable;
	nsamples = ink->nsamples;

	points       = (XPoint *) ckalloc (2*nsamples*sizeof(XPoint));
	SIG          = (double *) ckalloc (nsamples*sizeof(double));
	text_offset  = ink->text_offset ;

	xorg = TBOX_XORG(box);
	yorg = TBOX_YORG(box);
	width = TBOX_WIDTH(box);
	height = TBOX_HEIGHT(box);
	
	do_ignore_penups = ink->_ignore_penups &&
	                      ((strcmp(signam,"VA") == 0)
	                    || (strcmp(signam,"AA") == 0) );
	
/* signal title */

        make_sig_title(drwbl,ink,GRAPHICS_FONT,GRAPHICS_GC
                               ,xorg,yorg,width,height,signam);
                               
        

/* box around the signal subwindow */

	if (box->showbox) {
		XDrawRectangle(disp,drwbl,ink->bd_gc
			,xorg,yorg,width,height);
	}

/*old
	if (!ink->_remember_bounds) {
		vabs->xmin = ink->xmin;
		vabs->xmax = ink->xmax;
		vabs->ymin = ink->ymin;
		vabs->ymax = ink->ymax;
	}
*/
	
	vabs_copy_points(ink->fsample_of_data
	           ,ink->samples[0]
	           ,ink->samples[1]
	           ,ink->samples[2]
	           ,nsamples,vabs,signam,SIG);
	           
	X = vabs->sig_taxis;
	Y = vabs->sig_faxis;
	Z = ink->samples[2];
	
	minpres = ink->minpres;
	vabs_scale_points (nsamples,X,Y,Z,vabs,ink
		,(double)width,(double)height
		,(double)width/2,(double)height/2
		,ink->margin*width,use_global_bounds&&same_scale
		,minpres,signam);

	_show_sample  = ink->_show_sample;
	_only_subsegs = ink->_only_subsegs;
	_mark_penstreams = ink->_mark_penstreams;

	swidth = ink->swidth;

/* draw the axes */

        make_sig_axes(drwbl,ink,GRAPHICS_FONT,vabs
                              ,xorg,yorg,width,height,signam,x_annot);



#define S_UNKNOWN -1
#define S_PENDOWN  1
#define S_PENUP    0

	if (_mark_penstreams) {
		color = 0;
		s     = S_UNKNOWN;
		n     = 0;
		for (i=0;i<nsamples;i++) {
			x0 = XPOINT(X[i]);
			y0 = YPOINT(Y[i]);

			switch (s) {

				case S_UNKNOWN: /* PENLIFT UNKNOWN */

					s = (Z[i] > minpres);
					if (s==S_PENDOWN) { /* the first is a pen down, adminstrate it */
						n = 1;
						points[0].x = x0;
						points[0].y = y0;
					} else { /* the first is a pen up, draw it */
						if (swidth>0)
							XDrawArc(disp,drwbl,ink->m_gc[color%ncolors]
								,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
					}
					break;

				case S_PENUP: /* PEN_LIFT WAS UP */

					if (Z[i]<=minpres) { /* was up, is up, draw it */
						if (swidth>0)
							XDrawArc(disp,drwbl,ink->m_gc[color%ncolors]
								,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
					} else { /* going down, was up, administrate the first down */
						s = 1;

						points[0].x = x0;
						points[0].y = y0;
						n = 0;
						
						color++;
					}
					break;

				case S_PENDOWN: /* PENLIFT WAS DOWN */

					if (Z[i]<=minpres) { /* was down, going up, flush downs */
						/* disabled LS 
						   if (swidth>0)
							XDrawArc(disp,drwbl,ink->m_gc[color%ncolors]
								,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
						*/
						color++;
						s = S_PENUP;
						if (n>1) {
						/*				
							if (Z[i]!=-1) {
								points[n].x = x0;
								points[n].y = y0;
								n++;
							}
						*/
							n = vabs_compress_line (points,n);
							if(do_ignore_penups) {
							   n = n - 6;
							   if (n>1) {
								if (_only_subsegs)
									XDrawLines(disp,drwbl,ink->pd_gc,&points[3],n,CoordModeOrigin);
								XDrawLines(disp,drwbl,ink->m_gc[color%ncolors]
								,&points[3],n,CoordModeOrigin);
								color++;
							   }
							} else  {
							   if (n>1) {
								if (_only_subsegs)
									XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
								XDrawLines(disp,drwbl,ink->m_gc[color%ncolors]
								,points,n,CoordModeOrigin);
								color++;
							   }
							}
						}
						n = 0;
					} else { /* was down, is down, administrate it */
						points[n].x = x0;
						points[n].y = y0;
						n++;
					}
					break;
			}

			if (n>0) {
				if (s==S_PENDOWN) {
					n = vabs_compress_line (points,n);

                                        ok = 1;
					if(do_ignore_penups ) {
					      
					   if(i < nsamples-3) {
						if(Z[i+1] <= minpres) ok = 0;
						if(Z[i+2] <= minpres) ok = 0;
						if(Z[i+3] <= minpres) ok = 0;
	
					   }
					}
					if(ok) {					    
					   if (n>1) {
						if (_only_subsegs) {
							XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
						}
						XDrawLines(disp,drwbl,ink->m_gc[color%ncolors]
						,points,n,CoordModeOrigin);
					   }
					}
				}
			}
		

		}
        } else {

	  if(!_only_subsegs && ink->dwidth > 0) {
	    if(strcmp(signam,"P") == 0) {
	    	n = 0;
		x0 = XPOINT(X[n]);
		y0 = YPOINT(Y[n]);
		for (i=1; i<nsamples; i++) {
			if(Y[i] != Y[i-1]) {
			    x0 = XPOINT(X[i-1]);
			    y0 = YPOINT(Y[i]);
			    points[n].x = x0;
			    points[n].y = y0;
			    ++n;
			}
			x0 = XPOINT(X[i]);
			y0 = YPOINT(Y[i]);
			points[n].x = x0;
			points[n].y = y0;
			++n;
		}
		XDrawLines(disp,drwbl,ink->pd_gc
				       ,points,n,CoordModeOrigin);


	    } else { /* other signals */
	    
		n = 0;
		for (i=0;i<nsamples;i++) {
			x0 = XPOINT(X[i]);
			y0 = YPOINT(Y[i]);
			if (Z[i]>=minpres) {
				points[n].x = x0;
				points[n].y = y0;
				n++;
			} else if (n>0) {
/*
				points[n].x = x0;
				points[n].y = y0;
				n++;
*/
				n = vabs_compress_line (points,n);
				if(do_ignore_penups ) {
				     
				   n = n - 6;
				   if (n>1)
					XDrawLines(disp,drwbl,ink->pd_gc
					,&points[3],n,CoordModeOrigin);
				} else {
				   if (n>1)
					XDrawLines(disp,drwbl,ink->pd_gc
					,points,n,CoordModeOrigin);
				}
				n = 0;
			}
		}
		if (n>1) {
			n = vabs_compress_line (points,n);
			if(do_ignore_penups ) {
			     
			    n = n - 6;
			    if (n>1)
				XDrawLines(disp,drwbl,ink->pd_gc
				       ,&points[3],n,CoordModeOrigin);
			    
			} else {
			    if (n>1)
				XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
			}
		}
	   }
	  }

	  if ((swidth=ink->uwidth)>0) {
		for (i=0;i<nsamples;i++) {
			x0 = XPOINT(X[i]);
			y0 = YPOINT(Y[i]);
			if (Z[i]<minpres && Z[i]>=0) {
				XDrawArc(disp,drwbl,ink->pu_gc,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
			}
		}
	  }

	  swidth       = ink->swidth;
	  subsegments  = &(ink->subsegments);
	  if (subsegments->nsubsegments!=0) {
		for (s=0;s<subsegments->nsubsegments;s++) {
			if (subsegments->m_width[s]<=0)
				continue;
			mstart = subsegments->m_start[s];
			mend   = subsegments->m_end[s];
			if (strcmp(subsegments->m_color[s],"selected_color")==0) {
				m_gc   = ink->sel_gc;
			} else {
				m_gc   = ink->m_gc[s%ncolors];
			}
			n = 0;
			xmin =  999999;
			xmax = -999999;
			for (i=mstart;i<=mend;i++) {
				if (i>=nsamples) {
#ifdef DEBUG_NORMAL
					fprintf (stderr,"error: marker end [%d] must be <= nsamples [%d]!\n",mend,nsamples);
#endif
					break;
				}
				if (i<0) {
#ifdef DEBUG_NORMAL
					fprintf (stderr,"error: marker start [%d] must be >= [0]!\n",mend);
#endif
					break;
				}
				x0 = (short) X[i];
				y0 = YPOINT(Y[i]);
				if (xmin>x0) xmin = x0;
				if (xmax<x0) xmax = x0;
				x0 = XPOINT(X[i]);
				if (Z[i]<=minpres) {
					if (n>1) {
						n = vabs_compress_line (points,n);
						if (n>1) {
							if (_only_subsegs)
								XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
							XDrawLines(disp,drwbl,m_gc,points,n,CoordModeOrigin);
						}
					}
					if (swidth>0) {
						XDrawArc(disp,drwbl,m_gc,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
					}
					n = 0;
				} else {
					points[n].x = x0;
					points[n].y = y0;
					n++;
				}
			}
			if (n>1) {
				n = vabs_compress_line (points,n);
				if (_only_subsegs)
					if (n>1)
						XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
				if (n>1)
					XDrawLines(disp,drwbl,m_gc,points,n,CoordModeOrigin);
			}
			
			if(text_offset >= 0) {
			   if (mend-mstart>=1) {
				XQueryTextExtents(disp,LABEL_FONT,subsegments->m_name[s],strlen(subsegments->m_name[s])
					,&x0,&x0,&x0,&c_struct);
				c_left = c_struct.width/2;
				XDrawString(disp,drwbl,LABEL_GC
					,XPOINT((xmax-xmin)/2+xmin-c_left),YPOINT(height-text_offset)
					,subsegments->m_name[s],strlen(subsegments->m_name[s]));
			   }
			}
		}
	  }

	  if (_show_sample&&ink->swidth>0) {
		for (i=0;i<nsamples;i++) {
			x0 = XPOINT(X[i]);
			y0 = YPOINT(Y[i]);
			XDrawArc(disp,drwbl,ink->sample_gc,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
		}
	  }

	  if(text_offset >= 0) {
	     if (ink->labelString != NULL && subsegments->nsubsegments == 0) {
		xmin =  999999;
		xmax = -999999;
		for (i=0;i<nsamples;i++) {
			x0 = (short) X[i];
			if (xmin>x0) xmin = x0;
			if (xmax<x0) xmax = x0;
		}
		x0 = strlen(ink->labelString);
		XQueryTextExtents(disp,GRAPHICS_FONT
			,ink->labelString,x0
			,&x1,&x1,&x1,&c_struct);
		c_left = c_struct.width/2;
		XDrawString(disp,drwbl,GRAPHICS_GC
			,XPOINT((xmax-xmin)/2+xmin-c_left),YPOINT(height-text_offset)
			,ink->labelString,x0);
	     }
	  }
	}
	
	if (ink->cursor>=0) {
		if (ink->cursor>=nsamples) ink->cursor = 0;
		/* vertical cursor line */
		x0 = XPOINT(X[ink->cursor]);
		y0 = YPOINT(Y[ink->cursor]);
		DrawCursor(ink,x0,y0,box,0);

		make_sig_value(drwbl,ink,GRAPHICS_FONT,GRAPHICS_GC
                        ,xorg,yorg,width,height,SIG[ink->cursor]);

	}
	ckfree(points);
	ckfree(SIG);
}


void vabsCreate (VabsResources *vabs, Tcl_Interp *interp)
{
	memset ((char *)vabs,0,sizeof(VabsResources));
	vabs->interp = interp;
}

void vabsDisplay (InkWidgetResources *ink, VabsResources *vabs)
{
	ContainerBox *box = &vabs->box;
	double xorg,yorg,width,height;
	char *signam;
	
	signam = box->signam;
	
	if(strncmp(signam,"NO",2) == 0) return;

#define XPOINT(x) ((short)(x+xorg))
#define YPOINT(y) ((short)(y+yorg))

	xorg = TBOX_XORG(box);
	yorg = TBOX_YORG(box);
	width = TBOX_WIDTH(box);
	height = TBOX_HEIGHT(box);
	if (box->showbox) {
		XDrawRectangle(ink->display,ink->drawable,ink->bd_gc
			,xorg,yorg,width,height);
	}
	USE_THE_DISPLAY_FROM_HWR_BUT_WITH_DIFFERENT_SCALING(ink,vabs);
}

void vabsDelete (VabsResources *vabs)
{
	if (vabs->n!=0) {
		ckfree(vabs->sig_taxis);
		ckfree(vabs->sig_faxis);
	}
	vabs->n = 0;
	vabs->sig_taxis = NULL;
	vabs->sig_faxis = NULL;
}

