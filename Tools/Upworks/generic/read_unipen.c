/**************************************************************************
*																		 *
*  UNIPEN PROJECT														 *
*																		 *
*	(c) Nijmegen Institute for Cognition and Information				 *
*																		 *
***************************************************************************
*																		 *
*  AUTHORS:															   *
*																		 *
*	Gerben H. Abbink, Lambert Schomaker and Louis Vuurpijl			   *
*																		 *
*  DISCLAIMER:															*
*																		 *
*	USER SHALL BE FREE TO USE AND COPY THIS SOFTWARE FREE OF CHARGE OR   *
*	FURTHER OBLIGATION.												  *
*																		 *
*	THIS SOFTWARE IS NOT OF PRODUCT QUALITY AND MAY HAVE ERRORS OR	   *
*	DEFECTS.															 *
*																		 *
*	PROVIDER GIVES NO EXPRESS OR IMPLIED WARRANTY OF ANY KIND AND ANY	*
*	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR PURPOSE ARE	*
*	DISCLAIMED.														  *
*																		 *
*	PROVIDER SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL,	  *
*	INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THIS   *
*	SOFTWARE.															*
*																		 *
**************************************************************************/

#include <stdio.h>
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif
#include "uplib.h"
#include "upsiglib.h"
#include "uphierarchy.h"
#include "jump_through_segments.h"
#include <string.h>
#include <ctype.h>
#include <tcl.h>

/*
 * Default directory in which to look for libraries:
 */

static char defaultLibraryDir[200] = UNIPEN_TCL_LIBRARY;

#if defined(__WIN32__)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   undef WIN32_LEAN_AND_MEAN

/*
 * VC++ has an alternate entry point called DllMain, so we need to rename
 * our entry point.
 */

#   if defined(_MSC_VER)
#	define EXPORT(a,b) __declspec(dllexport) a b
#	define DllEntryPoint DllMain
#   else
#	if defined(__BORLANDC__)
#		define EXPORT(a,b) a _export b
#	else
#		define EXPORT(a,b) a b
#	endif
#   endif
#else
#   define EXPORT(a,b) a b
#endif

extern EXPORT(int,Upworks_Init) _ANSI_ARGS_((Tcl_Interp *interp));


/*
 *----------------------------------------------------------------------
 *
 * DllEntryPoint --
 *
 *	This wrapper function is used by Windows to invoke the
 *	initialization code for the DLL.  If we are compiling
 *	with Visual C++, this routine will be renamed to DllMain.
 *	routine.
 *
 * Results:
 *	Returns TRUE;
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#ifdef __WIN32__
BOOL APIENTRY
DllEntryPoint(hInst, reason, reserved)
	HINSTANCE hInst;		/* Library instance handle. */
	DWORD reason;		/* Reason this function is being called. */
	LPVOID reserved;		/* Not used. */
{
	return TRUE;
}
#endif

/**************************************************************/
#define TCL_BUFSZ 128
#define TCL_NBUFS  24

static char *nextbuf (void)
{
	static int i = -1;
	static char buffer[TCL_NBUFS][TCL_BUFSZ];

	i = (i+1)%TCL_NBUFS;
	return buffer[i];
}

static char *i2s (int n)
{
	char *result = nextbuf();
	sprintf(result," %d ",n);
	return result;
}

static char *d2s (double d)
{
	char *result = nextbuf();
	sprintf(result," %f ",d);
	return result;
}


#define check_args(n,msg) {\
	if (argc!=n) {\
		fprintf (stderr,"use: %s %s!\ngot:\n",argv[0],msg);\
		Tcl_AppendResult (interp,"use: ",argv[0]," ",msg,NULL);\
		for (i=0;i<argc;i++) {\
			fprintf (stderr,"  %s\n",argv[i]);\
		}\
		return 0;\
	}\
}

#define check_unipen(msg) {\
	if (pUnipen==NULL) {\
		Tcl_AppendResult (interp,msg,": FIRST open unipenfile!",NULL);\
		return 1;\
	}\
}

#define set_hierarchy_info(e) {\
	h       = &hierarchies[level].segments[segnr];\
	entry   = h->entry;\
	may_de_administrate(h);\
	if ((children=hierFindChildren(pUnipen,hierarchies,level,segnr))==NULL) {\
		if (e) {\
			Tcl_AppendResult(interp,"child ",i2s(idx),"does not exist for ",i2s(level),i2s(segnr),NULL);\
			Tcl_AppendResult(interp,"Maybe you are viewing a low-level hierarchy with 'show_subsegments' set?",NULL);\
			return TCL_ERROR;\
		}\
		else {\
			Tcl_AppendResult(interp,"0",NULL);\
			return TCL_OK;\
		}\
	}\
	n = h->nchildren;\
	if (idx<0||idx>=n) {\
		Tcl_AppendResult(interp,"child ",i2s(idx),"does not exist for ",i2s(level),i2s(segnr),NULL);\
		Tcl_AppendResult(interp,"Maybe you are viewing a low-level hierarchy with 'show_subsegments' set?",NULL);\
		return TCL_ERROR;\
	}\
}

static tUPUnipen *pUnipen	= NULL;
static tUPLevel *hierarchies = NULL;

static int check_level (char *arg, Tcl_Interp *interp, int *level)
{
	*level = atoi(arg);
	if (*level<0||*level>=pUnipen->NrOfLevels) {
		fprintf (stderr,"level %d should lay within [0,%d]!\n",*level,pUnipen->NrOfLevels);
		Tcl_AppendResult (interp,"level ",i2s(*level)
			," should lay within [0,",i2s(pUnipen->NrOfLevels),"]",NULL);
		return 0;
	}
	return 1;
}

static int check_segment (char *arg, Tcl_Interp *interp, int level, int *segnr)
{
	if (hierarchies==NULL) {
		hierarchies = hierCreateLevels (pUnipen);
	}
	*segnr = atoi(arg);
	if (*segnr<0||*segnr>=hierarchies[level].nsegments) {
		fprintf (stderr,"segnr %d in level %d should lay within [0,%d]!\n"
			,*segnr,level,hierarchies[level].nsegments);
		Tcl_AppendResult (interp,"segnr ",i2s(*segnr),"at level "
			,i2s(level)," should lay within [0,",i2s(hierarchies[level].nsegments),"]",NULL);
		return 0;
	}
	return 1;
}


static int closeUnipen (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	if (pUnipen==NULL)
		return TCL_OK;
	if (hierarchies!=NULL) {
		hierDeleteLevels (pUnipen,hierarchies);
		hierarchies = NULL;
	}
	upDelUnipen(pUnipen);
	pUnipen = NULL;
	return TCL_OK;
}

static int openUnipen (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i;
	
	check_args(2,"UNIPEN-file");

	if (pUnipen==NULL) {
		pUnipen = upNewUnipen("unipen.def");
	}
	else {
		if (hierarchies!=NULL) {
			hierDeleteLevels (pUnipen,hierarchies);
			hierarchies = NULL;
		}
		closeUnipen(cd,interp,argc,argv);
	}

	if (!upNextFile(pUnipen,argv[1])) {
		fprintf (stderr,"\nread_unipen/openUnipen: unable to open unipenfile '%s' correctly!\n",argv[1]);
		Tcl_AppendResult (interp,"unable to open unipenfile!",NULL);
		return 1;
	}
	hierarchies = hierCreateLevels (pUnipen);
	Tcl_AppendResult (interp,i2s(pUnipen->NrOfLevels),NULL);
	for (i=0;i<pUnipen->NrOfLevels;i++) {
		Tcl_AppendResult (interp,pUnipen->Levels[i]," ",NULL);
	}
	return TCL_OK;
}

static char *change_braces (char *name)
{
	static char *result = NULL;
	static int rlength = 0;
	int i,j;

	if (result==NULL) {
		rlength = 2*strlen(name)+2;
		result = (char *) ckalloc (rlength);
	} else if (rlength<strlen(name)) {
		rlength = 2*strlen(name)+2;
		result = (char *) realloc (result,rlength);
	}
	j = 0;
	if (name[0]=='{' || name[0]=='}' || name[0]=='\\') {
		result[j++] = '\\';
	}
	result[j++] = name[0];
	for (i=1;i<strlen(name);i++) {
		switch (name[i]) {
			case '{': case '}': case '\\':
				result[j++] = '\\';
			default:
				break;
		}
		result[j++] = name[i];
	}
	result[j] = '\0';
	return result;
}

