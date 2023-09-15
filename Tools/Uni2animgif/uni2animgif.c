#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include <uplib.h>
#include <upsiglib.h>
#include <up_segment_io.h>

#include "examine_gif.h"
#include "scale.h"
#include "gifmerge_patch.h"

#define EMPTY   ((char) 0)
#define NCHAR   200
#define PU      0
#define PD      1
#ifndef _PUFNAME 
#define _PUFNAME "perlred-medium.gif"
#endif
#ifndef _PDFNAME 
#define _PDFNAME "perlgreen-medium.gif"
#endif
#ifndef _BACKFNAME 
#define _BACKFNAME "nici_copyright.gif"
#endif

static char *UNIPEN2ANIMGIF_PATH = NULL;

void usage_abort()
{
  fprintf(stderr,"Usage: unipen2gifanim unipen-file [unipen-files] [options] [output.gif]\n");
  fprintf(stderr,"Options:\n");
  fprintf(stderr,"	-S  <scale>                  scale in both directions\n");
  fprintf(stderr,"	-x  <xoff>                   translate horizontally after scaling\n");
  fprintf(stderr,"	-y  <yoff>                   translate vertically after scaling\n");
  fprintf(stderr,"	-m  <margin>                 surrounding margin (between [0,1])\n");
  fprintf(stderr,"	-a  <alpha>                  RC filter alpha\n");
  fprintf(stderr,"	-L  <loops>                  number of loops in display\n");
  fprintf(stderr,"	-n  <subsampler>             only take each n-th sample\n");
  fprintf(stderr,"	-d  <dt in ms>               between frames\n\n");
  fprintf(stderr,"	-t  <threshold>              ink pressure\n\n");
  fprintf(stderr,"	-b  <background.gif>         the image file of the canvas\n");
  fprintf(stderr,"	-pd <pendown.gif|no_pendown> the image to use for pendown samples\n");
  fprintf(stderr,"	-pu <penup.gif|no_penup>     the image to use for penup samples\n");
  fprintf(stderr,"	-o  <output.gif>             the output file\n");
  fprintf(stderr,"	-s  <query>                  string to search for in UNIPEN file\n");
  fprintf(stderr,"	-l  <level>                  level to search for in UNIPEN file\n");
  fprintf(stderr,"\nUsage example: unipen2gifanim data/test.dat -l CHAR -s data/test.dat a -o dum.gif\n");
  exit(1);
}

typedef struct {
	int dt;
	int nloop;
	int subsample;
	int threshold;
	char background[NCHAR];
	char pdfname[NCHAR];
	char pufname[NCHAR];
	char outfile[NCHAR];
	FILE *fp_out;
} GifMergeInfo;

void init_gif_merge (GifMergeInfo *info)
{
	GifMergeInstall (info->background,info->fp_out, info->dt, info->nloop);
}


void try2open (char *msg, char fname[NCHAR])
{
	FILE *fp;
	char tmpfile[NCHAR];

	if ((fp=fopen(fname,"rb"))!=NULL) {
		fclose(fp);
		fprintf(stderr,"using %s as %s\n",fname,msg);
		return;
	}
	sprintf (tmpfile,"%s/%s",UNIPEN2ANIMGIF_PATH,fname);
	strcpy(fname,tmpfile);
	fprintf(stderr,"trying %s as %s\n",fname,msg);
	if ((fp=fopen(fname,"rb"))!=NULL) {
		fclose(fp);
		return;
	} else {
		fprintf(stderr,"unable to open %s!\n",fname);
		exit(1);
	}
}

