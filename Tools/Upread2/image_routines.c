#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "image_routines.h"
#include "bitmap_routines.h"

/* this is ugly */
unsigned char pd_red, pd_green, pd_blue
	, pu_red, pu_green, pu_blue
	, bg_red, bg_green, bg_blue;

void write_image (char *fname, int **image, int w, int h, int frmt)
{
	switch (frmt) {
		case O_XBM:
			write_bitmap_data (fname,image,w,h);
			break;
		case O_PPM:
			write_data (fname,image,w,h);
			break;
		default:
			fprintf (stderr,"unknown format [%d] while trying to write image to '%s'\n"
				,frmt,fname);
			break;
	}
}

void write_merged_image (int **image, int w, int h, char *fname)
{
	FILE *fp;
	unsigned char *pgm_row,*ptr;
	int gray_max,i,j;

	if ((fp=fopen(fname,"wb"))==NULL) {
		fprintf (stderr,"unable to open %s for output!!\n",fname);
		exit(1);
	}
	gray_max = 0;
	for (i=0;i<h;i++)
		for (j=0;j<w;j++)
			if (image[i][j]>gray_max) gray_max = image[i][j];
	pgm_row = (unsigned char *) malloc (3*w);
	fprintf (fp,"P6\n# CREATOR: OME LOE\n%d %d\n255\n",w,h);
	for (i=0;i<h;i++) {
		ptr = pgm_row;
		for (j=0;j<w;j++) {
			ptr[0] = ptr[1] = ptr[2] = (unsigned char) (256 - ( (int) (256.0/gray_max*image[i][j]) ));
			ptr += 3;
		}
		fwrite(pgm_row,sizeof(unsigned char),3*w,fp);
	}
	fclose(fp);
}

void write_bitmap_data (char *fname, int **image, int w, int h)
{
	char **data;
	int i,j;

	data = (char **) malloc (h*sizeof(char *));
	for (i=0;i<h;i++) {
		data[i] = (char *) calloc (w,sizeof(char));
		for (j=0;j<w;j++)
			if (image[i][j])
				data[i][j] = 1;
	}
	write_bitmap (w,h,data,fname);
	for (i=0;i<h;i++)
		free(data[i]);
	free(data);
}

void write_data (char *fname, int **image, int w, int h)
{
	FILE *fp;
	unsigned char *pgm_row,*ptr;
	int i,j;

	if ((fp=fopen(fname,"wb"))==NULL) {
		fprintf (stderr,"unable to open %s for output!!\n",fname);
		exit(1);
	}
	pgm_row = (unsigned char *) malloc (3*w);
	fprintf (fp,"P6\n# CREATOR: OME LOE\n%d %d\n255\n",w,h);
	for (i=0;i<h;i++) {
		ptr = pgm_row;
		for (j=0;j<w;j++) {
			switch (image[i][j]) {
				case BG_COLOR:
					*ptr++ = bg_red;
					*ptr++ = bg_green;
					*ptr++ = bg_blue;
					break;
				case PU_COLOR:
					*ptr++ = pu_red;
					*ptr++ = pu_green;
					*ptr++ = pu_blue;
					break;
				case PD_COLOR:
					*ptr++ = pd_red;
					*ptr++ = pd_green;
					*ptr++ = pd_blue;
					break;
				case GREEN:
					*ptr++ = 91;
					*ptr++ = 100;
					*ptr++ = 100;
					break;
				case RED:
					*ptr++ = 245;
					*ptr++ = 119;
					*ptr++ = 91;
					break;
				case WHITE:
					*ptr++ = 255;
					*ptr++ = 255;
					*ptr++ = 255;
					break;
				default:
					ptr[0] = ptr[1] = ptr[2] = (unsigned char) ((int) image[i][j]);
					ptr += 3;
					break;
			}
		}
		fwrite(pgm_row,sizeof(unsigned char),3*w,fp);
	}
	fclose(fp);
}

void brush (int **im, int x, int y, int col, int BRUSH)
{
	int lx, ly;
	int hx, hy;
	int x1, y1, x2, y2;

	hy = (int) BRUSH/2;
	hx =  (int)BRUSH/2;
	x1 = x - hx;
	x2 = x1+BRUSH;
	y1 = y - hy;
	y2 = y1 + BRUSH;
	if (col==ADD_PIXEL)
		for (ly = y1; (ly < y2); ly++) {
			for (lx = x1; (lx < x2); lx++) {
				im[ly][lx] += 1;
			}
		}
	else
		for (ly = y1; (ly < y2); ly++) {
			for (lx = x1; (lx < x2); lx++) {
				im[ly][lx] = col;
			}
		}
}


void draw_line_in_image (int **image, int x1, int y1, int x2, int y2, int col, int BRUSH)

/* stolen from gd.c by LOE */
/* Bresenham as presented in Foley & Van Dam */