static int getSegmentNames (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int level,i;
	int fstr,fsam,lstr,lsam;
	char delbuf[256];
	sigDelineation *del;

	check_unipen("getSegmentNames");
	check_args(2,"level");
	if (!check_level(argv[1],interp,&level))
		return 1;
	Tcl_AppendResult(interp,i2s(hierarchies[level].nsegments),NULL);
	for (i=0;i<hierarchies[level].nsegments;i++) {
		del = hierarchies[level].segments[i].del;
		fstr = del->delineations[0].first_streamnr;
		fsam = del->delineations[0].first_samplenr;
		lstr = del->delineations[del->ndels-1].last_streamnr;
		lsam = del->delineations[del->ndels-1].last_samplenr;
		sprintf (delbuf,"} %d:%d-%d:%d",fstr,fsam,lstr,lsam);
		Tcl_AppendResult (interp,"{{",change_braces(hierarchies[level].segments[i].name),delbuf,"} ",NULL);
	}
	return TCL_OK;
}

static int resetSegments (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	upResetSegments(pUnipen);
	return TCL_OK;
}

static int getFullSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	sigDelineation *del;
	tUPHierarchy *h;
	sigSignal *sig;
	char buf[128];
	int i,level,segnr;

	check_unipen("getSegment");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	h = &hierarchies[level].segments[segnr];
	del = sigCreateBoundingDelineation(h->del);
	sig = sigDelineation2Signal(pUnipen,del);
	sigFreeDelineation(del);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,h->entry->Entry,")!",NULL);
		return TCL_ERROR;
	}
	for (i=0;i<sig->nsamples;i++) {
		sprintf (buf," %d %d %d",sig->x[i],sig->y[i],sig->z[i]);
		Tcl_AppendResult(interp,buf,NULL);
	}
	sigDeleteSignal(sig);
	return TCL_OK;
}

static int getSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *h;
	sigSignal *sig;
	char buf[128];
	int i,level,segnr;

	check_unipen("getSegment");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;
	h = &hierarchies[level].segments[segnr];
	sig = sigDelineation2Signal(pUnipen,h->del);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,h->entry->Entry,")!",NULL);
		return TCL_ERROR;
	}
	for (i=0;i<sig->nsamples;i++) {
		sprintf (buf," %d %d %d",sig->x[i],sig->y[i],sig->z[i]);
		Tcl_AppendResult(interp,buf,NULL);
	}
	sigDeleteSignal(sig);
	return TCL_OK;
}

static int getSegmentBounds (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *h;
	sigSignal *sig;
	int i,level,segnr;
	double xmin,xmax,ymin,ymax;

	check_unipen("getSegmentBounds");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	h = &hierarchies[level].segments[segnr];
	sig = sigDelineation2Signal(pUnipen,h->del);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,h->entry->Entry,")!",NULL);
		return TCL_ERROR;
	}
	/* determine min,max */
	xmin = xmax = sig->x[0];
	ymin = ymax = sig->y[0];
	for (i=1;i<sig->nsamples;i++) {
		if (xmin>sig->x[i]) xmin = sig->x[i];
		if (xmax<sig->x[i]) xmax = sig->x[i];
		if (ymin>sig->y[i]) ymin = sig->y[i];
		if (ymax<sig->y[i]) ymax = sig->y[i];
	}
	sigDeleteSignal(sig);
	Tcl_AppendResult(interp,d2s(xmax-xmin),d2s(ymax-ymin),NULL);
	return TCL_OK;
}

/* a nice utility to be called from a .tcl script
 * get_scaled_segment level segnr xrange yrange width height xmid ymid margin
 */

#define XSCALE(x) (xoffset+((x-xmin)/xrange-.5)*xfactor*w)
#define YSCALE(y) (yoffset-((y-ymin)/yrange-.5)*yfactor*h)

static void scale_segment_points (sigSignal *sig, double *x, double *y
	, double xrange, double yrange
	, double w, double h, double xoffset, double yoffset, double margin)
{
	double xfactor;
	double yfactor;
	double xmin,xmax,ymin,ymax;
	int i;

	w -= 2. * margin;
	h -= 2. * margin;

	/* determine min,max */
	xmin = xmax = sig->x[0];
	ymin = ymax = sig->y[0];
	for (i=1;i<sig->nsamples;i++) {
		if (xmin>sig->x[i]) xmin = sig->x[i];
		if (xmax<sig->x[i]) xmax = sig->x[i];
		if (ymin>sig->y[i]) ymin = sig->y[i];
		if (ymax<sig->y[i]) ymax = sig->y[i];
	}
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

	for (i=0;i<sig->nsamples;i++) {
		x[i] = XSCALE(sig->x[i]);
		y[i] = YSCALE(sig->y[i]);
	}
}

static int getScaledSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *hier;
	sigSignal *sig;
	char buf[128];
	int i,level,segnr;
	double xrange, yrange;
	double w,h;
	double xoffset,yoffset,margin;
	static int nallocated = 0;
	static double *x, *y;

	check_unipen("getScaledSegment");
	check_args(10,"level segnr w h xrange yrange xoff yoff margin");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;
	sscanf(argv[3],"%lf",&w);
	sscanf(argv[4],"%lf",&h);
	sscanf(argv[5],"%lf",&xrange);
	sscanf(argv[6],"%lf",&yrange);
	sscanf(argv[7],"%lf",&xoffset);
	sscanf(argv[8],"%lf",&yoffset);
	sscanf(argv[9],"%lf",&margin);

	hier = &hierarchies[level].segments[segnr];
	sig = sigDelineation2Signal(pUnipen,hier->del);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,hier->entry->Entry,")!",NULL);
		return TCL_ERROR;
	}
	if (nallocated<sig->nsamples) {
		if (nallocated==0) {
			x = (double *) malloc (sig->nsamples*sizeof(double));
			y = (double *) malloc (sig->nsamples*sizeof(double));
		} else {
			x = (double *) realloc (x,sig->nsamples*sizeof(double));
			y = (double *) realloc (y,sig->nsamples*sizeof(double));
		}
		nallocated = sig->nsamples;
	}

	scale_segment_points (sig,x,y,xrange,yrange,w,h,xoffset,yoffset,margin);

	for (i=0;i<sig->nsamples;i++) {
/*
		sprintf (buf," %f %f %d",x[i],y[i],sig->z[i]);
*/
		sprintf (buf," %f %f",x[i],y[i]);
		Tcl_AppendResult(interp,buf,NULL);
	}
	sigDeleteSignal(sig);
	return TCL_OK;
}

static int getChildrenEntries (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h;
	tUPEntry *entry;
	int n,level,segnr;
	int i;
	char *delbuf;
	sigDelineation *del;

	check_unipen("getChildren");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	h     = &hierarchies[level].segments[segnr];
	entry = h->entry;
	may_de_administrate(h);
	if ((children=hierFindChildren(pUnipen,hierarchies,level,segnr))==NULL) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_OK;
	}
	n = h->nchildren;
	Tcl_AppendResult(interp,i2s(n),NULL);
	if (n==0)
		return TCL_OK;
	delbuf = (char *) ckalloc (strlen(entry->Entry)+128);
	Tcl_AppendResult(interp,pUnipen->Levels[level+1]," {",NULL);
	for (i=0;i<n;i++) {
		del = children[i]->del;
		sprintf (delbuf,"{%d: %s [%d:%d-%d:%d]} "
			,children[i]->level_idx
			,children[i]->name
			,del->delineations[0].first_streamnr
			,del->delineations[0].first_samplenr
			,del->delineations[del->ndels-1].last_streamnr
			,del->delineations[del->ndels-1].last_samplenr);
		Tcl_AppendResult(interp,delbuf,NULL);
	}
	Tcl_AppendResult(interp," }",NULL);
	ckfree(delbuf);

	return TCL_OK;
}

static int getChildren (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h;
	tUPEntry *entry;
	int n,level,segnr;

	int i;

	check_unipen("getChildren");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	h     = &hierarchies[level].segments[segnr];
	entry = h->entry;
	may_de_administrate(h);
	if ((children=hierFindChildren(pUnipen,hierarchies,level,segnr))==NULL) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_OK;
	}
	n = h->nchildren;
	Tcl_AppendResult(interp,i2s(n),NULL);
	if (n==0)
		return TCL_OK;

	for (i=0;i<n;i++) {
		Tcl_AppendResult(interp," {",i2s(i),children[i]->name,"}",NULL);
	}

	return TCL_OK;
}

