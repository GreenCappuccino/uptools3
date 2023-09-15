/* siglib.c Copyright L.R.B.Schomaker 1996 

 Package siglib.c plus optional Main program and tester of the package 
  
 This package is intended for use in the UNIPEN software toolkit.
 
 No warranties. Use as is. Please contact Lambert Schomaker
                (schomaker@nici.kun.nl) in case of commercial use.

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifndef PI
#define PI 3.141592654
#endif
#define MINR 0.0001

double siglib_angle(x,y)
/*
  INPUT: X,Y = COORDINATES OF VECTOR
  FUNCTION VALUE: ANGLER = ANGLE IN RADIANS
*/
double x,y;
{
	double phi;

	if (x != 0.0) {
		phi = atan(y / x);
		if(x < 0. && y >= 0.) phi = phi + (double) PI;
		if(x < 0. && y < 0.) phi = phi - (double) PI;
	}
	else {
/*   special case: vertical vector */
		phi = 2. * (double) PI;
		if(y > 0.) phi = .5 * (double) PI;
		if(y < 0.) phi = -.5 * (double) PI;
	}
	return phi;
}



double siglib_rmean(arr, n)
double *arr;
int n;
{
	double sum;
	register int i;

        if ( n < 1) {
		fprintf(stderr, "%%RMEAN n<1\n");
		exit(1);
	}
	sum = (double) 0.0;
	for(i = 0; i < n; i++)
		sum = sum + (double) arr[i];
        return (sum / (double) n);
}


void siglib_polar(double x[], double y[], double gain[], double angle[], int n)
/*
 From cartesian to polar
 -Input:
   X,Y (real arrays)
   N (integer)
 -Output:     
   GAIN,ANGLE (real arrays)

 X mag dezelfde array zijn als GAIN in de aanroeper
 Y mag dezelfde array zijn als ANGLE in de aanroeper
 Let hier wel op bj veranderingen!

 Revised: 18-apr-1986 16:58:09 
*/
{
	double r, phi;
	int i;

	for(i = 0; i < n; ++i) {
/*
    niets doen als vectorlengte kleiner dan een criterium is.
    (ook veilig voor atan()
*/
	    r = sqrt( x[i] * x[i] + y[i] * y[i] );
	    
	    if(r < MINR) {
	       /* we cannot calculate a reliable angle */
               if(i == 0) {
                  phi = 0.0;
               } else {
                  phi = angle[i-1]; /* take previous as best estimate */
               }
	    } else {

	       phi = siglib_angle(x[i], y[i]);
            }
            
	    gain[i] = r;
	    angle[i] = phi;
        }
} /* end siglib_polar() */

void siglib_fir_filter(
    double arrin[],
    double arrout[],
    int na,
    double h[],
    int nh)
/*

 Time domain filtering by convolution with an impulse response.
 Example smoothing with a rectangular window is done by filling H with
 the value 1/nh at all places. Better impulse responses are designed
 with DESIGN/FCHECK. Can also be used for differentiation (see DIFFER).

 -Input:
      ARRIN (real array) NA values    input data
      H (real array) NH values        impulse response

      NH number of FIR coefficients

 -Output:
      ARROUT (real array) NA values   output data

 Author: L.R.B. Schomaker
 Revised: 26-apr-1989 10:58:57 (comments only) 
*/

