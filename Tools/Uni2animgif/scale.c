#include <stdio.h>
#include <stdlib.h>

/* something to scale points */

#define rescaled(a,r,f,min,offset,s) (offset + ((a-min)/r-.5)*f*s)
#define my_rescaled(a,r,f,min,offset,s) (((a-min)/r-.5)*f*s)
#define XS(x) (xoffset+((x-xmin)/xrange-.5)*xfactor*w)
#define YS(y) (height-yoffset-((y-ymin)/yrange-.5)*yfactor*h)

extern void hwr_scale_points (int n, double *x, double *y,
	double xmin, double xmax, double ymin, double ymax,
	double w, double h, double xoffset, double yoffset, double margin)
{
	double xrange = xmax-xmin;
	double yrange = ymax-ymin;
	double xfactor;
	double yfactor;
	double new_val,height = h;
	int i;

	w -= 2. * w*margin;
	h -= 2. * h*margin;

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
		new_val = rescaled(x[i],xrange,xfactor,xmin,xoffset,w);
		x[i] = XS(x[i]);
		new_val = height - rescaled(y[i],yrange,yfactor,ymin,yoffset,h);
		y[i] = YS(y[i]);
	}
}