static void print_entry_with_proper_delineation (FILE *fp, tUPHierarchy *h)
{
	char *delbuf,*entrybuf;

	delbuf = sigDelineation2String (h->del);
	if (delbuf==NULL) {
		fprintf (stderr,"UNABLE TO DETECT DELINEATION, taking Entry!\n");
		fprintf (fp,h->entry->Entry);
		return;
	}
	entrybuf = (char *) ckalloc (strlen(h->entry->Entry)+strlen(delbuf)+1);
	sprintf (entrybuf,".SEGMENT %s %s OK \"%s\""
		,pUnipen->Levels[h->level]
		,delbuf
		,h->name);
	fprintf (fp,"%s\n",entrybuf);
	free(delbuf);
	free(entrybuf);
}

static void print_hierarchy (FILE *fp, int level, int segnr)
{
	tUPHierarchy **children,*h;
	tUPEntry *entry;
	int i,n;

	h     = &hierarchies[level].segments[segnr];
	entry = h->entry;
	print_entry_with_proper_delineation (fp,h);
	may_de_administrate(h);
	if ((children=hierFindChildren(pUnipen,hierarchies,level,segnr))==NULL) {
		return;
	}
	n = h->nchildren;

	for (i=0;i<n;i++) {
		print_hierarchy(fp,children[i]->level,children[i]->level_idx);
	}
}

static int print_charstreams(FILE *fp, tUPEntry *entry, int last_stream, sigDelineation *del)
{
	tUPStream *stream;
	int i,first,last;
	int *valid = pUnipen->valid_streams;
	
	first = del->delineations[0].first_streamnr;
	last = del->delineations[del->ndels-1].last_streamnr;
	for (i=last_stream+1;i<first;i++) {
		stream = &pUnipen->stream_sequence[valid[i]];
		fprintf (fp,"%s\n",stream->entry->Entry);
	}
	for (i=first;i<=last;i++) {
		stream = &pUnipen->stream_sequence[valid[i]];
		fprintf (fp,"%s\n",stream->entry->Entry);
	}
	return last;
}

static int saveUnipen (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPEntry *entry;
	char keyword[256];
	int keyid,i;
	FILE *fp;
	int last_stream;
	int *valid = pUnipen->valid_streams;

	check_unipen("saveUnipen");
	check_args(2,"unipen-filename");

	if ((fp=fopen(argv[1],"w"))==NULL) {
		Tcl_AppendResult(interp,"unable to open ",argv[1]," for writing!!",NULL);
		return TCL_ERROR;
	}

	/* print all non-delineation and non-stream related stuff */
	for (i=0;i<pUnipen->TotalNrOfEntries;i++) {
      entry = pUnipen->entry_sequence[i];
		if (sscanf(entry->Entry,"%s",keyword)!=1) {
			fprintf (stderr,"invalid keyword entry in entry %d:\n   %s\n",i,entry->Entry);
			continue;
		}
		keyid = upGetKeywordId(pUnipen,keyword);
		if (keyid!=pUnipen->PenLapseId    &&
			keyid!=pUnipen->PenDownId &&
			keyid!=pUnipen->PenUpId   &&
			keyid!=pUnipen->IncludeId   &&
			keyid!=pUnipen->SegmentId) {
			fprintf (fp,"%s\n",entry->Entry);
		}
	}

	if (hierarchies==NULL) {
		hierarchies = hierCreateLevels (pUnipen);
	}
	last_stream = 0;
	for (i=0;i<hierarchies[0].nsegments;i++) {
		entry = hierarchies[0].segments[i].entry;
		print_hierarchy(fp,0,i);
		last_stream = print_charstreams(fp,entry,last_stream,hierarchies[0].segments[i].del);
	}
	for (i=last_stream+1;i<pUnipen->nvalid_streams;i++) {
		fprintf (fp,"%s\n",pUnipen->stream_sequence[valid[i]].entry->Entry);
	}
	fclose(fp);
	return TCL_OK;
}

static int saveSegments (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPEntry *entry;
	FILE *fp;
	int i;

	check_unipen("saveSegments");
	check_args(2,"segment-filename");

	if ((fp=fopen(argv[1],"w"))==NULL) {
		Tcl_AppendResult(interp,"unable to open ",argv[1]," for writing!!",NULL);
		return TCL_ERROR;
	}

	if (hierarchies==NULL) {
		hierarchies = hierCreateLevels (pUnipen);
	}
	for (i=0;i<hierarchies[0].nsegments;i++) {
		entry = hierarchies[0].segments[i].entry;
		print_hierarchy(fp,0,i);
	}
	fclose(fp);
	return TCL_OK;
}

static int saveWithOtherDelineations (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPEntry *entry;
	FILE *fp;
	int i,keyid;
	char keyword[256];
	int last_stream;
	int *valid = pUnipen->valid_streams;

	check_unipen("saveStreams");
	check_args(3,"streams-filename segment_filename");

	if ((fp=fopen(argv[2],"r"))==NULL) {
		Tcl_AppendResult(interp,"unable to open delineation-file",argv[2]," for reading!!",NULL);
		return TCL_ERROR;
	}
	fclose(fp);

	if ((fp=fopen(argv[1],"w"))==NULL) {
		Tcl_AppendResult(interp,"unable to open ",argv[1]," for writing!!",NULL);
		return TCL_ERROR;
	}

	/* print all non-delineation and non-stream related stuff */
	for (i=0;i<pUnipen->TotalNrOfEntries;i++) {
      entry = pUnipen->entry_sequence[i];
		if (sscanf(entry->Entry,"%s",keyword)!=1) {
			fprintf (stderr,"invalid keyword entry in entry %d:\n   %s\n",i,entry->Entry);
			continue;
		}
		keyid = upGetKeywordId(pUnipen,keyword);
		if (keyid!=pUnipen->PenLapseId    &&
			keyid!=pUnipen->PenDownId &&
			keyid!=pUnipen->PenUpId   &&
			keyid!=pUnipen->IncludeId   &&
			keyid!=pUnipen->SegmentId) {
			fprintf (fp,"%s\n",entry->Entry);
		}
	}

	fprintf (fp,".INCLUDE %s\n",argv[2]);

	if (hierarchies==NULL) {
		hierarchies = hierCreateLevels (pUnipen);
	}
	last_stream = 0;
	for (i=0;i<hierarchies[0].nsegments;i++) {
		entry = hierarchies[0].segments[i].entry;
		last_stream = print_charstreams(fp,entry,last_stream,hierarchies[0].segments[i].del);
	}
	for (i=last_stream+1;i<pUnipen->nvalid_streams;i++) {
		fprintf (fp,"%s\n",pUnipen->stream_sequence[valid[i]].entry->Entry);
	}
	fclose(fp);
	return TCL_OK;
}