{
	double *bufin, *bufout;
	int iodd, idel, np2, iadd, isub, nd, nl;
	double fy, ffy, oldy;
	register int i,ii,k,m,j,io,npi,k1;

	bufin = (double *) malloc(nh * sizeof(double));
	bufout = (double *) malloc(nh * sizeof(double));

	nd = 0;
	oldy = 0.0;

/* Initialisatie pointers */
	ii = 0;
	io = 0;
/* oneven aant filtpunten */
	iodd = nh % 2;
/* helft van nh */
	np2 = nh/2;
	iadd = 1 + iodd;
	isub = 1 - iodd;
/* Vul buffer met nh waarden. */
	k = 0;
	for(i=0; i < nh; i++){
	        bufin[k++] = arrin[ii++];
	}
/* Inloop v.h. filter: benader eerste deel */

	for(k=1-iodd; k < (np2+1); k++){

	   idel = k - np2;
           fy = 0.0;
           for(m = 0; m < nh; m++){
              k1 = m + idel;
              if(k1 <= 0) k1 = -k1;
              fy = fy + bufin[k1] * h[m];
           }
	   ffy = fy;
	   if((k == 1) && (iodd == 0)) oldy = fy;

/* bereken juiste punt igv even filter */

	   fy = 0.5 * (iadd * fy + isub * oldy);
	   oldy = ffy;
	   bufout[k] = fy;
        }

	if(iodd != 1) bufout[0] = bufout[1];

/* Buffer out */

	k = 0;
	for(j = 0; j < (np2+iodd); j++){
	   arrout[io++] = bufout[k++];
	}

/* Normale convolutie (middenstuk). */
/* NPI wijst naar het oudste (overschrijfbare) sample */

	npi = 0;
	while(ii < na){

/* Nieuw sample overschrijft oudste waarde in buffer. */
	   bufin[npi] = arrin[ii++];

	   fy = 0.;
	   for(m = 0; m < nh; m++){
	      nd = ((m+npi+1) % nh);
	      fy = fy + bufin[nd] * h[m];
	   }
	   ffy = fy;

/* bereken juiste punt igv even filter */

	   fy = 0.5 * (iadd * fy + isub * oldy);
	   oldy = ffy;

/* filt punt wegschrijven */

	   arrout[io++] = fy;
           npi++;
	   npi = npi % nh;
	}

	nl = nd + nh;

/* laatste punt (virtueel) in buffer */
        nd = nd + 2;
	nd = nd % nh;

/* laatste deel */

	for(k = 0; k < np2; k++){
	   fy = 0.;
	   for(m = 0; m < nh; m++){

/* het te vermenigv punt */

	      k1 = nd + m;

/* spiegel */

	      if(k1 >= nl) k1 = 2 * nl - k1;
              if(k1 >= nh) k1 = k1 - nh;
	      fy = fy + bufin[k1] * h[m];
           }
	   nd++;
	   ffy = fy;

/* bereken juiste punt igv even filter */
	   fy = 0.5 * (iadd * fy + isub * oldy);
	   oldy = ffy;

/* filt punt wegschrijven */
	   arrout[io++] = fy;
	}
	free(bufin);
	free(bufout);
} 

void siglib_smooth(
    double arrin[],
    double arrout[],
    int na,
    int nsmo)
/*
 Boxcar filter

 -Input:
      ARRIN (real array) NA values    input data
      H (real array) NH values        impulse response

      NSMO 

 -Output:
      ARROUT (real array) NA values   output data

 Author: L.R.B. Schomaker

*/

{
	int i;
	double *h;
	
	if(nsmo >= 1) {
		h = (double *) malloc( nsmo * sizeof(double));
		for(i = 0; i < nsmo; ++i) {
			h[i] = 1.0/(double) nsmo;
		}
/* LOE: PUT THIS INSIDE IF */
		siglib_fir_filter(arrin, arrout, na, h, nsmo);
		free(h);	
/* LOE_END */
	}
}


void siglib_repeated_smooth(
   double x[], 
   double y[], 
   int n, 
   int nsmo,
   int nrepeat)
{
   int i;
 
 	if (nrepeat>0 && n>1 && nsmo>1) {
		if (n<nsmo)
			nsmo = n;
		for(i = 0; i < nrepeat; ++i) {
			siglib_smooth(x,x,n,nsmo);
			siglib_smooth(y,y,n,nsmo);
		}
	}
}


#define NH_DIFFER 5

