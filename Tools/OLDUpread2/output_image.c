#include <stdio.h>
#include <stdlib.h>
#include <uplib.h>
#include <upsiglib.h>

#include "unipen_identifications.h"
#include "upread2.h"
#include "output_image.h"
#include "image_routines.h"
#include "allo_resample.h"

void do_output_image (char *fname, int ** image, int brush, int margin
	, int w, int h
	, int ns, int m, int *xi, int *yi, int *zi
	, int ofrmt, double dfactor)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	float xs[NSMAX], ys[NSMAX], zs[NSMAX], va[NSMAX];
	double X[NSMAX],Y[NSMAX];
	int offset, i, no, ns_org;

	ns_org = ns;
	/* remove pen_up head */
	offset = 0;
	while (zi[offset]<15.) {
		if (offset==ns-1) {
			fprintf (stderr,"skipping penup segment!\n");
			return;
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
			zs[i] = z[i];
		}
	}

	clear_image(image,w,h);
	float_scale_allo(no,X,Y,xs,ys,w,h,margin);
	float_plot_allo_in_image(image,no,xs,ys,zs,RED,brush);
#define _DEBUG_ORIGINAL_IMAGE_
#ifdef _DEBUG_ORIGINAL_IMAGE_
	if (dfactor!=-1.0) {
		fprintf (stderr,"also showing original image\n");
		no = 0;
		for (i=0;i<ns_org;i++) {
			if (zi[i]>15.) {
				X[no] = (float) xi[i];
				Y[no] = (float) yi[i];
				zs[no] = (float) zi[i];
				no++;
			}
		}
		float_scale_allo(no,X,Y,xs,ys,w,h,margin);
		float_plot_allo_in_image(image,no,xs,ys,zs,WHITE,brush);
	}
#endif
	write_image(fname,image,w,h,ofrmt);
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
		fprintf (stderr,"getting samples for '%s'\n",entry->Entry);
		if (oinfo->im_frmt==O_XBM)
			sprintf (fname,"%s_%d.xbm",oinfo->outfile,idx++);
		else if (oinfo->im_frmt==O_PPM)
			sprintf (fname,"%s_%d.ppm",oinfo->outfile,idx++);
		else {
			fprintf (stderr,"output format must be ppm or xbm!\n");
			return 0;
		}
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
		fprintf (stderr,"got %d samples\n",ns);
		do_output_image (fname,image,oinfo->brush,oinfo->margin
			,width,height,ns,oinfo->m,xi,yi,zi
			,oinfo->im_frmt,oinfo->dfact);
		free(xi);
		free(yi);
		free(zi);
	}
	return 1;

	return 1;
}