static int getNChildren (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i,level,segnr;
	tUPHierarchy *h;

	check_unipen("getNChildren");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (!check_segment(argv[2],interp,level,&segnr)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	h = &hierarchies[level].segments[segnr];
	may_de_administrate(h);
	(void) hierFindChildren(pUnipen,hierarchies,level,segnr);
	Tcl_AppendResult(interp,i2s(h->nchildren),NULL);
	return TCL_OK;
}

static int saveSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i,nbytes,level,segnr;
	sigDelineation *del;
	char *entrybuf;
	tUPHierarchy *h;

	check_unipen("saveSegment");
	check_args(5,"level segnr delineation label");
	if (!check_level(argv[1],interp,&level)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (!check_segment(argv[2],interp,level,&segnr)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	h = &hierarchies[level].segments[segnr];
	hierDeleteHierarchyEntry(h);

	/* create the new delineation and corresponding entry */

	nbytes = 17+strlen(pUnipen->Levels[level])+strlen(argv[3])+strlen(argv[4]);
	entrybuf = (char *) ckalloc (nbytes);
	if (entrybuf==NULL) {
		Tcl_AppendResult (interp,"unable to allocate memory to copy ",h->entry->Entry,NULL);
		return TCL_ERROR;
	}
	del = sigParseDelineation(argv[3]);
	sprintf (entrybuf,".SEGMENT %s %d:%d-%d:%d OK \"%s\"",pUnipen->Levels[level]
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[0].last_streamnr
		,del->delineations[0].last_samplenr,argv[4]);

	hierFillHierarchyEntry (h,upNewEntry(entrybuf,0),strdup(argv[4]),level,segnr,del);
	return TCL_OK;
}


static int getParentDelineation (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int level,segnr;
	int i,s_start,s_end;
	char delbuf[128];

	check_unipen("getParentDelineation");
	check_args(5,"level segnr s_start s_end");
	if (!check_level(argv[1],interp,&level)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (!check_segment(argv[2],interp,level,&segnr)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	s_start = atoi(argv[3]);
	s_end   = atoi(argv[4]);
	do_get_segment_delineation (pUnipen,s_start,s_end,delbuf);
	Tcl_AppendResult(interp,delbuf,NULL);
	return TCL_OK;
}

static int getChildDelineation (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h;
	tUPEntry *entry,dummy_entry;
	int level,segnr,idx;
	int i,n,s_start,s_end;
	sigSignal *sig;
	sigDelineation *del;
	char dummy_buf[512],entrybuf[256];

	check_unipen("getChildDelineation");
	check_args(7,"level segnr idx s_start s_end delineation");
	if (!check_level(argv[1],interp,&level)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (!check_segment(argv[2],interp,level,&segnr)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	idx     = atoi(argv[3]);
	s_start = atoi(argv[4]);
	s_end   = atoi(argv[5]);
	set_hierarchy_info(1);

	del = sigParseDelineation(argv[6]);
	sig = sigDelineation2Signal(pUnipen,del);
	if (sig==NULL) {
		fprintf (stderr,"parent delineation for signal should be %d:%d-%d:%d\n"
			,del->delineations[0].first_streamnr
			,del->delineations[0].first_samplenr
			,del->delineations[0].last_streamnr
			,del->delineations[0].last_samplenr);
		Tcl_AppendResult(interp,"unable to get signal from ("
			,i2s(del->delineations[0].first_streamnr)
			,i2s(del->delineations[0].first_samplenr)
			,i2s(del->delineations[0].last_streamnr)
			,i2s(del->delineations[0].last_samplenr)
				,")!",NULL);
		return TCL_ERROR;
	}
	sigFreeDelineation (del);

	/* create a dummy delineation */
	sprintf (dummy_buf,".SEGMENT %s %s OK \"something\""
		,pUnipen->Levels[level],argv[6]);
	dummy_entry.Entry = &dummy_buf[0];


	if ((del=sigBounds2Delineation(pUnipen,&dummy_entry,sig,s_start,s_end))==NULL) {
		Tcl_AppendResult(interp,"unable to get delineation for segment ",i2s(s_start),i2s(s_end)
		,"to entry: ",dummy_entry.Entry," having ",i2s(sig->nsamples)," samples!",NULL);
		sigFreeDelineation(del);
		sigDeleteSignal(sig);
		return TCL_ERROR;
	}
	sigDeleteSignal(sig);

	n = del->ndels - 1;
	sprintf (entrybuf,"%d:%d-%d:%d"
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[n].last_streamnr
		,del->delineations[n].last_samplenr);
	Tcl_AppendResult(interp,entrybuf,NULL);
	sigFreeDelineation(del);
	return TCL_OK;
}

static int checkChildDelineation (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h;
	tUPEntry *entry;
	int n,level,segnr,idx;
	int i;
	sigDelineation *del;

	check_unipen("checkChildDelineation");
	check_args(5,"level segnr idx delineation");
	if (!check_level(argv[1],interp,&level)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (!check_segment(argv[2],interp,level,&segnr)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}

	idx      = atoi(argv[3]);
	set_hierarchy_info(0);

	if ((del=sigParseDelineation(argv[4]))==NULL) {
		Tcl_AppendResult (interp,"Invalid delineation '",argv[4],"'!!!",NULL);
		return TCL_ERROR;
	}
	if (!sigSegmentInSegment(h->del,del)) {
			Tcl_AppendResult(interp,"0",NULL);
			sigFreeDelineation(del);
			return TCL_OK;
	} else {
			Tcl_AppendResult(interp,"1",NULL);
			sigFreeDelineation(del);
			return TCL_OK;
	}
}

static int saveChild (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h;
	tUPEntry *entry;
	int n,nbytes,level,segnr,idx;
	int i,overrule;
	sigDelineation *del;
	char *entrybuf;

	tUPEntry *newentry;
	char *newname;

	check_unipen("saveChild");
	check_args(7,"level segnr idx delineation label overrule");
	if (!check_level(argv[1],interp,&level)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (!check_segment(argv[2],interp,level,&segnr)) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}

	idx      = atoi(argv[3]);
	overrule = atoi(argv[6]);

	set_hierarchy_info(1);

	if ((del=sigParseDelineation(argv[4]))==NULL) {
		Tcl_AppendResult (interp,"Invalid delineation '",argv[4],"'!!!",NULL);
		return TCL_ERROR;
		if (!overrule) {
			Tcl_AppendResult(interp,"0",NULL);
			return TCL_OK;
		}
	}
	if (!sigSegmentInSegment(h->del,del)) {
		fprintf (stderr,"invalid '%s'\n",argv[4]);
		if (!overrule) {
			sigFreeDelineation(del);
			Tcl_AppendResult(interp,"0",NULL);
			return TCL_OK;
		}
	}

	/* nbytes = space for new entry 
		= .SEGMENT level delineation OK "name" */
	nbytes = 17+strlen(pUnipen->Levels[level+1])+strlen(argv[4])+strlen(argv[5]);
	entrybuf = (char *) ckalloc (nbytes);
	if (entrybuf==NULL) {
		Tcl_AppendResult (interp,"unable to allocate memory to copy ",h->entry->Entry,NULL);
		return TCL_ERROR;
	}
	memset(entrybuf,0,nbytes);

	/* ok now we know we may overwrite 'idx', delete it */
	idx = children[idx]->level_idx;
	may_de_administrate(h);

	h = &hierarchies[level+1].segments[idx];
	hierDeleteHierarchyEntry(h);

	/* create the new delineation and corresponding entry */

	sprintf (entrybuf,".SEGMENT %s %s OK \"%s\"",pUnipen->Levels[level+1],argv[4],argv[5]);
	newentry = upNewEntry(entrybuf,0);
	newname  = strdup(argv[5]);
	hierFillHierarchyEntry (h,newentry,newname,level+1,idx,del);

	Tcl_AppendResult(interp,"1",NULL);
	return TCL_OK;
}

static int delSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *segments;
	int i,level,segnr,n;

	check_unipen("delSegment");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	/* I know we have a memory leak here .... */
	segments = hierarchies[level].segments;
	n = hierarchies[level].nsegments;
	for (i=segnr;i<n-1;i++) {
		memcpy((char *)&segments[i],(char *)&segments[i+1],sizeof(tUPHierarchy));
		segments[i].level_idx = i;
	}
	hierarchies[level].nsegments -= 1;
	return TCL_OK;
}

static int addSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *segments,*h,*c_src,*c_dst;
	int i,level,segnr,insert,n,nbytes;
	tUPEntry *newentry;
	char *newname,*entrybuf,*delbuf;
	sigDelineation *newdel;

	check_unipen("addSegment");
	check_args(4,"level segnr insert");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;
	insert = atoi(argv[3]);

	/* allocate place for new segment in current level */
	n = hierarchies[level].nsegments;
	if (n==0) {
		hierarchies[level].segments = (tUPHierarchy *) ckalloc (sizeof(tUPHierarchy));
	}
	else {
		hierarchies[level].segments = (tUPHierarchy *) realloc
			(hierarchies[level].segments,(n+1)*sizeof(tUPHierarchy));
		if (hierarchies[level].segments==NULL) {
			Tcl_AppendResult(interp,"unable to allocate memory for another segment",NULL);
			return TCL_ERROR;
		}
	}
	hierarchies[level].nsegments++;
	segments = hierarchies[level].segments;
	h = &segments[segnr];

	n = hierarchies[level].nsegments-1;

	if (insert) { /* move all segments including segnr one down */
		for (i=n;i>segnr;i--) {
			memcpy((char *)&segments[i],(char *)&segments[i-1],sizeof(tUPHierarchy));
			segments[i].level_idx = i;
		}
		c_dst = &segments[segnr];
		c_src = &segments[segnr+1];
	}

	else { /* move all segments excluding segnr one down */
		for (i=n;i>segnr+1;i--) {
			memcpy((char *)&segments[i],(char *)&segments[i-1],sizeof(tUPHierarchy));
			segments[i].level_idx = i;
		}
		c_dst = &segments[segnr+1];
		c_src = &segments[segnr];
		segnr++;
	}

	nbytes = strlen(c_src->entry->Entry)+10;
	entrybuf = (char *) ckalloc(nbytes);
	delbuf = (char *) ckalloc(nbytes);
	if (delbuf==NULL||entrybuf==NULL) {
		Tcl_AppendResult (interp,"unable to allocate memory to copy ",h->entry->Entry,NULL);
		return TCL_ERROR;
	}
	memset(delbuf,0,nbytes);
	memset(entrybuf,0,nbytes);
	sscanf(c_src->entry->Entry,"%*s%*s%s",delbuf);
	sprintf (entrybuf,".SEGMENT %s %s OK \"????\""
		,pUnipen->Levels[level],delbuf);

	newentry = upNewEntry(entrybuf,0);
	newname  = strdup("????");
	newdel   = sigCopyDelineation(c_src->del);
	hierFillHierarchyEntry (c_dst,newentry,newname,level,segnr,newdel);
	ckfree(delbuf);

	return TCL_OK;
}

static int delChild (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h,*segments;
	tUPEntry *entry;
	int n,level,segnr,idx;
	int i;

	check_unipen("delSegment");
	check_args(4,"level segnr idx");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	idx   = atoi(argv[3]);
	set_hierarchy_info(1);

	n        = hierarchies[level+1].nsegments;
	segments = hierarchies[level+1].segments;
	idx      = h->children[idx]->level_idx;
	may_de_administrate(h);

	/* move all segments except the last one and excluding idx one up */
	for (i=idx;i<n-2;i++) {
			memcpy((char *)&segments[i],(char *)&segments[i+1],sizeof(tUPHierarchy));
			segments[i].level_idx = i;
	}
	/* make sure to copy the last ones */
	if (n>1&&idx!=n-1) {
		hierCopyHierarchyEntries (&segments[n-2],&segments[n-1]);
		segments[n-2].level_idx = n-2;
	}
	hierDeleteHierarchyEntry(&segments[n-1]);
	hierarchies[level+1].nsegments--;

	return TCL_OK;
}

static int expand_delineation (char *buf, sigDelineation *hdel, sigDelineation *cdel, int insert)
{
	int nh,nc;
	int str_start,sam_start;
	int *valid = pUnipen->valid_streams;

	nh = hdel->ndels-1;
	nc = cdel->ndels-1;
	if (insert) {
		if ( (cdel->delineations[0].first_streamnr==hdel->delineations[0].first_streamnr)&&
			  (cdel->delineations[0].first_samplenr==hdel->delineations[0].first_samplenr) )
				return 0;
		str_start = cdel->delineations[0].first_streamnr;
		if (cdel->delineations[0].first_samplenr>0) {
			sam_start = cdel->delineations[0].first_samplenr-1;
		}
		else if (cdel->delineations[0].first_samplenr==0) {
			str_start--;
			sam_start = pUnipen->stream_sequence[valid[str_start]].nsamples - 1;
		} else {
			str_start = hdel->delineations[nh].last_streamnr;
			sam_start = hdel->delineations[nh].last_samplenr;
		}
		sprintf (buf,"%d:%d-%d:%d"
			,hdel->delineations[0].first_streamnr,hdel->delineations[0].first_samplenr
			,str_start,sam_start);
	}
	else {
		if ( (cdel->delineations[nc].last_streamnr==hdel->delineations[nh].last_streamnr)&&
			  (cdel->delineations[nc].last_samplenr==hdel->delineations[nh].last_samplenr) )
				return 0;
		str_start = cdel->delineations[nc].last_streamnr;
		if (cdel->delineations[nc].last_samplenr<pUnipen->stream_sequence[valid[str_start]].nsamples-1) {
			sam_start = cdel->delineations[nc].last_samplenr+1;
		}
		else if (cdel->delineations[nc].last_samplenr==pUnipen->stream_sequence[valid[str_start]].nsamples-1) {
			str_start++;
			sam_start = 0;
		} else {
			str_start = hdel->delineations[nh].first_streamnr;
			sam_start = hdel->delineations[nh].first_samplenr;
		}
		sprintf (buf,"%d:%d-%d:%d",str_start,sam_start
			,hdel->delineations[nh].last_streamnr,hdel->delineations[nh].last_samplenr);
	}
	return 1;
}

static int addChild (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *c_src,*h,**children,*c_dst,*segments;
	tUPEntry *entry;
	int nbytes,n,i,level,segnr,idx,insert;
	char *delbuf,*entrybuf;

	tUPEntry *newentry;
	sigDelineation *newdel;
	char *newname;

	check_unipen("addSegment");
	check_args(5,"level segnr idx insert");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	idx = atoi(argv[3]);
	set_hierarchy_info(1);

	insert = atoi(argv[4]);

	c_src = &(hierarchies[level+1].segments[h->children[idx]->level_idx]);
	nbytes = strlen(c_src->entry->Entry)+10;
	delbuf = (char *) ckalloc(nbytes);
	if (delbuf==NULL) {
		Tcl_AppendResult (interp,"unable to allocate memory to copy ",h->entry->Entry,NULL);
		return TCL_ERROR;
	}
	memset(delbuf,0,nbytes);
	if ( (idx==h->nchildren-1&&!insert) || (idx==0&&insert) ) {
		if (!expand_delineation(delbuf,h->del,c_src->del,insert)) {
			Tcl_AppendResult(interp,"0",NULL);
			ckfree(delbuf);
			return TCL_OK;
		}
		newdel = sigParseDelineation(delbuf);
	}
	else {
		sscanf(c_src->entry->Entry,"%*s%*s%s",delbuf);
		newdel   = sigCopyDelineation(c_src->del);
	}

	/* allocate place for new child in next level */
	n = hierarchies[level+1].nsegments;
	if (n==0) {
		hierarchies[level+1].segments = (tUPHierarchy *) ckalloc (sizeof(tUPHierarchy));
	}
	else {
		hierarchies[level+1].segments = (tUPHierarchy *) realloc
			(hierarchies[level+1].segments,(n+1)*sizeof(tUPHierarchy));
		if (hierarchies[level+1].segments==NULL) {
			Tcl_AppendResult(interp,"unable to allocate memory for another child",NULL);
			return TCL_ERROR;
		}
	}
	hierarchies[level+1].nsegments++;
	segments = hierarchies[level+1].segments;
	entry = h->entry;
	idx = h->children[idx]->level_idx;
	may_de_administrate(h);

	n = hierarchies[level+1].nsegments-1;

	if (insert) { /* move all segments including idx one down */
		for (i=n;i>idx;i--) {
			memcpy((char *)&segments[i],(char *)&segments[i-1],sizeof(tUPHierarchy));
			segments[i].level_idx = i;
		}
		c_dst = &segments[idx];
		c_src = &segments[idx+1];
	}

	else { /* move all segments excluding idx one down */
		for (i=n;i>idx+1;i--) {
			memcpy((char *)&segments[i],(char *)&segments[i-1],sizeof(tUPHierarchy));
			segments[i].level_idx = i;
		}
		c_dst = &segments[idx+1];
		c_src = &segments[idx];
		idx++;
	}

	entrybuf = (char *) ckalloc(nbytes);
	if (entrybuf==NULL) {
		Tcl_AppendResult (interp,"unable to allocate memory to copy ",h->entry->Entry,NULL);
		return TCL_ERROR;
	}
	memset(entrybuf,0,nbytes);
	sprintf (entrybuf,".SEGMENT %s %s OK \"????\""
		,pUnipen->Levels[level+1],delbuf);
	newentry = upNewEntry(entrybuf,0);
	newname  = strdup("????");
	hierFillHierarchyEntry (c_dst,newentry,newname,level+1,idx,newdel);
	ckfree(delbuf);

	Tcl_AppendResult(interp,"1",NULL);
	return TCL_OK;
}

static int addFirstInHierarchy (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	char *entrybuf,*newname;
	tUPEntry *newentry;
	sigDelineation *newdel;
	int i,level;

	check_unipen("addFirstInHierarchy");
	check_args(3,"level delineation");
	if (!check_level(argv[1],interp,&level))
      return 1;
	newdel   = sigParseDelineation(argv[2]);
	if (newdel==NULL) {
		Tcl_AppendResult(interp,"invalid delineation ",argv[2],NULL);
		return TCL_ERROR;
	}
	if (hierarchies==NULL) {
		hierarchies = hierCreateLevels (pUnipen);
	}
	hierarchies[level].segments = (tUPHierarchy *) ckalloc (sizeof(tUPHierarchy));
	hierarchies[level].nsegments = 1;

	entrybuf = (char *) ckalloc (strlen(argv[2])+strlen(pUnipen->Levels[level])+20);
	sprintf (entrybuf,".SEGMENT %s %s OK \"????\""
		,pUnipen->Levels[level],argv[2]);
	newentry = upNewEntry(entrybuf,0);
	newname  = strdup("????");

	hierFillHierarchyEntry (&hierarchies[level].segments[0],newentry,newname,level,0,newdel);
	return TCL_OK;
}

static int addFirstSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *c_src,*h,*c_dst,*segments;
	tUPEntry *entry;
	int nbytes,n,i,pidx,idx,level,segnr;
	char *entrybuf, *delbuf;

	tUPEntry *newentry;
	sigDelineation *newdel;
	char *newname;

	check_unipen("addFirstSegment");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	n     = hierarchies[level+1].nsegments;
	h     = &hierarchies[level].segments[segnr];
	entry = h->entry;
	idx = n;

	/* allocate place for new child in next level */
	if (n==0) {
		hierarchies[level+1].segments = (tUPHierarchy *) ckalloc (sizeof(tUPHierarchy));
	}
	else {
		hierarchies[level+1].segments = (tUPHierarchy *) realloc
			(hierarchies[level+1].segments,(n+1)*sizeof(tUPHierarchy));
		if (hierarchies[level+1].segments==NULL) {
			Tcl_AppendResult(interp,"unable to allocate memory for another child",NULL);
			return TCL_ERROR;
		}
		/* now find the place where to add the child */
		segments = hierarchies[level+1].segments;
		for (i=0;i<n;i++) {
			pidx = hierSearchParentIndex(hierarchies,&segments[i]);
			if (pidx>segnr) { /* insert it just before this one */
				idx = i;
				break;
			}
		}
		for (i=n;i>idx;i--) {
			memcpy((char *)&segments[i],(char *)&segments[i-1],sizeof(tUPHierarchy));
			segments[i].level_idx = i;
		}
	}
	hierarchies[level+1].nsegments = n+1;
	segments = hierarchies[level+1].segments;
	c_src = h;
	c_dst = &segments[idx];

	nbytes = strlen(c_src->entry->Entry)+10;
	entrybuf = (char *) ckalloc(nbytes);
	delbuf = (char *) ckalloc(nbytes);
	if (delbuf==NULL||entrybuf==NULL) {
		Tcl_AppendResult (interp,"unable to allocate memory to copy ",h->entry->Entry,NULL);
		return TCL_ERROR;
	}
	memset(delbuf,0,nbytes);
	memset(entrybuf,0,nbytes);
	sscanf(c_src->entry->Entry,"%*s%*s%s",delbuf);
	sprintf (entrybuf,".SEGMENT %s %s OK \"????\""
		,pUnipen->Levels[level+1],delbuf);

	newentry = upNewEntry(entrybuf,0);
	newname  = strdup("????");
	newdel   = sigCopyDelineation(c_src->del);

	hierFillHierarchyEntry (c_dst,newentry,newname,level+1,idx,newdel);
	ckfree(delbuf);

	return TCL_OK;
}

static int getSegmentEntry (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i,level,segnr;
	char qualbuf[128],delbuf[256];
	sigDelineation *del;
	char *name,*delibuf;
	tUPEntry *entry;

	check_unipen("getSegmentEntry");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return TCL_ERROR;
	if (!check_segment(argv[2],interp,level,&segnr))
		return TCL_ERROR;

	del = hierarchies[level].segments[segnr].del;
	sprintf (delbuf," %d:%d-%d:%d"
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[del->ndels-1].last_streamnr
		,del->delineations[del->ndels-1].last_samplenr);
	entry = hierarchies[level].segments[segnr].entry;
	name = upEntryName(entry);
	delibuf = (char *) ckalloc (strlen(entry->Entry));
	sscanf(entry->Entry,".SEGMENT %*s %s %s",delibuf,qualbuf);
	Tcl_AppendResult(interp
		,"{.SEGMENT "
		,pUnipen->Levels[level]," "
		,delibuf," "
		,qualbuf," "
		,"{",name,"}","} "
		,delbuf,NULL);
	ckfree(delibuf);
	free(name);
	return TCL_OK;
}

static int getEntry (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h,*e;
	tUPEntry *entry;
	int i,n,level,segnr,idx;
	char delbuf[256];
	sigDelineation *del;

	check_unipen("getEntry");
	check_args(4,"level segnr idx");
	if (!check_level(argv[1],interp,&level))
		return TCL_ERROR;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return TCL_ERROR;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return TCL_ERROR;

	idx = atoi(argv[3]);
	set_hierarchy_info(1);
	e = &hierarchies[level+1].segments[children[idx]->level_idx];
	del = hierarchies[level+1].segments[children[idx]->level_idx].del;
	sprintf (delbuf," %d:%d-%d:%d"
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[del->ndels-1].last_streamnr
		,del->delineations[del->ndels-1].last_samplenr);
	Tcl_AppendResult(interp,e->entry->Entry,delbuf,NULL);

	may_de_administrate(h);

	return TCL_OK;
}

static int getChildSignal (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h,*e;
	tUPEntry *entry;
	int n,level,segnr,idx;
	int i;
	sigSignal *sig;
	char buf[128];
	sigDelineation *del;

	check_unipen("getChildSignal");
	check_args(4,"level segnr idx");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	idx   = atoi(argv[3]);
	set_hierarchy_info(1);
	e = children[idx];

	del = sigCreateBoundingDelineation(children[idx]->del);
	sig = sigDelineation2Signal (pUnipen,del);
	sigFreeDelineation(del);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,children[idx]->entry->Entry,")!",NULL);
		return TCL_ERROR;
	}
	for (i=0;i<sig->nsamples;i++) {
		sprintf (buf," %d %d %d",sig->x[i],sig->y[i],sig->z[i]);
		Tcl_AppendResult(interp,buf,NULL);
	}
	sigDeleteSignal(sig);
	may_de_administrate(h);

	return TCL_OK;
}

static int initiateSegmentWalk (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int i,level,segnr;

	check_unipen("initiateSegmentWalk");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return TCL_ERROR;
	if (!check_segment(argv[2],interp,level,&segnr))
		return TCL_ERROR;
	initiate_segment_walk(pUnipen,hierarchies,level,segnr);
	return TCL_OK;
}

static char *basic_jumpsize[NBASIC_JUMPSIZES] = {
   "1 sample",
	"10 samples",
	"100 samples",
	"1000 samples",
	"Y-extrema",
	"penstreams"
};

static int set_jumpsize (char *arg, int *childlevel)
{
	int i;
	char **levels = pUnipen->Levels;
	int nlevels = pUnipen->NrOfLevels;

	*childlevel = -1;
	for (i=0;i<NBASIC_JUMPSIZES;i++) {
		if (strcmp(basic_jumpsize[i],arg)==0) {
			return i;
		}
	}
	for (i=0;i<nlevels;i++) {
		if (strcmp(levels[i],arg)==0) {
			*childlevel = i;
			return NBASIC_JUMPSIZES+i;
		}
	}
	fprintf (stderr,"error: arg (%s)\n",arg);
	return -1;
}

static int getSegmMarkers (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *h;
	int i,level,childlevel,segnr,mark,jumpsize,m_start,m_end;

	check_unipen("getSegmMarkers");
	check_args(7,"level segnr mark jumpsize m_start m_end");
	if (!check_level(argv[1],interp,&level))
		return TCL_ERROR;
	if (!check_segment(argv[2],interp,level,&segnr))
		return TCL_ERROR;

	mark     = atoi(argv[3]);
	jumpsize = set_jumpsize(argv[4],&childlevel);
	m_start  = atoi(argv[5]);
	m_end    = atoi(argv[6]);

	if (mark<0 || mark>7) {
		Tcl_AppendResult(interp,"error mark ",i2s(mark),"must be in [0...7]!",NULL);
		return TCL_ERROR;
	}

	h = &hierarchies[level].segments[segnr];
	if (!do_set_segm_markers(mark,jumpsize,level,segnr,childlevel,hierarchies,pUnipen))
		Tcl_AppendResult(interp,"0",NULL);
	else
		Tcl_AppendResult(interp,"1",NULL);
	return TCL_OK;
}

static int getEditedSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	sigSignal *sig;
	char delbuf[128],buf[128];
	int i;

	check_unipen("getEditedSegment");
	check_args(1,"");

	sig = do_get_edited_segment(pUnipen,delbuf);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from edited segment!",NULL);
		return TCL_ERROR;
	}
	Tcl_AppendResult(interp,"{",delbuf,"} {",NULL);
	for (i=0;i<sig->nsamples;i++) {
		sprintf (buf," %d %d %d",sig->x[i],sig->y[i],sig->z[i]);
		Tcl_AppendResult(interp,buf,NULL);
	}
	Tcl_AppendResult(interp,"}",NULL);
	sigDeleteSignal(sig);
	return TCL_OK;
}

