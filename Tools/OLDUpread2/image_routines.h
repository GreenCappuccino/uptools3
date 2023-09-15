#ifndef _IMAGE_ROUTINES
#define _IMAGE_ROUTINES

#define MINPRES .25

#define MIN_BRUSH 2

#define MAX_COORDINATES_IN_HWR 2048
#define MAX_PIXELS_IN_IMG      100000

#define ADD_PIXEL -999
#define RED   -1
#define WHITE -2
#define GREEN -3

#define O_XBM 0
#define O_PPM 1

extern void write_image (char *fname, int **image, int w, int h, int frmt);

extern void write_merged_image (int **image, int w, int h, char *fname);
extern void write_data (char *fname, int **image, int w, int h);
extern void write_bitmap_data (char *fname, int **image, int w, int h);
extern void brush (int **im, int x, int y, int col, int BRUSH);
extern void draw_line_in_image (int **image, int x1, int y1, int x2, int y2, int col, int BRUSH);
extern void make_rectangles (int **image, int x0, int y0, int N, int width);
extern void plot_allo_in_image (int **image, int n, int *x, int *y, int *z, int col, int BRUSH);
extern void float_plot_allo_in_image (int **image, int n
	, float *x, float *y, float *z, int col, int BRUSH);
extern void normalize_allo_xy (int n, int *x, int *y, double *xo, double *yo);
extern void float_normalize_allo_xy (int n, float *x, float *y, double *xo, double *yo);
extern void scale_allo (int n, double *xi, double *yi, int *x, int *y, int width, int height, int margin);
extern void float_scale_allo (int n, double *xi, double *yi
	, float *x, float *y, int width, int height, int margin);
extern int determine_brush (int xmin,int xmax,int ymin,int ymax,int width,int height);
extern int count_pixels_in_sub_image (int **image, double *X, double *Y, int x0, int y0, int N, int width);

extern void visualize_image (int **, int, int, int);
extern void clear_image (int **, int, int);


#endif
