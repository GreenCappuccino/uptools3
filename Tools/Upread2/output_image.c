#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <uplib.h>
#include <upsiglib.h>

#include "unipen_identifications.h"
#include "upread2.h"
#include "output_image.h"
#include "image_routines.h"
#include "allo_resample.h"


int cleanup_and_scale_signal (double dfactor,int m, int ns, int *xi, int *yi, int *zi
	, float x[NSMAX], float y[NSMAX], float z[NSMAX]
	, double X[NSMAX], double Y[NSMAX])
{
	float xs[NSMAX], ys[NSMAX], zs[NSMAX], va[NSMAX];
	int offset, i, no;

	/* remove pen_up head */
	offset = 0;
	while (zi[offset]<15.) {
		if (offset==ns-1) {
			fprintf (stderr,"skipping penup segment!\n");
			return 0;
		}
		offset++;
	}
	/* remove pen-up tail */
	ns = ns - 1;
	while (zi[ns]<15.&&ns>offset) {
		ns--;
	}

	ns = ns-offset+1;
	for (i=0;i<ns;i++) {
		x[i] = (float) xi[i+offset];
		y[i] = (float) yi[i+offset];
		z[i] = (float) zi[i+offset];
	}

	if (m!=-1) {
		no = m;
		recog_spatial_z_sampler(x,y,z,ns
			,xs,ys,zs,no,va,0,ns-1,15.);
		if (dfactor!=-1.0)
			adjust_resampling (ns,xi,yi,zi
	   		,no,xs,ys,zs,dfactor);
		float_normalize_allo_xy(no,xs,ys,X,Y);
	} else {
		no = ns;
		for (i=0;i<no;i++) {
			X[i] = (double) xi[i+offset];
			Y[i] = (double) yi[i+offset];
		}
	}
	return no;
}

void do_output_image (char *fname, int ** image, int brush, int margin
	, int w, int h
	, int ns, int m, int *xi, int *yi, int *zi
	, int ofrmt, double dfactor)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	double X[NSMAX],Y[NSMAX];
	int i, no, ns_org;

	ns_org = ns;
	no = cleanup_and_scale_signal(dfactor,m,ns,xi,yi,zi,x,y,z,X,Y);
	if (no==0)
		return;
	float_scale_allo(no,X,Y,x,y,w,h,margin);
	float_plot_allo_in_image(image,no,0,0,x,y,z,RED,GREEN,brush);
#define _DEBUG_ORIGINAL_IMAGE_
#ifdef _DEBUG_ORIGINAL_IMAGE_
	if (dfactor!=-1.0) {
		fprintf (stderr,"also showing original image\n");
		no = 0;
		for (i=0;i<ns_org;i++) {
			if (zi[i]>15.) {
				X[no] = (float) xi[i];
				Y[no] = (float) yi[i];
				no++;
			}
		}
		float_scale_allo(no,X,Y,x,y,w,h,margin);
		float_plot_allo_in_image(image,no,0,0,x,y,z,PD_COLOR,PU_COLOR,brush);
	}
#endif
	write_image(fname,image,w,h,ofrmt);
}

void do_output_image_part (char *fname, int ** image, int brush, int margin
	, int w, int h
	, int ns, int m, int *xi, int *yi, int *zi
	, double dfactor, int xoff, int yoff)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	double X[NSMAX],Y[NSMAX];
	int no;

	no = cleanup_and_scale_signal(dfactor,m,ns,xi,yi,zi,x,y,z,X,Y);
	if (no==0)
		return;
	float_scale_allo(no,X,Y,x,y,w,h,margin);
	float_plot_allo_in_image(image,no,xoff,yoff,x,y,z,PD_COLOR,PU_COLOR,brush);
}


void determine_global_scales (double dfactor, int m, int ns
	, int *xi, int *yi, int *zi)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	double X[NSMAX],Y[NSMAX];
	double xmin,xmax,ymin,ymax;
	double xrange,yrange;
	int i,no;

	no = cleanup_and_scale_signal(dfactor,m,ns,xi,yi,zi,x,y,z,X,Y);
	if (no==0)
		return;

	xmin = xmax = X[0];
	ymin = ymax = Y[0];
	for (i=1;i<no;i++) {
		if (xmin>X[i]) xmin = X[i];
		if (xmax<X[i]) xmax = X[i];
		if (ymin>Y[i]) ymin = Y[i];
		if (ymax<Y[i]) ymax = Y[i];
	}
	xrange = xmax-xmin;
	yrange = ymax-ymin;
	if (xrange>global_xrange)
		global_xrange = xrange;
	if (yrange>global_yrange)
		global_yrange = yrange;
	if (global_xmin>xmin) global_xmin = xmin;
	if (global_xmax<xmax) global_xmax = xmax;
	if (global_ymin>ymin) global_ymin = ymin;
	if (global_ymax<ymax) global_ymax = ymax;
}