{
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;

	dx = abs(x2-x1);
	dy = abs(y2-y1);
	if (dy <= dx) {
		d = 2*dy - dx;
		incr1 = 2*dy;
		incr2 = 2 * (dy - dx);
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}
brush(image,x,y,col,BRUSH);
		if (((y2 - y1) * ydirflag) > 0) {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y++;
					d+=incr2;
				}
brush(image,x,y,col,BRUSH);
			}
		} else {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y--;
					d+=incr2;
				}
brush(image,x,y,col,BRUSH);
			}
		}		
	} else {
		d = 2*dx - dy;
		incr1 = 2*dx;
		incr2 = 2 * (dx - dy);
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}
brush(image,x,y,col,BRUSH);
		if (((x2 - x1) * xdirflag) > 0) {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x++;
					d+=incr2;
				}
brush(image,x,y,col,BRUSH);
			}
		} else {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x--;
					d+=incr2;
				}
brush(image,x,y,col,BRUSH);
			}
		}
	}
}

void make_rectangles (int **image, int x0, int y0, int N, int width)
{
	int dx = width/N;
	int xorg,yorg;

	xorg = x0*dx/2;
	yorg = y0*dx/2;
	draw_line_in_image(image,xorg,yorg,xorg,yorg+dx-1,WHITE,1);
	draw_line_in_image(image,xorg,yorg+dx-1,xorg+dx-1,yorg+dx-1,WHITE,1);
	draw_line_in_image(image,xorg+dx-1,yorg+dx-1,xorg+dx-1,yorg,WHITE,1);
	draw_line_in_image(image,xorg+dx-1,yorg,xorg,yorg,WHITE,1);
	brush(image,xorg,yorg,GREEN,1);
}

void plot_allo_in_image (int **image, int n, int xoff, int yoff
	, int *x, int *y, int *z, int pd_col, int pu_col, int BRUSH)
{
	int i;
	int x0,y0,z0,x1,y1,z1;

	x0 =  x[0] + xoff;
	y0 =  y[0] + yoff;
	z0 =  z[0];
	for (i=1;i<n;i++) {
		x1 =  x[i] + xoff;
		y1 =  y[i] + yoff;
		z1 =  z[i];
		if (z0<=MINPRES)
			draw_line_in_image (image,x0,y0,x1,y1,pu_col,BRUSH);
		else
			draw_line_in_image (image,x0,y0,x1,y1,pd_col,BRUSH);
		x0 = x1;
		y0 = y1;
		z0 = z1;
	}
}

void float_plot_allo_in_image (int **image, int n, int xoff, int yoff
	, float *x, float *y, float *z, int pd_col, int pu_col, int BRUSH)
{
	int i;
	int x0,y0,z0,x1,y1,z1;

	x0 =  (int) x[0] + xoff;
	y0 =  (int) y[0] + yoff;
	z0 =  (int) z[0];
	for (i=1;i<n;i++) {
		x1 =  (int) x[i] + xoff;
		y1 =  (int) y[i] + yoff;
		z1 =  (int) z[i];
		if (z0<=MINPRES)
			draw_line_in_image (image,x0,y0,x1,y1,pu_col,BRUSH);
		else
			draw_line_in_image (image,x0,y0,x1,y1,pd_col,BRUSH);
		x0 = x1;
		y0 = y1;
		z0 = z1;
	}
}

#define KWADRAAT(a) ((a)*(a))
void float_normalize_allo_xy (int n, float *x, float *y, double *xo, double *yo)
{
	double rmx,rmy,dd,d;
	int i;

	rmx = rmy = 0.0;
	for (i=0;i<n;i++) {
		rmx += x[i];
		rmy += y[i];
	}
	rmx  /= n;
	rmy  /= n;
	
	dd = 0.0;

	for (i=0;i<n;i++) {
		dd += KWADRAAT(rmx-x[i]) + KWADRAAT(rmy-y[i]);
	}
	d = sqrt(dd/n);
	if (d<=0.000001) d = 1.;

	for (i=0;i<n;i++) {
		xo[i] = (double) (x[i]-rmx) / d;
		yo[i] = (double) (y[i]-rmy) / d;
	}
}

void normalize_allo_xy (int n, int *x, int *y, double *xo, double *yo)
{
	double rmx,rmy,dd,d;
	int i;

	rmx = rmy = 0.0;
	for (i=0;i<n;i++) {
		rmx += x[i];
		rmy += y[i];
	}
	rmx  /= n;
	rmy  /= n;
	
	dd = 0.0;

	for (i=0;i<n;i++) {
		dd += KWADRAAT(rmx-x[i]) + KWADRAAT(rmy-y[i]);
	}
	d = sqrt(dd/n);
	if (d<=0.000001) d = 1.;

	for (i=0;i<n;i++) {
		xo[i] = (double) (x[i]-rmx) / d;
		yo[i] = (double) (y[i]-rmy) / d;
	}
}

