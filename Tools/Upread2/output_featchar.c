#include <stdio.h>
#include <stdlib.h>
#include <uplib.h>
#include <upsiglib.h>

#include "allo_resample.h"
#include "unipen_identifications.h"
#include "upread2.h"

void output_allograph (FILE *fp, char *name, int ns, int m, int *xi, int *yi, int *zi)
{
	float x[NSMAX];
	float y[NSMAX];
	float z[NSMAX];
	float xs[NSMAX], ys[NSMAX], zs[NSMAX], va[NSMAX];
	double X[MAX_RESAMPLE_POINTS*3];
	int offset, i, no;

	/* remove pen_up head */
	offset = 0;
	while (zi[offset]<15.) {
		if (offset==ns-1) {
			return;
		}
		offset++;
	}

	/* remove pen-up tail */
	ns = ns - 1;
	while (zi[ns]<15.&&ns>offset) {
		ns--;
	}

	ns = ns+1-offset;
	for (i=0;i<ns;i++) {
		x[i] = (float) xi[i+offset];
		y[i] = (float) yi[i+offset];
		z[i] = (float) zi[i+offset];
	}
	no = m;

	recog_spatial_z_sampler(x,y,z,ns,xs,ys,zs,no,va,0,ns-1,15.);
	for (i=0;i<no;i++) {
		X[3*i]   = (double) xs[i];
		X[3*i+1] = (double) ys[i];
		X[3*i+2] = (double) zs[i];
	}
	normalize_allo(X,no);

	fprintf (fp,"%s",name);
	for (i=0;i<no*3;i++) {
		fprintf (fp," %f",X[i]);
	}
	fprintf (fp,"\n");
}

void init_output_featchar (FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo)
{
	static int first = 1;
	int m;
	 
	if (!first)
		return;
	m = 3*oinfo->m;
	if (oinfo->do_add_sincos)
		m += 2*(oinfo->m-1);
	if (oinfo->do_add_phi)
		m += 2*(oinfo->m-2);
	fprintf (fp_out,"NAMED DATA ??? %d\n",m);
	first = 0;
}

char *my_name (tUPEntry *entry)
{
	static char result[256];
	char *name = upEntryName(entry);
	strcpy(result,name);
	free(name);
	strcat(result,"??????");
	return result;
}

int output_featchar (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams)
{
	tUPEntry *entry,**segment_entries;
	int i,ns, *xi,*yi,*zi;
	char *name;
	sigSignal *sig;

	segment_entries = pUnipen->Entries[pUnipen->SegmentId];
	for (i=0;i<nentries;i++) {
		entry = entries[i];
#ifdef _USE_CHAR_SIGNAL_
		sigCharStream2CharSignal (pUnipen,streams[i],&ns,&xi,&yi,&zi);
#else
		sig = sigCharstream2Signal (pUnipen,entry,TIME_EQUI_DIST,streams[i]);
		ns = sig->nsamples;
		xi = sig->x;
		yi = sig->y;
		zi = sig->z;
#endif

#ifdef USE_WID
		name = updirEntry2UnipenCode(entry
			,entries
			,pUnipen->NrOfEntries[pUnipen->SegmentId]
			,oinfo->writer_code,oinfo->char_only);
#else
	name = my_name(entry);
#endif
		if (ns>MAX_VALID_SAMPLES) {
			fprintf (stderr,"nsamples %d too large for char [%d]=%s! (skipping)\n",ns,i,name);
		} else {
			output_allograph(fp_out,name,ns,oinfo->m,xi,yi,zi);
		}
#ifdef _USE_CHAR_SIGNAL_
		sigDeleteSignal(sig);
#else
		free(xi);
		free(yi);
		free(zi);
#endif
	}
	return 1;
}