void init_output_image (FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo)
{
}

int output_image (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams)
{
	tUPEntry *entry,**segment_entries;
	int i,ns,width,height, *xi,*yi,*zi;
	static int **image = NULL;
	static int idx = 0;
	char fname[256];

	width = oinfo->width;
	height = oinfo->height;
	if (image==NULL) {
		image = (int **) malloc (height*sizeof(int *));
		for (i=0;i<height;i++)
			image[i] = (int *) calloc (width,sizeof(int));
	}

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	for (i=0;i<nentries;i++) {
		entry = entries[i];
		if (oinfo->im_frmt==O_XBM)
			sprintf (fname,"%s_%d.xbm",oinfo->outfile,idx++);
		else if (oinfo->im_frmt==O_PPM)
			sprintf (fname,"%s_%d.ppm",oinfo->outfile,idx++);
		else {
			fprintf (stderr,"output format must be ppm or xbm!\n");
			return 0;
		}
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
		do_output_image (fname,image,oinfo->brush,oinfo->margin
			,width,height,ns,oinfo->m,xi,yi,zi
			,oinfo->im_frmt,oinfo->dfact);
		free(xi);
		free(yi);
		free(zi);
	}
	return 1;
}

static int compute_boxsize (int n, int w, int h)
{
	int ncols = (int) sqrt((double)n);

	return (int) ((double)w/ncols);
}

int output_ppm (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams)
{
	tUPEntry *entry,**segment_entries;
	int i,ns,width,height, *xi,*yi,*zi;
	int bwidth,bheight;
	static int **image = NULL;
	char fname[256];
	int xoff, yoff, nrows,ncols;

	width   = oinfo->width;
	height  = oinfo->height;
	if (oinfo->ncols==-1) {
		ncols   = (int) sqrt((double)nentries);
		if (ncols*ncols<nentries)
			ncols += 1;
		nrows   = ncols;
		bwidth  = (int) ((double)width/ncols);
		bheight = bwidth;
	} else {
		ncols   = oinfo->ncols;
		nrows   = ( nentries + ncols -1 ) / ncols;
		bwidth  = (int) ((double)width/ncols);
		bheight = (int) ((double)height/nrows);
	}

fprintf (stderr,"OUTPUT_PPM: %d entries in %dx%d image: %d rows, %d cols, bsize=%dx%d\n"
	,nentries,width,height,nrows,ncols,bwidth,bheight);

	if (image==NULL) {
		fprintf (stderr,"allocating %dx%d image\n",width,height);
		image = (int **) malloc (height*sizeof(int *));
		for (i=0;i<height;i++)
			image[i] = (int *) calloc (width,sizeof(int));
	}

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];

	if (oinfo->use_same_scale) {
		use_same_scale = 1;
		for (i=0;i<nentries;i++) {
			entry = entries[i];
			sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
			determine_global_scales(oinfo->dfact,oinfo->m,ns,xi,yi,zi);
			free(xi);
			free(yi);
			free(zi);
		}
		fprintf (stderr,"all segments in [%f,%f] [%f,%f] and [%f,%f]\n"
			, global_xmin, global_xmax, global_ymin, global_ymax
			, global_xrange, global_yrange);
	}

	set_bg_image(image,width,height);
	for (i=0;i<nentries;i++) {
		xoff  = i%ncols *bwidth;
		yoff  = i/ncols * bheight;
		entry = entries[i];
		if (oinfo->im_frmt==O_XBM)
			sprintf (fname,"%s.xbm",oinfo->outfile);
		else if (oinfo->im_frmt==O_PPM)
			sprintf (fname,"%s.ppm",oinfo->outfile);
		else {
			fprintf (stderr,"output format must be ppm or xbm!\n");
			return 0;
		}
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
		do_output_image_part (fname,image,oinfo->brush,oinfo->margin
			,bwidth,bheight,ns,oinfo->m,xi,yi,zi
			,oinfo->dfact,xoff,yoff);
		free(xi);
		free(yi);
		free(zi);
	}
	write_image(fname,image,width,height,oinfo->im_frmt);
	return 1;

}