static int getChildMarkers (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *h;
	int i,level,childlevel,segnr,mark,jumpsize,m_start,m_end;
	sigSignal *sig;
	char delbuf[256];
	sigDelineation *del,*child_del;

	check_unipen("getChildMarkers");
	check_args(6,"level segnr mark jumpsize cur_del");
	if (!check_level(argv[1],interp,&level))
		return TCL_ERROR;
	if (!check_segment(argv[2],interp,level,&segnr))
		return TCL_ERROR;

	mark     = atoi(argv[3]);
	if (mark<0 || mark>7) {
		Tcl_AppendResult(interp,"error mark ",i2s(mark),"must be in [0...7]!",NULL);
		return TCL_ERROR;
	}
	jumpsize = set_jumpsize (argv[4],&childlevel);

	h = &hierarchies[level].segments[segnr];
	del = sigCreateBoundingDelineation(h->del);

	child_del = sigParseDelineation (argv[5]);
	if (!sigSegmentInSegment(del,child_del)) {
		Tcl_AppendResult(interp," -1 -1 ",argv[5],NULL);
		return TCL_OK;
	}
	sig = sigDelineation2Signal (pUnipen,del);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,h->entry->Entry,")!",NULL); 
		return TCL_ERROR;
	}  
	sigSegmentBoundsInDelineation(pUnipen,del,child_del,&m_start,&m_end);
	m_end += -1 + m_start;
	sigFreeDelineation(del);
	sigFreeDelineation(child_del);
	do_get_child_markers (mark,jumpsize,level,segnr,childlevel
		,&m_start,&m_end
		,hierarchies,sig,pUnipen,delbuf);

	sigDeleteSignal(sig);

	Tcl_AppendResult(interp,i2s(m_start),i2s(m_end),delbuf,NULL);
	return TCL_OK;
}

