#ifndef _ALLO_RESAMPLE_
#define _ALLO_RESAMPLE_

#define MAX_VALID_SAMPLES 150

#define MAX_RESAMPLE_POINTS 4096

#define NSMAX             4096

#define FEATCHAR_PENDOWN  1.0
#define FEATCHAR_PENUP    0.0

extern void allo_linearize (float *x, float *y, float *z
	, int ns, double prsmin);
extern float recog_allo_interpolate (float r[], int j, int n
	, double rn, float def);
extern void recog_spatial_z_sampler (float x_in[NSMAX], float y_in[NSMAX], float z[NSMAX], int n
	, float xo[NSMAX], float yo[NSMAX], float zo[NSMAX], int no
	, float va[NSMAX], int ii, int jj, double prsmin);
extern void recog_normalize_rms (int nnorm, float  xnorm[NSMAX], float  ynorm[NSMAX]);
extern void allo_sampler_interface (int nsamples, int *xi, int *yi, int *zi
	, float *xo, float *yo, float *zo, int m, double prsmin);
extern void allo_double_sampler_interface (int nsamples, double *xi, double *yi, double *zi
	, double *xo, double *yo, double *zo, int m, double prsmin);
extern void normalize_allo (double *v, int m);

extern void adjust_resampling (int ni, int *xi, int *yi, int *zi
	, int m, float *xo, float *yo, float *zo, double dfactor);


#endif