void siglib_differ(
double x[],
double dx[],
int n,
double dt)
{
/*

 Differentiation on array (see Dooijes)
 h=[0.083333333,-0.666666667,0.,0.666666667,-0.083333333]

 Boundaries: mirrored (see HELPT ARRFLT)

 -Input:
    X (real array)
    N (integer) number of samples
    DT (real)   delta time, 1/fsample
 -Output:
   DX (real array) (may be the same as x)
   
 -Uses: arrflt()

*/

	double h[NH_DIFFER] = 
	{ 0.083333333, -0.666666667, 0., 0.666666667, -0.083333333};
	
        int i;

	siglib_fir_filter(x,dx,n,h,NH_DIFFER);
	
	for(i=0; i < n; ++i) {
	        dx[i] = dx[i] / dt;
	}
}



/* siglib_get_signal.c */

#define nsignals 16

#define they_want(x) (0 == strcmp(sig,x))

char *siglib_sig_type(int i)
{
	static char *stypes[] = {"SX"
	                        ,"SY"
	                        ,"SC"
	                        ,"P"
	                        ,"VX"
	                        ,"VY"
	                        ,"VA"
	                        ,"DP"
	                        ,"AX"
	                        ,"AY"
	                        ,"AA"
	                        ,"PHI"
	                        ,"DPHI"
	                        ,"SINF"
	                        ,"COSF"
	                        ,"CURV"};

	if(i >= 0 && i < nsignals) {
		return(stypes[i]);
        } else {
        	return("?unknown signal type");
        }
}

int siglib_iget_signam(char *signam)
{
	int i;
	
	for(i = 0; i < nsignals; ++i) {
		if(strcmp(signam,siglib_sig_type(i)) == 0) {
			return(i);
		}
	}
	return(-1);
}


char *siglib_sig_title(char *signam)
{
	static char *stitles[] = {"Sx(t)"
	                        ,"Sy(t)"
	                        ,"Sc(t)"
	                        ,"P(t)"
	                        ,"Vx(t)"
	                        ,"Vy(t)"
	                        ,"Va(t)"
	                        ,"dP(t)"
	                        ,"Ax(t)"
	                        ,"Ax(t)"
	                        ,"Aa(t)"
	                        ,"Phi(t)"
	                        ,"dPHI(t)"
	                        ,"sin(phi(t))"
	                        ,"cos(phi(t))"
	                        ,"Curv(t)"};
	int i;
	
	i = siglib_iget_signam(signam);

	if(i >= 0 && i < nsignals) {
		return(stitles[i]);
        } else {
        	return("?unknown signal type");
        }
}

char *siglib_sig_units(char *signam)
{
	static char *sunits[] = {"mm"         /* "Sx(t)" */
	                        ,"mm"         /* "Sy(t)" */
	                        ,"mm"         /* "Sc(t)" */
	                        ,"g"          /* "P(t)" */
	                        ,"mm/s"       /* "Vx(t)" */
	                        ,"mm/s"       /* "Vy(t)" */
	                        ,"mm/s"       /* "Va(t)" */
	                        ,"g/s"        /* "dP(t)" */
	                        ,"mm/s^2"     /* "Ax(t)" */
	                        ,"mm/s^2"     /* "Ax(t)" */
	                        ,"mm/s^2"     /* "Aa(t)" */
	                        ,"rad"        /* "Phi(t)" */
	                        ,"rad/s"      /* "dPHI(t)" */
	                        ,"u"          /* "sin(phi(t))" */
	                        ,"u"          /* "cos(phi(t))" */
	                        ,"1/mm"       /* "Curv(t)" */
	                        };
	int i;
	
	i = siglib_iget_signam(signam);

	if(i >= 0 && i < nsignals) {
		return(sunits[i]);
        } else {
        	return("?unknown signal type");
        }
}