static int getChildData (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *h;
	sigDelineation *del;
	int i,level,segnr,s_start,s_end;
	sigSignal *sig;
	char buf[128];

	check_unipen("getChildData");
	check_args(5,"level segnr start end");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	s_start = atoi(argv[3]);
	s_end = atoi(argv[4]);
	h = &hierarchies[level].segments[segnr];
	del = sigCreateBoundingDelineation(h->del);
	sig = sigDelineation2Signal(pUnipen,del);
	sigFreeDelineation(del);

	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,h->entry->Entry,")!",NULL);
		return TCL_ERROR;
	}
	if (s_start<0||s_start>=sig->nsamples) {
		Tcl_AppendResult(interp,"error signal start",i2s(s_start),"must be in [0,",i2s(sig->nsamples),"]",NULL);
		return TCL_ERROR;
	}
	if (s_end<0||s_end>=sig->nsamples) {
		Tcl_AppendResult(interp,"error signal end",i2s(s_end),"must be in [0,",i2s(sig->nsamples),"]",NULL);
		return TCL_ERROR;
	}
	for (i=s_start;i<=s_end;i++) {
		sprintf (buf," %d %d %d",sig->x[i],sig->y[i],sig->z[i]);
		Tcl_AppendResult(interp,buf,NULL);
	}
	sigDeleteSignal(sig);

	return TCL_OK;
}