#define XS(x) (xoffset+((x-xmin)/xrange-.5)*xfactor*w)
#define YS(y) (height-yoffset-((y-ymin)/yrange-.5)*yfactor*h)

void scale_int_allo (int n, int *xi, int *yi, int *x, int *y, int width, int height, int margin)
{
	double xmin,xmax,ymin,ymax;
	double xrange,yrange,xfactor,yfactor,w,h;
	double xoffset = width/2.0;
	double yoffset = height/2.0;
	int i;

	xmin = xmax = xi[0];
	ymin = ymax = yi[0];
	for (i=1;i<n;i++) {
		if (xmin>xi[i]) xmin = xi[i];
		if (xmax<xi[i]) xmax = xi[i];
		if (ymin>yi[i]) ymin = yi[i];
		if (ymax<yi[i]) ymax = yi[i];
	}
	w = width  - margin;
	h = height - margin;
	xrange = xmax-xmin;
	yrange = ymax-ymin;
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
		x[i] = (int)XS(xi[i]);
		y[i] = (int)YS(yi[i]);
	}
}

void scale_allo (int n, double *xi, double *yi, int *x, int *y, int width, int height, int margin)
{
	double xmin,xmax,ymin,ymax;
	double xrange,yrange,xfactor,yfactor,w,h;
	double xoffset = width/2.0;
	double yoffset = height/2.0;
	int i;

	xmin = xmax = xi[0];
	ymin = ymax = yi[0];
	for (i=1;i<n;i++) {
		if (xmin>xi[i]) xmin = xi[i];
		if (xmax<xi[i]) xmax = xi[i];
		if (ymin>yi[i]) ymin = yi[i];
		if (ymax<yi[i]) ymax = yi[i];
	}
	w = width  - margin;
	h = height - margin;
	xrange = xmax-xmin;
	yrange = ymax-ymin;
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
		x[i] = (int)XS(xi[i]);
		y[i] = (int)YS(yi[i]);
	}
}

double global_xrange;
double global_yrange;
double global_xmin;
double global_xmax;
double global_ymin;
double global_ymax;
int use_same_scale;

void float_scale_allo (int n, double *xi, double *yi
	, float *x, float *y, int width, int height, int margin)
{
	double xmin,xmax,ymin,ymax;
	double xrange,yrange,xfactor,yfactor,w,h;
	double xoffset = width/2.0;
	double yoffset = height/2.0;
	int i;

	xmin = xmax = xi[0];
	ymin = ymax = yi[0];
	for (i=1;i<n;i++) {
		if (xmin>xi[i]) xmin = xi[i];
		if (xmax<xi[i]) xmax = xi[i];
		if (ymin>yi[i]) ymin = yi[i];
		if (ymax<yi[i]) ymax = yi[i];
	}
	w = width  - margin;
	h = height - margin;
	if (use_same_scale) {
		xrange = global_xrange;
		yrange = global_yrange;
	} else {
		xrange = xmax-xmin;
		yrange = ymax-ymin;
	}
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
		x[i] = (int)XS(xi[i]);
		y[i] = (int)YS(yi[i]);
	}
}

int determine_brush (int xmin,int xmax,int ymin,int ymax,int width,int height)
{
	double r;
	int brush;

	if (xmax-xmin>ymax-ymin)
		r = xmax-xmin;
	else
		r = ymax-ymin;
	brush = (int) (.2*50*width/r);
	if (brush<MIN_BRUSH) {
		return MIN_BRUSH;
	} else
		return brush;
}

int count_pixels_in_sub_image (int **image, double *X, double *Y, int x0, int y0, int N, int width)
{
	int dx = width/N;
	int i,j;
	double x,y;
	int xorg,yorg;
	int npixels = 0;

	xorg = x0*dx/2;
	yorg = y0*dx/2;
	for (i=xorg;i<xorg+dx;i++)
		for (j=yorg;j<yorg+dx;j++) {
			if (image[j][i]==RED) {
				x = (double) (i-xorg)/dx;
				y = (double) (j-yorg)/dx;
				X[npixels] = x;
				Y[npixels] = y;
				npixels++;
			}
	}
	return npixels;
}

void visualize_image (int **image, int w, int h, int dowait)
{
	char cmd[256];

	if (dowait)
		sprintf (cmd,"xv /tmp/loetje.gif");
	else
		sprintf (cmd,"xv /tmp/loetje.gif &");
	write_data("/tmp/loetje.gif",image,w,h);
	system (cmd);
}

void set_bg_image (int **image, int w, int h)
{
	int i,j;

	for (i=0;i<h;i++)
		for (j=0;j<w;j++)
			image[i][j] = BG_COLOR;
}

void clear_image (int **image, int w, int h)
{
	int i;

	for (i=0;i<h;i++)
		memset((void *)image[i],0,w*sizeof(int));
}