char *siglib_get_signal(double xin[], double yin[], double zin[],
   char sig[],
   double arr[],
   int n,
   double fsample)
{
/*

 Get signal of a specific type from an XYZ file
 -Input:
    SIG Char
     1       'Sx'     horizontal displacement [mm]
     2       'Sy'     vertical displacement  [mm]
     3       'Sc'     cumulative displacement (trajectory length) [mm]
     4       'P'      axial pen pressure [g]
     5       'Vx'     horizontal velocity [mm/s]
     6       'Vy'     vertical velocity [mm/s]
     7       'Va'     absolute or tangential velocity [mm/s]
     8       'dP'     differentiated pen pressure [g/s]
     9       'Ax'     horizontal acceleration [mm/s]
    10       'Ay'     vertical acceleration [mm/s]
    11       'Aa'     absolute or tangential acceleration [mm/s]
    12       'PHI'    running angle in degrees
    13       'dPHI'   differentiated running angle
    14       'SINF'   sinus of running angle
    15       'COSF'   cosinus of running angle
    16       'Curv'   curvature

    XIN,YIN,ZIN (real arrays) input data
    FSAMPLE (real)

 -Output:
    ARR (real array)
    N (integer) ARR #samples

 Remarks:
    In case of differentiated signals: the first and last two values are
    unreliable. In 2nd derivatives: the first and last four values.

*/


	double *tmpx, *tmpy, *tmpv;
	double dt, dx, dy, dphi, r, avg, vcrit, s;
	static char errmess[1024];
	char *err;
	int i;
	
	err = NULL;
	
        if(n < 1) {
        	return "%siglib_get_signal(No samples)";
        }
        
        tmpx = (double *) malloc(n * sizeof(double));
        tmpy = (double *) malloc(n * sizeof(double));

	if(fsample > 0.0) {
		dt = 1. / fsample;
	} else {
		dt = 1.;
	}

	if(they_want("SX")) {
		for(i = 0; i < n; ++i) {
			arr[i] = xin[i];
		}
		
	} else if(they_want("SY")) {
		for(i = 0; i < n; ++i) {
			arr[i] = yin[i];
		}
		
	} else if(they_want("P")) {
		for(i = 0; i < n; ++i) {
			arr[i] = zin[i];
		}
		
	} else if(they_want("VX")) {
		siglib_differ(xin,arr,n,dt);
		
	} else if(they_want("VY")) {
		siglib_differ(yin,arr,n,dt);
		
	} else if(they_want("VA")) {
		siglib_differ(xin,tmpx,n,dt);
		siglib_differ(yin,tmpy,n,dt);
		for(i = 0; i < n; ++i) {
			arr[i]=sqrt(tmpx[i]*tmpx[i]+tmpy[i]*tmpy[i]);
		}
		
	} else if(they_want("DP")) {
		siglib_differ(zin,arr,n,dt);
		
	} else if(they_want("AX")) {
		siglib_differ(xin,arr,n,dt);
		siglib_differ(arr,arr,n,dt);
		
	} else if(they_want("AY")) {
		siglib_differ(yin,arr,n,dt);
		siglib_differ(arr,arr,n,dt);
		
	} else if(they_want("AA")) {
		siglib_differ(xin,tmpx,n,dt);
		siglib_differ(tmpx,tmpx,n,dt);
		siglib_differ(yin,tmpy,n,dt);
		siglib_differ(tmpy,tmpy,n,dt);
		
		for(i = 0; i < n; ++i) {
			arr[i] = sqrt(tmpx[i]*tmpx[i]+tmpy[i]*tmpy[i]);
		}
		
	} else if(they_want("PHI") || they_want("DPHI")) {
/*
 PHI(t) contains the "running angle" 

 Velocities
*/

		siglib_differ(xin,tmpx,n,dt);
		siglib_differ(yin,tmpy,n,dt);
		siglib_polar(tmpx,tmpy,tmpx,arr,n);
		
		if(they_want("DPHI")) {
			siglib_differ(arr,arr,n,dt);
		}
		
	} else if(they_want("SC")) {
		siglib_differ(xin,tmpx,n,dt);
		siglib_differ(yin,tmpy,n,dt);
		for(i = 0; i < n; ++i) {
			arr[i] = sqrt(tmpx[i]*tmpx[i]+tmpy[i]*tmpy[i]);
		}
/*
 Calculate cumulative displacement by discrete integration of "Va" 
*/
		for(i = 1; i < n; ++i) {
			arr[i] = arr[i] + arr[i-1];
		}

	} else if(they_want("SINF")) {
/*
 Sinus of running angle
*/
		siglib_differ(xin,tmpx,n,dt);
		siglib_differ(yin,tmpy,n,dt);
		
                for(i = 0; i < n; ++i) {
                        dx = tmpx[i];
                        dy = tmpy[i];
 
                        r = sqrt(dx*dx+dy*dy);
                        if(r == 0.0) r=0.000001;
                        arr[i] = dy/r;
                }
                
	} else if(they_want("COSF")) {
/*
 Cosinus of running angle
*/
		siglib_differ(xin,tmpx,n,dt);
		siglib_differ(yin,tmpy,n,dt);
                for(i = 0; i < n; ++i) {
                        dx=tmpx[i];
                        dy=tmpy[i];
 
                        r=sqrt(dx*dx+dy*dy);
                        if(r == 0.0) r=0.000001;
                        arr[i] = dx/r;
                }
                
	} else if(they_want("CURV")) {
/*
 Curvature (by: dPhi/Vabs )
*/

          /* Vx,Vy Velocities */
		siglib_differ(xin,tmpx,n,dt);
		siglib_differ(yin,tmpy,n,dt);
		
          /* Phi */
                tmpv = (double *) malloc(n * sizeof(double));
                
		siglib_polar(tmpx,tmpy,tmpv,arr,n);
                /* tmpv is Vabs now, arr=Phi */
          /* dPhi */
		siglib_differ(arr,arr,n,dt);

          /* Curvature */

		avg = siglib_rmean(tmpv, n);
		vcrit = 0.1 * avg;
		if(vcrit < 0.01) vcrit = 0.01;

		for(i = 0; i < n; ++i) {
			if(tmpv[i] < vcrit) tmpv[i] = vcrit;
			arr[i] = arr[i] / tmpv[i];

			if(arr[i] < 0.0) {
				s = -1.;
				dphi = - arr[i];
			} else {
				s = 1.;
				dphi = arr[i];
			}
			arr[i] = s * sqrt(dphi);
		}

		free(tmpv);

/*
 Another function for curvature is:

               vx * ay - ax * vy
               -----------------
                   vabs^3

 which is numerically even more unstable
 
*/

	} else {
		sprintf(errmess
	       ,"%%siglib_get_signal(), illegal signal type [%s]\n Legal are: "
		       ,sig);
		       
		for(i = 0; i < nsignals; ++i) {
			strcat(errmess,siglib_sig_type(i));
			strcat(errmess," ");
			err = errmess;
		}

	}

	free(tmpx);
	free(tmpy);
	
	return(err);
}

