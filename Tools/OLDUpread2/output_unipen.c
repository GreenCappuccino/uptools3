/* get_all_segments.c
 * gets all UNIPEN .SEGMENT entries fulfilling a syntax
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <uplib.h>
#include <upsiglib.h>
#include <up_segment_io.h>

#include "upread2.h"

static void upread_print_header (FILE *fp_out, upsegQueryInfo *info
	, OFunction_Info *oinfo)
{
	int i;

	fprintf (fp_out,".VERSION 1.0\n");
	fprintf (fp_out,"\n");
	fprintf (fp_out,".COMMENT\n");
	fprintf (fp_out,"UNIPEN file automatically generated by upread\n");
	fprintf (fp_out,"upread: %s\n",UR_VERSION);
	fprintf (fp_out,"\n");
	fprintf (fp_out,"The files from which .%s segments were extracted were:\n",info->level);
	for (i=0;i<info->nfiles;i++)
  		fprintf (fp_out,"   %s\n",info->files[i]);
	fprintf (fp_out,"\n");
	fprintf (fp_out,"The query used for selection was:\n");
	fprintf (fp_out,"   -l %s\n",info->level);
	for (i=0;i<info->nstrings;i++)
		fprintf (fp_out,"   -s %s\n",info->strings[i]);
	fprintf (fp_out,"\n");
	fprintf (fp_out,".SETUP\n");
	fprintf (fp_out,".HIERARCHY");
	if (info->level[0] == '*') {
		fprintf (fp_out," %s\n",ANY_HIERARCHY);
	} else {
		fprintf (fp_out," %s\n",info->level);
	}

	/* make sure that .COORD is X Y Z or .COORD X Y */
	if (oinfo->use_Z)
		fprintf (fp_out,".COORD X Y Z\n");
	else
		fprintf (fp_out,".COORD X Y\n");

	fprintf (fp_out,"\n.START_SET\n");
	fprintf (fp_out,"\n");
}

static void transform_stream (FILE *fp_out, char *str, int use_Z, tUPUnipen *pUnipen)
{
	char *newptr,*ptr,stream_type[128];
	int x,y,z,pen_up,ncoords,zcoord;

	sscanf(str,"%s",stream_type);
	fprintf (fp_out,"%s\n",stream_type);
	pen_up = (strcmp(stream_type,".PEN_UP")==0);
	ncoords = pUnipen->NrOfCoords;
	if (pUnipen->has_p!=-1)
		zcoord = pUnipen->has_p;
	else
		zcoord = pUnipen->has_z;
	ptr = upNthString (str,1);
	if (!use_Z) {

		while (ptr!=NULL) {
			x = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr)
				break;
			ptr = newptr;
			y = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr)
				break;
			fprintf (fp_out,"\t%d %d\n",x,y);
			if (newptr!=NULL)
				ptr = upNthString(newptr,ncoords-2);
			else
				ptr = NULL;
		}
	}

	else if (zcoord==-1) { /* generate 0,100 .PEN_UP,.PEN_DOWN */

		while (ptr!=NULL) {
			x = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr)
				break;
			ptr = newptr;
			y = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr)
				break;
			if (pen_up)
				fprintf (fp_out,"\t%d %d 0\n",x,y);
			else
				fprintf (fp_out,"\t%d %d 100\n",x,y);
			ptr = upNthString(newptr,ncoords-2);
		}

	} else {

		while (ptr!=NULL) {
			x = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr)
				break;
			ptr = newptr;
			y = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr)
				break;
			ptr = upNthString(newptr,zcoord-2);
			z = (int) strtol(ptr,&newptr,10);
			if (ptr==newptr)
				break;
			fprintf (fp_out,"\t%d %d %d\n",x,y,z);
			ptr = upNthString(newptr,ncoords-zcoord-1);
		}

	}

}

