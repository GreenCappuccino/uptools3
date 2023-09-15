#include <stdio.h>
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif
#include <string.h>
#include <tk.h>

#include "ink_master.h"
#include "ink_widget.h"
#include "ink_hwr.h"

void hwrCreate (HwrResources *hwr)
{
	memset ((char *)hwr,0,sizeof(HwrResources));
	hwr->xmin = hwr->xmax = hwr->ymin = hwr->ymax = -1;
}

void hwrDelete (HwrResources *hwr)
{
	if (hwr->bufsize!=0) {
		ckfree(hwr->X);
		ckfree(hwr->Y);
	}
	hwr->bufsize = 0;
	hwr->X = NULL;
	hwr->Y = NULL;
}


/* something to scale points */

#define rescaled(a,r,f,min,offset,s) (offset + ((a-min)/r-.5)*f*s)
#define my_rescaled(a,r,f,min,offset,s) (((a-min)/r-.5)*f*s)
#define XS(x) (xoffset+((x-xmin)/xrange-.5)*xfactor*w)
#define YS(y) (height-yoffset-((y-ymin)/yrange-.5)*yfactor*h)

static void hwr_scale_points (int n, double *x, double *y,
	double xmin, double xmax, double ymin, double ymax,
	double w, double h, double xoffset, double yoffset, double margin, int same_scale)
{
	double xrange = xmax-xmin;
	double yrange = ymax-ymin;
	double xfactor;
	double yfactor;
	double height = h;
	int i;

	if (same_scale) {
		xrange = Xrange;
		yrange = Yrange;
	}

	w -= 2. * margin;
	h -= 2. * margin;

	if (xrange/w > yrange/h) {
	        xfactor = 1.;
		yfactor = yrange/xrange;
		if(yfactor * w > h) {
			w = h;
		} else {
			h = w;
		}
		
	} else {
		xfactor = xrange/yrange;
		yfactor = 1.;
		if(xfactor * h > w) {
			h = w;
		} else {
			w = h;
		}
	}
		

	for (i=0;i<n;i++) {
/*		new_val = rescaled(x[i],xrange,xfactor,xmin,xoffset,w);*/
		x[i] = XS(x[i]);
/*		new_val = height - rescaled(y[i],yrange,yfactor,ymin,yoffset,h);*/
		y[i] = YS(y[i]);
	}
}

static int hwr_compress_line (XPoint *points, int n)
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

static void hwr_copy_points (double *Xi, double *Yi, int n,HwrResources *hwr)
{
	int i;

	if (hwr->bufsize==0) {
		hwr->X = (double *) ckalloc(n*sizeof(double));
		hwr->Y = (double *) ckalloc(n*sizeof(double));
		if (hwr->Y==NULL) {
			fprintf (stderr,"FATAL: unable to allocate %d bytes for copying points (hoping for the best)\n",n*sizeof(double));
		}
		hwr->bufsize = n;
	} else if (hwr->bufsize<n) {
		hwr->X = (double *) realloc(hwr->X,n*sizeof(double));
		hwr->Y = (double *) realloc(hwr->Y,n*sizeof(double));
		if (hwr->Y==NULL) {
			fprintf (stderr,"FATAL: unable to reallocate %d bytes for copying points (hoping for the best)\n",n*sizeof(double));
		}
		hwr->bufsize = n;
	}

	for (i=0;i<n;i++) {
		hwr->X[i] = Xi[i];
		hwr->Y[i] = Yi[i];
	}
}