/* end siglib_get_signal.c */

void siglib_spatial_sampler(x,y,n,ii,jj,xo,yo,no,va)
/*  
    Routine for spatial resampling of trajectories. 
    There may be estimation errors (rounding, linear interpolation). 
    These can be reduced by repeatedly (iteratively) calling siglib_spatial_sampler 
    a number of times, say 3-6. Naturally, array arguments x vs xo etc  
    must be swapped: 
 
   siglib_spatial_sampler(x,y,n,xo,yo,no) !1st time  
   siglib_spatial_sampler(xo,yo,no,x,y,no) 
   siglib_spatial_sampler(x,y,no,xo,yo,no) 

    Hints for improvement are welcome.
 
    Author: L.R.B. Schomaker  
    Revised: 17-oct-1991 16:21:25  
*/

double  x[];   /* (input) movement x coordinate */
double  y[];   /* (input) movement y coordinate */
int    n;            /* Number of samples */
double  xo[];  /* (output) spatially normalized movement x coordinate */
double  yo[];  /* (output) spatially normalized movement y coordinate */
int    no;         /* Requested number of output samples */
double  va[];  /* (output) absolute velocity (should be flat) */
int    ii,jj;        /* Beginning and end of strokes */

{
	int i, j;
	double dx, dy, dc, cc, ctot, tc, to;
	double dc_rest, a, b, cco;

	double siglib_interpolate();
	
#ifdef DEBUG_SEVERE
fprintf(stderr,"%%siglib_SPATIAL_SAMPLER nin=%d nout=%d ii=%d jj=%d\n"
                    ,n, no, ii, jj); 
fflush(stderr);
#endif

 
/*  Calculate the deltas va[i] and the total trajectory length CTOT */ 
 	
	va[ii] = 0.0;
	ctot = 0.0;
	for(i=(ii+1); i <= jj; ++i) {
		dx = x[i]-x[i-1];
		dy = y[i]-y[i-1];
		va[i] = sqrt(dx*dx+dy*dy);
                ctot = ctot + va[i];
                if(i >= n) break;
	}
 
/*  Calculate the average delta dC */ 
 
	dc = ctot / (no-1);
	if(dc < 1.0E-10) {
            fprintf(stderr,"%%siglib_spatial_sampler, zero trajectory length\n");
            fflush(stderr);
            return;
	}

/* debug 
  fprintf(stderr,"Total delta displacement: %f\n", ctot);
  fprintf(stderr,"Average delta displacement: %f n=%d\n", dc, no);
*/
 
/*  Spatially resample. The sample length variable is cc which is  */ 
/*  continually incremented by Va[i] until it exceeds dC */
 
	cco = 0.;
	cc = va[ii];
	tc = 0.;
	to = 0.;

	xo[0] = x[ii];
	yo[0] = y[ii];

	j=1;
	i=ii;
	while(i < (jj+1)) {
                if (cc >= dc) {
 
/*  Calculate the actual time of reaching dC. */ 
/*  Er is een lineaire functie cc(t) = a t + b die voorspelt */
/*  wat de waarde van cc is als functie van de tijd. De inverse: */
/*   t = (cc' - b) / a zegt mij op welk tijdstip cc(t) de waarde cc' */
/*  bereikt */
 
/*  a = cc(t) - cc(t-1) / (1) */ 
/*  b = cc(t-1) - a * (1) */
 
/*  */ 
                       dc_rest = cc - dc;
 
                       a = (cc - cco) / ((double) i - to) ;
                       b = cco - a * to;
 
                       tc = (dc - b) / a ;
 
/*  and interpolate */ 
 
                       xo[j] = siglib_interpolate(x,n,tc);
                       yo[j] = siglib_interpolate(y,n,tc);
 
/*  The reservoir CC is set to the remainder and the output index */ 
/*  is incremented */
 
                       cc = dc_rest;
                       cco = 0.;
                       to = tc;
                       ++j;
                       if(j >= no) break;
                } else {
                       cco = cc;
                       to = (double) i;
                       ++i;
                       cc = cc + va[i];
                }
	}
 
	if(j < no) { 
                xo[no-1] = x[jj];
                yo[no-1] = y[jj];
	}
} /* End siglib_spatial_sampler */