static int upread_print_segment (FILE *fp_out, int streamnr, char *level
	, char *name, sigCharStream *streams
	, int transform_coord, int use_Z, tUPUnipen *pUnipen)
{
	int i,next_streamnr,last_samplenr;

	next_streamnr = streamnr+streams->nstreams-1;
	last_samplenr = streams->nsamples[streams->nstreams-1]-1;
	fprintf (fp_out,".SEGMENT %s %d:0-%d:%d OK \"%s\"\n"
		,level,streamnr,next_streamnr,last_samplenr,name);
	if (transform_coord) {
		for (i=0;i<streams->nstreams;i++) {
			transform_stream(fp_out,streams->streams[i],use_Z,pUnipen);
		}
	} else {
		for (i=0;i<streams->nstreams;i++) {
			fprintf (fp_out,"%s\n",streams->streams[i]);
		}
	}
	return next_streamnr+1;
}

void init_output_unipen (tUPUnipen *pUnipen, FILE *fp_out
	, upsegQueryInfo *info, OFunction_Info *oinfo)
{
	static int first = 1;
	char *key_entry;

	if (first)
		upread_print_header (fp_out,info,oinfo);

	/* try to get X-resolution */
	if ((key_entry=upGetArgument(pUnipen,".X_POINTS_PER_MM",0))==NULL) {
		if ((key_entry=upGetArgument(pUnipen,".X_POINTS_PER_INCH",0))==NULL) {
			fprintf (stderr,"no .X_POINTS_PER_MM nor .X_POINTS_PER_INCH given\n");
		} else
			fprintf (fp_out,"%s\n",key_entry);
	} else
		fprintf (fp_out,"%s\n",key_entry);

	/* try to get Y-resolution */
	if ((key_entry=upGetArgument(pUnipen,".Y_POINTS_PER_MM",0))==NULL) {
		if ((key_entry=upGetArgument(pUnipen,".Y_POINTS_PER_INCH",0))==NULL) {
			fprintf (stderr,"no .Y_POINTS_PER_MM nor .Y_POINTS_PER_INCH given\n");
		} else
			fprintf (fp_out,"%s\n",key_entry);
	} else
		fprintf (fp_out,"%s\n",key_entry);

	/* try to get Z-resolution */
	if ((key_entry=upGetArgument(pUnipen,".POINTS_PER_GRAM",0))==NULL) {
		if ((key_entry=upGetArgument(pUnipen,".POINTS_PER_GRAM",0))==NULL) {
			fprintf (stderr,"no .POINTS_PER_GRAM given\n");
		} else
			fprintf (fp_out,"%s\n",key_entry);
	} else
		fprintf (fp_out,"%s\n",key_entry);

	/* try to get sampling rate */
	if ((key_entry=upGetArgument(pUnipen,".POINTS_PER_SECOND",0))==NULL) {
			fprintf (stderr,"no .POINTS_PER_SECOND given\n");
	} else
		fprintf (fp_out,"%s\n",key_entry);

	first = 0;
}

int coords_are_not_as_requested (tUPUnipen *pUnipen, int use_Z)
{
	if (use_Z) {
		if (pUnipen->has_z==-1&&pUnipen->has_p==-1)
			return 1;
		if (pUnipen->NrOfCoords!=3)
			return 1;
		return 0;
	} else if (pUnipen->NrOfCoords!=2) {
		return 1;
	} else {
		return 0;
	}
}

int output_unipen (upsegQueryInfo *info, OFunction_Info *oinfo
	, FILE *fp_out
	, tUPUnipen *pUnipen
	, tUPEntry **entries
	, int nentries
	, char **level_names
	, char **names
	, sigCharStream **streams)
{
	int i,transform_coord;
	static int streamnr = 0;

	transform_coord = coords_are_not_as_requested(pUnipen,oinfo->use_Z);
	for (i=0;i<nentries;i++) {
		fprintf (fp_out,".COMMENT segment originating from '%s' (%d streams)\n"
			, info->files[0],streams[i]->nstreams);
		streamnr = upread_print_segment (fp_out
			,streamnr
			,level_names[i]
			,names[i]
			,streams[i]
			,transform_coord
			,oinfo->use_Z
			,pUnipen);
	}
	return 1;
}