void hwrDisplay (InkWidgetResources *ink, HwrResources *hwr)
{
	Display *disp;
	Drawable drwbl;
	int i,color,s;
	int x0,y0,x1;
	int c_left,xmin,xmax;
	int n,nsamples;
	int mstart,mend,swidth,text_offset;
	int _show_sample,_only_subsegs, _mark_penstreams;
	XPoint *points;
	InkSubSegments *subsegments;
	XCharStruct c_struct;
	ContainerBox *box = &hwr->box;
	double xorg,yorg,width,height;
	int same_scale = ink->_use_same_scale;
	double *X, *Y, *Z, minpres;
	char cursor_buf[256];
	GC m_gc;

	disp     = ink->display;
	drwbl    = ink->drawable;
	nsamples = ink->nsamples;

	points       = (XPoint *) ckalloc (nsamples*sizeof(XPoint));
	if (points==NULL) {
		fprintf (stderr,"unable to allocate %d Xpoints (skipping...)\n",nsamples);
		return;
	}
	text_offset  = ink->text_offset ;

	xorg = BOX_XORG(box);
	yorg = BOX_YORG(box);
	width = BOX_WIDTH(box);
	height = BOX_HEIGHT(box);
	if (box->showbox) {
		XDrawRectangle(disp,drwbl,ink->bd_gc
			,xorg,yorg,width,height);
	}
	if (!ink->_remember_bounds || (hwr->xmin==-1&&hwr->xmax==-1&&hwr->ymin==-1&&hwr->ymax==-1)) {
		hwr->xmin = ink->xmin;
		hwr->xmax = ink->xmax;
		hwr->ymin = ink->ymin;
		hwr->ymax = ink->ymax;
	}
	hwr_copy_points(ink->samples[0],ink->samples[1],nsamples,hwr);
	X = hwr->X;
	Y = hwr->Y;
	Z = ink->samples[2];
	minpres = ink->minpres;
	hwr_scale_points (nsamples,X,Y,hwr->xmin,hwr->xmax,hwr->ymin,hwr->ymax
		,(double)width,(double)height
		,(double)width/2,(double)height/2
		,ink->margin*width,use_global_bounds&&same_scale);

	_show_sample  = ink->_show_sample;
	_only_subsegs = ink->_only_subsegs;
	_mark_penstreams = ink->_mark_penstreams;

	swidth = ink->swidth;

#define XPOINT(x) ((short)(x+xorg))
#define YPOINT(y) ((short)(y+yorg))

	if (ink->cursor>=0) {
		if (ink->cursor>=nsamples) ink->cursor = 0;
		x0 = XPOINT(X[ink->cursor]);
		y0 = YPOINT(Y[ink->cursor]);
		DrawCursor(ink,x0,y0,box,1);
		sprintf (cursor_buf,"x=%.0f",ink->samples[0][ink->cursor]);
		XQueryTextExtents(ink->display,GRAPHICS_FONT,cursor_buf,strlen(cursor_buf),&x0,&y0,&i,&c_struct);
		x0 = BOX_WIDTH(box)-c_struct.width;
		y0 = BOX_YORG(box)+14;
		XDrawString(ink->display,ink->drawable,GRAPHICS_GC,x0,y0
					,cursor_buf,strlen(cursor_buf));
		sprintf (cursor_buf,"y=%.0f",ink->samples[1][ink->cursor]);
		XQueryTextExtents(ink->display,GRAPHICS_FONT,cursor_buf,strlen(cursor_buf),&x0,&y0,&i,&c_struct);
		n = y0 += c_struct.ascent+c_struct.descent+14;
		x0 = BOX_WIDTH(box)-c_struct.width;
		XDrawString(ink->display,ink->drawable,GRAPHICS_GC,x0,y0+14
					,cursor_buf,strlen(cursor_buf));
		sprintf (cursor_buf,"z=%.0f",ink->samples[2][ink->cursor]);
		XQueryTextExtents(ink->display,GRAPHICS_FONT,cursor_buf,strlen(cursor_buf),&x0,&y0,&i,&c_struct);
		y0 += c_struct.ascent+c_struct.descent+14;
		x0 = BOX_WIDTH(box)-c_struct.width;
		XDrawString(ink->display,ink->drawable,GRAPHICS_GC,x0,y0+n
					,cursor_buf,strlen(cursor_buf));
	}

	if (_mark_penstreams) {
		color = 0;
		s     = -1;
		n     = 0;
		for (i=0;i<nsamples;i++) {
			x0 = XPOINT(X[i]);
			y0 = YPOINT(Y[i]);

			switch (s) {

				case -1: /* PENLIFT UNKNOWN */

					s = (Z[i]>minpres);
					if (s==1) { /* the first is a pen down, adminstrate it */
						n = 1;
						points[0].x = x0;
						points[0].y = y0;
					} else { /* the first is a pen up, draw it */
						if (swidth>0)
							XDrawArc(disp,drwbl,ink->m_gc[color%ncolors]
								,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
					}
					break;

				case 0: /* PENLIFT WAS UP */

					if (Z[i]<=minpres) { /* was up, is up, draw it */
						if (swidth>0)
							XDrawArc(disp,drwbl,ink->m_gc[color%ncolors]
								,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
					} else { /* going down, was up, administrate the first down */
						s = 1;
						points[0].x = x0;
						points[0].y = y0;
						n = 1;
						color++;
					}
					break;

				case 1: /* PENLIFT WAS DOWN */

					if (Z[i]<=minpres) { /* was down, going up, flush downs */
						if (swidth>0)
							XDrawArc(disp,drwbl,ink->m_gc[color%ncolors]
								,x0-swidth/2,y0-swidth/2,swidth,swidth,0,360*64);
						color++;
						s = 0;
						if (n>1) {
						/*
							if (Z[i]!=-1) {
								points[n].x = x0;
								points[n].y = y0;
								n++;
							}
						*/
							n = hwr_compress_line (points,n);
							if (n>1) {
								if (_only_subsegs)
									XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
								XDrawLines(disp,drwbl,ink->m_gc[color%ncolors]
								,points,n,CoordModeOrigin);
								color++;
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
				if (s==1) {
					n = hwr_compress_line (points,n);
					if (n>1)
						if (_only_subsegs)
							XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
						XDrawLines(disp,drwbl,ink->m_gc[color%ncolors]
						,points,n,CoordModeOrigin);
				}
			}

		}
		ckfree(points);
		return;
	}

	if (!_only_subsegs && ink->dwidth>0) {
		n = 0;
		for (i=0;i<nsamples;i++) {
			x0 = XPOINT(X[i]);
			y0 = YPOINT(Y[i]);
			if (Z[i]>=minpres) {
				points[n].x = x0;
				points[n].y = y0;
				n++;
			} else if (n>0) {
				n = hwr_compress_line (points,n);
				if (n>1)
					XDrawLines(disp,drwbl,ink->pd_gc
					,points,n,CoordModeOrigin);
				n = 0;
			}
		}
		if (n>1) {
			n = hwr_compress_line (points,n);
			if (n>1)
				XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);
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
					fprintf (stderr,"error: marker start [%d] must be >= [0]!\n",mstart);
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
						n = hwr_compress_line (points,n);
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
				n = hwr_compress_line (points,n);
				if (_only_subsegs)
					if (n>1)
						XDrawLines(disp,drwbl,ink->pd_gc,points,n,CoordModeOrigin);

				if (n>1) {
					XDrawLines(disp,drwbl,m_gc,points,n,CoordModeOrigin);
				}

			}
			if (mend-mstart>=1 && text_offset >= -100) {
				XQueryTextExtents(disp,SUBSEG_FONT,subsegments->m_name[s],strlen(subsegments->m_name[s])
					,&x0,&x0,&x0,&c_struct);
				c_left = c_struct.width/2;

				XDrawString(disp,drwbl,m_gc
					,XPOINT((xmax-xmin)/2+xmin-c_left),YPOINT(height-text_offset)
					,subsegments->m_name[s],strlen(subsegments->m_name[s]));

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

	if (ink->labelString!=NULL&&subsegments->nsubsegments==0 && text_offset >= -100) {
		xmin =  999999;
		xmax = -999999;
		for (i=0;i<nsamples;i++) {
			x0 = (short) X[i];
			if (xmin>x0) xmin = x0;
			if (xmax<x0) xmax = x0;
		}
		x0 = strlen(ink->labelString);
		XQueryTextExtents(disp,LABEL_FONT
			,ink->labelString,x0
			,&x1,&x1,&x1,&c_struct);
		c_left = c_struct.width/2;
		XDrawString(disp,drwbl,LABEL_GC
			,XPOINT((xmax-xmin)/2+xmin-c_left),YPOINT(height-text_offset)
			,ink->labelString,x0);
	}
	ckfree(points);
}

#define cursor_dist(x0,y0,x1,y1) ( ((x0)-(x1))*((x0)-(x1)) + ((y0)-(y1))*((y0)-(y1)) )

int hwrGetCursorInSignal (InkWidgetResources *ink, HwrResources *hwr, int x, int y)
{
	double *X, *Y;
	double dist,min_dist;
	int i,n,min_index;
	ContainerBox *box = &hwr->box;
	double xorg,yorg;

	xorg = BOX_XORG(box);
	yorg = BOX_YORG(box);

	if ((n=ink->nsamples)==0)
		return -1;
	X = hwr->X;
	Y = hwr->Y;
	min_index = 0;
	min_dist = cursor_dist(XPOINT(X[0]),YPOINT(Y[0]),x,y);
	for (i=1;i<n;i++) {
		dist = cursor_dist(XPOINT(X[i]),YPOINT(Y[i]),x,y);
		if (dist<min_dist) {
			min_dist = dist;
			min_index = i;
		}
	}
	return min_index;
}