void siglib_spatial_z_sampler(x,y,z,n,ii,jj,xo,yo,zo,no,va,ts,prsmin)
/*  
    Routine for spatial resampling of trajectories. Includes z[].
    NOTE: z[] is thresholded 0 (penup) or FEATCHAR_PENDOWN (pendown)
    

    Author: L.R.B. Schomaker  
    Revised: 17-oct-1991 16:21:25  
*/

#ifdef ARTIFICIAL_Z_SIGNAL
#define FEATCHAR_PENDOWN  2.0
#define FEATCHAR_PENUP    0.0
#endif


double  x[];   /* (input) movement x coordinate */
double  y[];   /* (input) movement y coordinate */
double  z[];   /* (input) movement z coordinate */
int    n;            /* Number of samples */
double  xo[];  /* (output) spatially normalized movement x coordinate */
double  yo[];  /* (output) spatially normalized movement y coordinate */
double  zo[];  /* (output) spatially normalized movement z coordinate */
int    no;         /* Requested number of output samples */
double  va[];  /* (output) absolute velocity (should be flat) */
int    ii,jj;        /* Beginning and end of strokes */
double  ts[];  /* (output) time of original sample points */
double prsmin;
{
	int i, j;
	double dx, dy, dc, cc, ctot, tc, to;
	double dc_rest, a, b, cco;

	double siglib_interpolate();
	
#ifdef DEBUG_SEVERE
fprintf(stderr,"%%siglib_SPATIAL_Z_SAMPLER nin=%d nout=%d ii=%d jj=%d\n"
                    ,n, no, ii, jj); 
fflush(stderr);
#endif

 
/*  Calculate the deltas va[i] and the total trajectory length CTOT 
    of the ink */ 
 	
	va[ii] = 0.0;
	ctot = 0.0;
	for(i=(ii+1); i <= jj; ++i) {
	    if(z[i] > prsmin) {
		dx=x[i]-x[i-1];
		dy=y[i]-y[i-1];
		va[i] = sqrt(dx*dx+dy*dy);
                ctot = ctot + va[i];
                if(i >= n) break;
            } else {
            	va[i] = 0.0;
            }
	}
 
/*  Calculate the average delta dC */ 
 
	dc = ctot / (no-1);
	if(dc < 1.0E-10) {
            fprintf(stderr,"%%siglib_spatial_sampler, zero trajectory length\n");
            fflush(stderr);
            return;
	}

/* debug 
  fprintf(stderr,"Total delta displacement: %f\n", ctot);
  fprintf(stderr,"Average delta displacement: %f n=%d\n", dc, no);
*/
 
/*  Spatially resample. The sample length variable is cc which is  */ 
/*  continually incremented by Va[i] until it exceeds dC */
 
	cco = 0.;
	cc = va[ii];
	tc = 0.;
	to = 0.;

	xo[0] = x[ii];
	yo[0] = y[ii];
	zo[0] = z[ii];
#ifdef ARTIFICIAL_Z_SIGNAL
	if(zo[0] > prsmin) {
		zo[0] = FEATCHAR_PENDOWN;
	} else {
		zo[0] = FEATCHAR_PENUP;
	}
#endif

	j=1;
	i=ii;
	while(i < (jj+1)) {
                if (cc >= dc) {
 
/*  Calculate the actual time of reaching dC. */ 
/*  Er is een lineaire functie cc(t) = a t + b die voorspelt */
/*  wat de waarde van cc is als functie van de tijd. De inverse: */
/*   t = (cc' - b) / a zegt mij op welk tijdstip cc(t) de waarde cc' */
/*  bereikt */
 
/*  a = cc(t) - cc(t-1) / (1) */ 
/*  b = cc(t-1) - a * (1) */
 
/*  */ 
                       dc_rest = cc - dc;
 
                       a = (cc - cco) / ((double) i - to) ;
                       b = cco - a * to;
 
                       tc = (dc - b) / a ;
 
/*  and interpolate */ 
 
                       xo[j] = siglib_interpolate(x,n,tc);
                       yo[j] = siglib_interpolate(y,n,tc);
                       zo[j] = siglib_interpolate(z,n,tc);
                       ts[j] = tc;
                       
#ifdef ARTIFICIAL_Z_SIGNAL
                       if(zo[j] > prsmin) {
                       	     zo[j] = FEATCHAR_PENDOWN;
                       } else {
                       	     zo[j] = FEATCHAR_PENUP;
                       }
#endif
 
/*  The reservoir CC is set to the remainder and the output index */ 
/*  is incremented */
 
                       cc = dc_rest;
                       cco = 0.;
                       to = tc;
                       ++j;
                       if(j >= no) break;
                } else {
                       cco = cc;
                       to = (double) i;
                       ++i;
                       cc = cc + va[i];
                }
	}
 
	if(j < no) { 
                xo[no-1] = x[jj];
                yo[no-1] = y[jj];
                zo[no-1] = z[jj];
                
                ts[no-1] = (double) jj;
#ifdef ARTIFICIAL_Z_SIGNAL
                if(zo[no-1] > prsmin) {
                     zo[no-1] = FEATCHAR_PENDOWN;
                } else {
                     zo[no-1] = FEATCHAR_PENUP;
                }
#endif
       	}
} /* End siglib_spatial_z_sampler */