static int getChildBounds (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy **children,*h;
	tUPEntry *entry;
	int n,level,segnr,idx;
	int i,offset,nsamples;

	check_unipen("getChildBounds");
	check_args(4,"level segnr idx");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;

	idx = atoi(argv[3]);
	set_hierarchy_info(1);

	sigSegmentBoundsInDelineation(pUnipen,h->del,children[idx]->del,&offset,&nsamples);
	Tcl_AppendResult(interp,i2s(offset),i2s(offset-1+nsamples)," {",children[idx]->name,"}",NULL);
	may_de_administrate(h);
	return TCL_OK;
}

static int getChildrenBounds (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPEntry *entry;
	tUPHierarchy **children,*c,*h;
	int n,level,segnr,idx;
	int i,offset,nsamples;
	sigSignal *sig;
	int *o,*s,*a;

	check_unipen("getChildrenBounds");
	check_args(3,"level segnr");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (level==pUnipen->NrOfLevels) {
		Tcl_AppendResult(interp,"0",NULL);
		return 0;
	}
	if (!check_segment(argv[2],interp,level,&segnr))
		return 1;
	
	idx = 0;
	set_hierarchy_info(0);
	Tcl_AppendResult(interp,i2s(n),NULL);
	if (n==0)
		return TCL_OK;

	o = (int *) ckalloc (n*sizeof(int));
	s = (int *) ckalloc (n*sizeof(int));
	a = (int *) ckalloc (n*sizeof(int));

	sig = sigDelineation2Signal(pUnipen,h->del);
	if (sig==NULL) {
		Tcl_AppendResult(interp,"unable to get signal from ("
			,h->entry->Entry,")!",NULL);
		return TCL_ERROR;
	}
	sigDeleteSignal(sig);

	for (i=0;i<n;i++) {
		c = children[i];
		sigSegmentBoundsInDelineation(pUnipen,h->del,c->del,&offset,&nsamples);
		a[i] = 1;
		o[i] = offset;
		s[i] = offset-1+nsamples;
	}

	for (i=0;i<n;i++) {
		if (a[i]) {
			Tcl_AppendResult(interp," {",change_braces(children[i]->name),"} ",i2s(o[i]),i2s(s[i]),NULL);
		}
	}

	may_de_administrate(h);
	ckfree(o);
	ckfree(a);
	ckfree(s);
	return TCL_OK;
}

static int getAllBounds (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	tUPHierarchy *h;
	int i,j,level;
	sigSignal *sig;
	double xmin,xmax,ymin,ymax;
	double xrange,yrange;

	check_unipen("getAllBounds");
	check_args(2,"level");
	if (!check_level(argv[1],interp,&level))
		return 1;

	xrange = yrange = 0;
	for (i=0;i<hierarchies[level].nsegments;i++) {
		h = &hierarchies[level].segments[i];
		xmin = ymin = 9999999.0;
		xmax = ymax = -9999999.0;
		sig = sigDelineation2Signal(pUnipen,h->del);
		if (sig==NULL) {
			Tcl_AppendResult(interp,"unable to get signal from ("
				,h->entry->Entry,")!",NULL);
			return TCL_ERROR;
		}
		for (j=0;j<sig->nsamples;j++) {
			if (sig->x[j]<xmin) xmin = sig->x[j];
			if (sig->y[j]<ymin) ymin = sig->y[j];
			if (sig->x[j]>xmax) xmax = sig->x[j];
			if (sig->y[j]>ymax) ymax = sig->y[j];
		}
		if (xrange<xmax-xmin) xrange = xmax-xmin;
		if (yrange<ymax-ymin) yrange = ymax-ymin;
		sigDeleteSignal(sig);
	}

	Tcl_AppendResult(interp,d2s(xrange),d2s(yrange),NULL);

	return TCL_OK;
}