void parse_args(argc, argv, scale_fact, xoff, yoff, margin, alpha, info, qinfo)
int argc;
char *argv[];
double *scale_fact;
double *xoff;
double *yoff;
double *margin;
double *alpha;
GifMergeInfo *info;
upsegQueryInfo *qinfo;
{
	int i;	

	if ((UNIPEN2ANIMGIF_PATH=getenv("UNIPEN2ANIMGIF_PATH"))==NULL) {
		fprintf (stderr,"environment variable UNIPEN2ANIMGIF_PATH not set!\n");
		fprintf (stderr,"this is needed for uni2animgif to find its gif-pictures\n");
		fprintf (stderr,"typically, you would set it to $UPTOOLS/Tools/Uni2animgif/gifs\n");
		exit(1);
	}
	*scale_fact = 1.0;
	*xoff = 0.0;
	*yoff = 0.0;
	*margin = .1;
	*alpha = 1.;
	info->subsample = 1;
	info->nloop = -1;
	info->dt = 5; /* ms */
	info->threshold = 1;
	sprintf(info->background,"%s/%s",UNIPEN2ANIMGIF_PATH,_BACKFNAME);
	sprintf(info->pdfname,"%s/%s",UNIPEN2ANIMGIF_PATH, _PDFNAME);
	sprintf(info->pufname,"%s/%s",UNIPEN2ANIMGIF_PATH, _PUFNAME);
	sprintf(info->outfile,"tmp.gif");

	if (argc<2)
		usage_abort();

	i = 1;
	while(i < argc) {
		if (strcmp(argv[i],"-s")==0) {
			i++;
			upsegAddQuery(qinfo,argv[i]);
		} else if (strcmp(argv[i],"-l")==0) {
			i++;
			strcpy(qinfo->level,argv[i]);
			fprintf (stderr,"level: %s\n",qinfo->level);
		} else if (strcmp(argv[i],"-S")==0) {
			i++;
			*scale_fact = atof(argv[i]);
		} else if (strcmp(argv[i],"-x")==0) {
			i++;
			*xoff = atof(argv[i]); 
		} else if (strcmp(argv[i],"-y")==0) {
			i++;
			*yoff = - atof(argv[i]); 
		} else if (strcmp(argv[i],"-m")==0) {
			i++;
			*margin = atof(argv[i]); 
		} else if (strcmp(argv[i],"-a")==0) {
			i++;
			*alpha = atof(argv[i]); 
		} else if (strcmp(argv[i],"-t")==0) {
			i++;
			info->threshold = atoi(argv[i]); 
		} else if (strcmp(argv[i],"-L")==0) {
			i++;
			info->nloop = atoi(argv[i]); 
		} else if (strcmp(argv[i],"-n")==0) {
			i++;
			info->subsample = atoi(argv[i]); 
		} else if (strcmp(argv[i],"-d")==0) {
			i++;
			info->dt = atoi(argv[i]); 
		} else if (strcmp(argv[i],"-pd")==0) {
				++i;
				if(i < argc) {
					strcpy(info->pdfname,argv[i]);
				} else {
					fprintf(stderr,"Missing -pd pendown.gif or no_pendown\n");
				}
				if(strcmp(info->pdfname,"no_pendown") == 0) {
					info->pdfname[0] = EMPTY;
				} else {
					try2open("pen_down image",info->pdfname);
				}
		} else if (strcmp(argv[i],"-pu")==0) { 
				++i;
				if(i < argc) {
					strcpy(info->pufname,argv[i]);
				} else {
					fprintf(stderr,"Missing -pu penup.gif or no_penup\n");
				}
				if(strcmp(info->pufname,"no_penup") == 0) {
					info->pufname[0] = EMPTY;
				} else {
					try2open("pen_up image",info->pufname);
				}
		} else if (strcmp(argv[i],"-o")==0) {
			++i;
			if(i < argc) {
				strcpy(info->outfile,argv[i]);
			} else {
				fprintf(stderr,"Missing -o outputfile\n");
		     	usage_abort();
			}
		} else if (strcmp(argv[i],"-b")==0) {
			++i;
			if(i < argc) {
				strcpy(info->background,argv[i]);
				try2open("background image",info->background);
			} else {
				fprintf(stderr,"Missing -b background gif file\n");
		     	usage_abort();
			}
		} else {
			if (argv[i][0]=='-') {
		     fprintf (stderr,"unknown option '%s'!\n",argv[i]);
			  usage_abort();
			}
			upsegAddFile(qinfo,argv[i]);
		}
		++i;
	}
	fprintf(stderr,"Using %s as background, output is %s\n"
	              ,info->background,info->outfile);
	if ((info->fp_out=fopen(info->outfile,"r"))!=NULL) {
		fprintf (stderr,"outfile '%s' already exists!\n",info->outfile);
		fclose(info->fp_out);
		exit(1);
	}
	if ((info->fp_out=fopen(info->outfile,"w"))==NULL) {
		fprintf (stderr,"unable to open outfile '%s'!\n",info->outfile);
		exit(1);
	}
}