double siglib_interpolate(r,n,rn)
 
/*
     linear interpolation of value in double array
     at (doubleing) index rn

*/
double  r[];   /* Array in which to interpolate */
int    n;     /* Number of values in array r[] */
double rn;    /* Index to array r as doubleing value. */

/* double Function result: Interpolated value r[rn] */

{
       double ret_val, d;
       int i1, i2;
      
/* test */ 
 
       if(rn < 0.0 || rn > (double) (n-1) + 0.01) { 
         fprintf(stderr,
              "%%siglib_interpolate (warn) rn outside [0,%d]: %f\n",
			n-1,rn);
	 fflush(stderr);
       }
 
       if(rn <= 0) { 
                ret_val = r[0];
       } else if(rn >= n-1) {
                ret_val = r[n-1];
       } else {
                i1 = (int) rn;
                i2 = i1 + 1;
                d = rn - (double) i1;
                ret_val = (1.-d) * r[i1] + d * r[i2];
       }
       return(ret_val);
}

#ifdef MAINTEST

#define FSAMP   1.     /* test & demo */
#define NMAX    10000
#define NREPEAT 3



void read_hwr(
   double x[NMAX], 
   double y[NMAX], 
   double z[NMAX], 
   int *n)
{
	int i, ix, iy, iz;
	
	i = 0;
	while(scanf("%d%d%d\n", &ix,&iy,&iz) != EOF) {
		x[i] = (double) ix;
		y[i] = (double) iy;
		z[i] = (double) iz;
		++i;
	}
	*n = i;
}