static int getUnipenTabletInfo (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
#define MM_PER_INCH   25.4
	double fsampl_of_data,resol_mm,resol_g,ppmm,ppg;

	if (!upGetDoubleFromEntries(pUnipen,".POINTS_PER_SECOND",&fsampl_of_data,0)) {
		fprintf (stderr,"taking default samplerate of 100HZ\n");
		fsampl_of_data = 100.0;
	}
	if (!upGetDoubleFromEntries(pUnipen,".X_POINTS_PER_MM",&ppmm,0)) {
		if (!upGetDoubleFromEntries(pUnipen,".X_POINTS_PER_INCH",&ppmm,0)) {
			fprintf (stderr,"'%s' contains no X_POINTS_PER_MM nor X_POINTS_PER_INCH!\n",pUnipen->cur_file_open);
			fprintf (stderr,"(taking default resolution of 200 points/inch)\n");
			ppmm = 200.0;
		}
		ppmm /= MM_PER_INCH;
	}
	resol_mm = 1./ppmm;
	if (!upGetDoubleFromEntries(pUnipen,".POINTS_PER_GRAM",&ppg,0)) {
		resol_g = 100.;
		if (pUnipen->has_p!=-1)
			fprintf (stderr,"taking default pressure resolution of %.3f gram/unit\n", resol_g);
	} else {
		resol_g = 1./ppg;
	}
	Tcl_AppendResult(interp,d2s(fsampl_of_data),d2s(resol_mm),d2s(resol_g),NULL);
	return TCL_OK;
}

/* something for searching in sorted segments */
static char **my_sorted_names;
static int *my_sorted_indices;

static int my_sort_names (const void *p1, const void *p2)
{
	int idx1 = *((int *) p1);
	int idx2 = *((int *) p2);

	return strcmp(my_sorted_names[idx1],my_sorted_names[idx2]);
}

static int searchSegment (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int level,i,n;

	check_unipen("searchSegment");
	check_args(4,"level string sorted");
	if (!check_level(argv[1],interp,&level))
		return 1;
	n = hierarchies[level].nsegments;
	if (atoi(argv[3])==1) {
		my_sorted_names = (char **) malloc (n*sizeof(char *));
		my_sorted_indices = (int *) malloc (n*sizeof(int));
		for (i=0;i<n;i++) {
			my_sorted_names[i] = hierarchies[level].segments[i].name;
			my_sorted_indices[i] = i;
		}
		qsort((void *)my_sorted_indices,n,sizeof(int),my_sort_names);
		for (i=0;i<n;i++) {
			if (strncmp(my_sorted_names[i],argv[2],strlen(argv[2]))==0) {
				Tcl_AppendResult(interp,i2s(i),NULL);
				free(my_sorted_names);
				free(my_sorted_indices);
				return TCL_OK;
			}
		}
		free(my_sorted_names);
		free(my_sorted_indices);
	} else {
		for (i=0;i<n;i++) {
			if (strncmp(hierarchies[level].segments[i].name,argv[2],strlen(argv[2]))==0) {
				Tcl_AppendResult(interp,i2s(i),NULL);
				return TCL_OK;
			}
		}
	}
	Tcl_AppendResult(interp,"-1",NULL);
	return TCL_OK;
}

static int upGetSegnr (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	int level,i,idx;
	char c;

	check_unipen("upGetSegnr");
	check_args(4,"level string index");
	if (!check_level(argv[1],interp,&level))
		return 1;
	if (strcmp(argv[2],"*")==0) {
		Tcl_AppendResult(interp,argv[3],NULL);
		return TCL_OK;
	}
	idx = atoi(argv[3]);
	for (i=0;i<hierarchies[level].nsegments;i++) {
		if (strcmp(hierarchies[level].segments[i].name,argv[2])==0) {
			if (idx==0) {
				Tcl_AppendResult(interp,i2s(i),NULL);
				return TCL_OK;
			}
			idx--;
		}
	}
	/* not found, if it is a CHARACTER, try lowercase */
	if (strlen(argv[2])==1) {
		c = (char) tolower((int)argv[2][0]);
		idx = atoi(argv[3]);
		for (i=0;i<hierarchies[level].nsegments;i++) {
			if (hierarchies[level].segments[i].name[0]==c &&hierarchies[level].segments[i].name[1]=='\0') {
				if (idx==0) {
					Tcl_AppendResult(interp,i2s(i),NULL);
					return TCL_OK;
				}
				idx--;
			}
		}
	}
	Tcl_AppendResult(interp,"error: your request for level='"
		,argv[1],"' , string='"
		,argv[2],"', index='"
		,argv[3],"' not OK!",NULL);
	return TCL_ERROR;
}


static int getMaxDel (ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
	char result[128];
	int n = pUnipen->nvalid_streams;
	int *valid = pUnipen->valid_streams;

	sprintf (result,"0:0-%d-%d\n",n-1,pUnipen->stream_sequence[valid[n-1]].nsamples-1);
	Tcl_AppendResult(interp,result,NULL);
	return TCL_OK;
}

static char initScript[] =
"proc upworksInit {} {\n\
	global unipen_library unipen_version env\n\
	rename upworksInit {}\n\
	set dirs {}\n\
	if [info exists env(UNIPEN_TCL_LIBRARY)] {\n\
	lappend dirs $env(UNIPEN_TCL_LIBRARY)\n\
	}\n\
	if [info exists env(EXT_FOLDER)] {\n\
	lappend dirs [file join $env(EXT_FOLDER) \"Tool Command Language\" lib unipen$unipen_version]\n\
	}\n\
	lappend dirs $unipen_library\n\
	set unipen_library {}\n\
	if ![catch {uplevel #0 source -rsrc unipen}] {\n\
	return\n\
	}\n\
	lappend dirs [file join [file dirname [info library]] unipen$unipen_version]\n\
	set parentDir [file dirname [file dirname [info nameofexecutable]]]\n\
	lappend dirs [file join $parentDir lib unipen$unipen_version]\n\
	lappend dirs [file join [file dirname $parentDir] unipen$unipen_version library]\n\
	lappend dirs [file join $parentDir library]\n\
	foreach i $dirs {\n\
	set unipen_library $i\n\
	if ![catch {uplevel #0 source [file join $i upworks]}] {\n\
		return\n\
	}\n\
	}\n\
	set unipen_library {}\n\
	set msg \"Can't find a usable upworks in the following directories: \n\"\n\
	foreach d $dirs\n\
		append msg \"  $d\n\"\n\
	append msg \"This probably means that Unipen wasn't installed properly.\n\"\n\
	error $msg\n\
}\n\
upworksInit";

int Upworks_Init(interp)
	Tcl_Interp *interp;
{
	char *libDir;

	if (Tcl_PkgRequire(interp, "Tcl", EXPECTED_TCL_VERSION, 0) == NULL) {
	return TCL_ERROR;
	}
	if (Tcl_PkgProvide(interp, "Upworks", UNIPEN_VERSION) != TCL_OK) {
	return TCL_ERROR;
	}

	libDir = Tcl_GetVar(interp, "unipen_library", TCL_GLOBAL_ONLY);
	if (libDir == NULL) {
	Tcl_SetVar(interp, "unipen_library", defaultLibraryDir, TCL_GLOBAL_ONLY);
	}
	Tcl_SetVar(interp, "unipen_version", UNIPEN_VERSION , TCL_GLOBAL_ONLY);
	
	Tcl_CreateCommand (interp,"open_unipen",openUnipen,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"close_unipen",closeUnipen,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_scaled_segment",getScaledSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_segment",getSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_segment_bounds",getSegmentBounds,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_full_segment",getFullSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"reset_segments",resetSegments,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_segment_names",getSegmentNames,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand (interp,"get_edited_segment",getEditedSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"initiate_segment_walk",initiateSegmentWalk,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_segm_markers",getSegmMarkers,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_child_markers",getChildMarkers,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_child_bounds",getChildBounds,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_child_data",getChildData,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_child_signal",getChildSignal,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_children_entries",getChildrenEntries,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_children",getChildren,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_children_bounds",getChildrenBounds,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_all_bounds",getAllBounds,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand (interp,"search_segment",searchSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand (interp,"get_segment_entry",getSegmentEntry,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_entry",getEntry,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand (interp,"save_segment",saveSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_nchildren",getNChildren,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_parent_delineation",getParentDelineation,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_child_delineation",getChildDelineation,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"check_child_delineation",checkChildDelineation,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"save_child",saveChild,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"add_first_segment",addFirstSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"add_child",addChild,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"add_segment",addSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"del_child",delChild,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"del_segment",delSegment,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand (interp,"get_unipen_tablet_info",getUnipenTabletInfo,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"get_max_del",getMaxDel,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"add_first_in_hierarchy",addFirstInHierarchy,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand (interp,"upgetsegnr",upGetSegnr,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);

	Tcl_CreateCommand (interp,"save_segments",saveSegments,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"save_with_other_dels",saveWithOtherDelineations,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand (interp,"save_unipen",saveUnipen,(ClientData)NULL,
		(Tcl_CmdDeleteProc *) NULL);
	return Tcl_Eval(interp, initScript);
}