void append_sample (GifMergeInfo *info, int x, int y, int z)
{
	if (z>=info->threshold) {
		GifMergeFile (info->pdfname,info->fp_out,x,y);
	}
	else {
		GifMergeFile (info->pufname,info->fp_out,x,y);
	}
}

int main (int argc, char *argv[])
{
	int i,j;
	double scale_fact,xo,yo,xoff, yoff, alpha;
	double *X,*Y;
	int nsamples;
	double xmin,xmax,ymin,ymax;
	GifMergeInfo gifmerge_info;
	int width,height;
	double margin;
	static upsegQueryInfo info;
	sigSignal *sig,**signals;
	int nallocated,nsignals;
	char **names;

	if(argc < 2) {
		usage_abort();
	}

	upsegInitializeQueryInfo(&info);
	parse_args(argc, argv, &scale_fact, &xoff, &yoff, &margin, &alpha, &gifmerge_info,&info);
	signals = upsegGetSignals(&info,&nsignals,&names);
	fprintf (stderr,"got %d signals, merging as individual gifs\n",nsignals);

	if (!examine_gif(gifmerge_info.background,&width,&height))
		exit(1);

	nallocated = 0;
	X = Y = NULL;
	init_gif_merge(&gifmerge_info);
	for (j=0;j<nsignals;j++) {
		if (j!=0) {
			GifMergeReset(gifmerge_info.background,gifmerge_info.fp_out);
		}
		sig = signals[j];
		fprintf (stderr,"SIGNAL[%d] (%s): %d samples\n",j,names[j],sig->nsamples);
		nsamples = sig->nsamples;
		if (nsamples<=0) {
			fprintf (stderr,"nsamples %d too small\n",nsamples);
			exit(1);
		}
		if (nsamples>nallocated) {
			nallocated = nsamples;
			if (X==NULL) {
				X = (double *) malloc (nsamples*sizeof(double));
				Y = (double *) malloc (nsamples*sizeof(double));
			}
			else {
				X = (double *) realloc (X,nsamples*sizeof(double));
				Y = (double *) realloc (Y,nsamples*sizeof(double));
			}
		}

		xo = X[0] = scale_fact*sig->x[0];
		yo = Y[0] = scale_fact*sig->y[0];
		xmax = xmin = X[0];
		ymax = ymin = Y[0];
		for (i=1;i<nsamples;i++) {
			X[i] = scale_fact*sig->x[i];
			Y[i] = scale_fact*sig->y[i];
			if (alpha>=0.0&& alpha<1.0) {
				xo = X[i] = alpha*X[i] + (1.0 - alpha)*xo;
				yo = Y[i] = alpha*Y[i] + (1.0 - alpha)*yo;
			}
			if (X[i]<xmin) xmin = X[i];
			if (X[i]>xmax) xmax = X[i];
			if (Y[i]<ymin) ymin = Y[i];
			if (Y[i]>ymax) ymax = Y[i];
		}

		hwr_scale_points(nsamples,X,Y,xmin,xmax,ymin,ymax
			,(double)scale_fact*width
			,(double)scale_fact*height
			,xoff+(double)width/2.0
			,yoff+(double)height/2.0
			,margin);
		for (i=0;i<nsamples;i++) {
			if (i%gifmerge_info.subsample==0)
				append_sample (&gifmerge_info,(int)X[i],(int)Y[i],(sig->z[i]>1.0));
		}
		sigDeleteSignal(sig);
		free(names[j]);
	}
	GifMergeClose(gifmerge_info.fp_out);
	free(names);
	free(signals);
	free(X);
	free(Y);
	return 0;
}


/*
			fprintf(fp,"-pos%.0f,%.0f %s \\\n"
				, X[i], Y[i], dot[PD]);
		}
		if (j!=nsignals-1) {
			fprintf(fp,"-d2 -pos0,0 %s -d1 -pos0,0 %s\\\n "
				,background,background);
		}
	}
*/

/* **********
			sscanf(buf,"%d%d", &ix, &iy);
		}
	}
			
			if(subsample < 2 ||
			        ((i % subsample) == 1)) {
			    if(dot[ipen][0] != EMPTY) {
			        fprintf(fp,"-pos%.0f,%.0f %s \\\n"
			           , x*scale_fact, yoff-y*scale_fact, dot[ipen]);
			    }
			}
		} else {
			if(ipen == PD) {
				ipen = PU;
			} else {
				ipen = PD;
			}
		}
	} 
**********/