void main(
int argc,
char *argv[])
{
	double x[NMAX], y[NMAX], z[NMAX], arr[NMAX], arr2[NMAX];
	double xo[NMAX], yo[NMAX], zo[NMAX], va[NMAX], ts[NMAX];
	double prsmin = 15.;
	char *p1, *p2, *sig1, *sig2;
	int i, n, no;
	int nsmo;
	
        if(argc != 4) {
        	fprintf(stderr
        	,"Usage: siglib SIGNAM nsmo sampmode[t or s]\n");
        	exit(1);
        }
	
	nsmo = atoi(argv[2]);

        read_hwr(x,y,z,&n);	
	
        siglib_repeated_smooth(x,y,n,nsmo,NREPEAT);
        
        if(strcmp(argv[3],"s") == 0) {
           no = n;
           siglib_spatial_z_sampler(x,y,z,n,0,n-1,xo,yo,zo,no,va,ts,prsmin);
           for(i = 0; i < n; ++i) { 
               x[i] = xo[i];
               y[i] = yo[i];
               z[i] = zo[i];
           } 
        }

        if(strcmp("HWR", argv[1]) == 0) {	
	    for(i = 0; i < n; ++i) {
		printf("%.0f %.0f %.0f\n", x[i], y[i], z[i]);
            }
            
        } else if((p1 = strchr(argv[1],'(')) != NULL && 
                  (p2 = strchr(argv[1],')')) != NULL &&
                    p2 > p1) {

            *p1 = (char) 0;
            sig1 = argv[1];
            
            *p2 = (char) 0;
            sig2 = p1 + 1;
            
            siglib_get_signal(x, y, z, sig1, arr, n, FSAMP);
            siglib_get_signal(x, y, z, sig2, arr2, n, FSAMP);

            for(i = 0; i < n; ++i) {
	       printf("%f %f\n", arr2[i], arr[i]);
	    }
	    
	} else {	
	
            siglib_get_signal(x, y, z, argv[1], arr, n, FSAMP);
            
	    for(i = 0; i < n; ++i) {
		printf("%d. %f\n", i, arr[i]);
	    }
	} 
}
#endif

