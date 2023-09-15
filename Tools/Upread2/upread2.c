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
#include "output_image.h"
#include "unipen_identifications.h"

static int nmatching_entries_found = 0;
static char *output_function_names[NOUTPUT_FUNCTIONS] = {
	"output_unipen",
	"output_featchar",
	"output_image",
	"output_ppm",
};

void print_options (void)
{
	int i;

	fprintf (stderr,"options are:\n");
	fprintf (stderr,"     -o  outfile (used as base for 'output_image')\n");
	fprintf (stderr,"     -p  patfile\n");
	fprintf (stderr,"     -s  string\n");
	fprintf (stderr,"     -wc writer_code\n");
	fprintf (stderr,"     -ac (to also include .COMMENT statements)\n");
	fprintf (stderr,"     -co (if only char instead of full name)\n");
	fprintf (stderr,"     -i  index\n");
	fprintf (stderr,"     -I  index-file containing one or more indices\n");
	fprintf (stderr,"     -l  level\n");
	fprintf (stderr,"     -F  first\n");
	fprintf (stderr,"     -L  last\n");
	fprintf (stderr,"     -Z  (for output in .COORD X Y Z format, default=X Y)\n");
	fprintf (stderr,"     -O  output-function\n");
	fprintf (stderr,"where output-function can be any of the following strings:\n");
	for (i=0;i<NOUTPUT_FUNCTIONS;i++)
		fprintf (stderr,"         %s\n",output_function_names[i]);
	/* options for output_image and output_featchar */
	fprintf (stderr,"     -n  #resample points (for 'output_image' and 'output_featchar')\n");
	fprintf (stderr,"     -d  factor for adjusting resampling (between [0,1])\n");
	fprintf (stderr,"     -m  margin      (for 'output_image')\n");
	fprintf (stderr,"     -b  brush       (for 'output_image')\n");
	fprintf (stderr,"     -nc ncols       (for 'output_image')\n");
	fprintf (stderr,"     -w  width       (for 'output_image')\n");
	fprintf (stderr,"     -h  height      (for 'output_image')\n");
	fprintf (stderr,"     -frmt [xbm|ppm] (for 'output_image')\n");
	fprintf (stderr,"     -pd <red> <green> <blue>\n");
	fprintf (stderr,"     -pu <red> <green> <blue>\n");
	fprintf (stderr,"     -bg <red> <green> <blue>\n");
	fprintf (stderr,"     -S for 'use_same_scale'\n");
}

int parse_output_function (char *function_name)
{
	int i;

	for (i=0;i<NOUTPUT_FUNCTIONS;i++)
		if (strcmp(output_function_names[i],function_name)==0) {
			return i;
		}
	fprintf (stderr,"unknown output_function '%s'!!\n",function_name);
	fprintf (stderr,"use one of: %d = %s\n",0,output_function_names[0]);
	for (i=1;i<NOUTPUT_FUNCTIONS;i++)
		fprintf (stderr,"            %d = %s\n",i,output_function_names[i]);
	exit(1);
}

void initialize_output_function (OFunction_Info *oinfo)
{
	oinfo->index_file[0]   = '\0';
	oinfo->output_function = _OUTPUT_UNIPEN;
	oinfo->width           = oinfo->height = 250;
	oinfo->brush           = 5;
	oinfo->margin          = 6;
	oinfo->m               = 30;
	oinfo->dfact           = 1.0;
	oinfo->do_add_sincos   = 0;
	oinfo->do_add_phi      = 0;
	oinfo->use_Z           = 0;
	oinfo->determine_wc    = 1;
	oinfo->add_comments    = 0;
	oinfo->char_only       = 0;
	oinfo->width           = 500;
	oinfo->height          = 500;
	oinfo->brush           = 1;
	oinfo->margin          = 3;
	oinfo->use_same_scale  = 0;
	oinfo->ncols           = -1;
	pd_red          = 0;
	pd_green        = 0;
	pd_blue         = 0;
	pu_red          = 127;
	pu_green        = 127;
	pu_blue         = 127;
	bg_red          = 255;
	bg_green        = 255;
	bg_blue         = 255;
}

upsegQueryInfo *parse_args (int argc, char *argv[], char outfile[], OFunction_Info *oinfo)
{
	int i;
	static upsegQueryInfo info;

	if (argc<2) {
		fprintf (stderr,"use: %s unipen-file [unipen-files] [options]!\n",argv[0]);
		print_options();
		exit(1);
	}

	/* set default values */

	initialize_output_function(oinfo);
	upsegInitializeQueryInfo(&info);

	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-s")==0) {
			i++;
			upsegAddQuery(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-p")==0) {
			i++;
			upsegAddQueriesFromPatfile(&info,argv[i]);
		}
		else if (strcmp(argv[i],"-S")==0) {
			i++;
			info.writing_style = argv[i];
		}
		else if (strcmp(argv[i],"-O")==0) {
			i++;
			oinfo->output_function = parse_output_function(argv[i]);
		}
		else if (strcmp(argv[i],"-o")==0) {
			i++;
			strcpy(outfile,argv[i]);
		}
		else if (strcmp(argv[i],"-i")==0) {
			i++;
			info.index = atoi(argv[i]);
			fprintf (stderr,"index set to %d\n",info.index);
		}
		else if (strcmp(argv[i],"-I")==0) {
			i++;
			strcpy(oinfo->index_file,argv[i]);
			fprintf (stderr,"using Index-file '%s'\n",oinfo->index_file);
		}
		else if (strcmp(argv[i],"-l")==0) {
			i++;
			strcpy(info.level,argv[i]);
			fprintf (stderr,"level set to %s\n",info.level);
		}
		else if (strcmp(argv[i],"-F")==0) {
			i++;
			info.first = atoi(argv[i]);
			fprintf (stderr,"first set to %d\n",info.first);
		}
		else if (strcmp(argv[i],"-L")==0) {
			i++;
			info.last = atoi(argv[i]);
			fprintf (stderr,"last set to %d\n",info.last);
		}
		else if (strcmp(argv[i],"-m")==0) {
			i++;
			oinfo->margin = atoi(argv[i]);
			fprintf (stderr,"using margin = %d\n",oinfo->margin);
		}
		else if (strcmp(argv[i],"-n")==0) {
			i++;
			oinfo->m = atoi(argv[i]);
			fprintf (stderr,"using %d resample points\n",oinfo->m);
		}
		else if (strcmp(argv[i],"-d")==0) {
			i++;
			sscanf(argv[i],"%lf",&oinfo->dfact);
			fprintf (stderr,"using dfact=%f\n",oinfo->dfact);
		}
		else if (strcmp(argv[i],"-nc")==0) {
			i++;
			oinfo->ncols = atoi(argv[i]);
			fprintf (stderr,"using ncols = %d\n",oinfo->ncols);
		}
		else if (strcmp(argv[i],"-w")==0) {
			i++;
			oinfo->width = atoi(argv[i]);
			fprintf (stderr,"using width = %d\n",oinfo->width);
		}
		else if (strcmp(argv[i],"-wc")==0) {
			i++;
			oinfo->writer_code = atoi(argv[i]);
			oinfo->determine_wc = 0;
			fprintf (stderr,"using writer_code = %d\n",oinfo->writer_code);
		}
		else if (strcmp(argv[i],"-ac")==0) {
			oinfo->add_comments = 1;
			fprintf (stderr,"also adding comments in case of UNIPEN\n");
		}
		else if (strcmp(argv[i],"-co")==0) {
			oinfo->char_only = 1;
			fprintf (stderr,"using char labels only\n");
		}
		else if (strcmp(argv[i],"-frmt")==0) {
			i++;
			if (strcmp(argv[i],"xbm")==0) {
				oinfo->im_frmt = O_XBM;
				fprintf (stderr,"using image output in XBM format\n");
			} else if (strcmp(argv[i],"ppm")==0) {
				oinfo->im_frmt = O_PPM;
				fprintf (stderr,"using image output in PPM format\n");
			} else {
				fprintf (stderr,"unknown image output-format '%s'!\n",argv[i]);
				fprintf (stderr,"use one of 'xbm' or 'ppm'\n");
				print_options();
				exit(1);
			}
		}
		else if (strcmp(argv[i],"-S")==0) {
			oinfo->use_same_scale = 1;
			fprintf (stderr,"using same scale per segment...\n");
		}
		else if (strcmp(argv[i],"-h")==0) {
			i++;
			oinfo->height = atoi(argv[i]);
			fprintf (stderr,"using height = %d\n",oinfo->height);
		}
		else if (strcmp(argv[i],"-bg")==0) {
			i++;
			bg_red = (unsigned char) atoi(argv[i]);
			i++;
			bg_green = (unsigned char) atoi(argv[i]);
			i++;
			bg_blue = (unsigned char) atoi(argv[i]);
			fprintf (stderr,"using bg (r,g,b) = (%d,%d,%d)\n"
				, (int) bg_red, (int) bg_green, (int) bg_blue);
		}
		else if (strcmp(argv[i],"-pd")==0) {
			i++;
			pd_red = (unsigned char) atoi(argv[i]);
			i++;
			pd_green = (unsigned char) atoi(argv[i]);
			i++;
			pd_blue = (unsigned char) atoi(argv[i]);
			fprintf (stderr,"using pd (r,g,b) = (%d,%d,%d)\n"
				, (int) pd_red, (int) pd_green, (int) pd_blue);
		}
		else if (strcmp(argv[i],"-pu")==0) {
			i++;
			pu_red = (unsigned char) atoi(argv[i]);
			i++;
			pu_green = (unsigned char) atoi(argv[i]);
			i++;
			pu_blue = (unsigned char) atoi(argv[i]);
			fprintf (stderr,"using pu (r,g,b) = (%d,%d,%d)\n"
				, (int) pu_red, (int) pu_green, (int) pu_blue);
		}
		else if (strcmp(argv[i],"-b")==0) {
			i++;
			oinfo->brush = atoi(argv[i]);
			fprintf (stderr,"using brush = %d\n",oinfo->brush);
		}
		else if (strcmp(argv[i],"-Z")==0) {
			fprintf (stderr,"using .COORD X Y Z\n");
			oinfo->use_Z = 1;
		}
		else {
			if (argv[i][0]=='-') {
				fprintf (stderr,"wrong option '%s'!\n",argv[i]);
				fprintf (stderr,"use: %s unipen-file [options]!\n",argv[0]);
				print_options();
				exit(1);
			}
			else
				upsegAddFile(&info,argv[i]);
		}
	}
	if (oinfo->margin<oinfo->brush) {
		fprintf (stderr,"WARNING: margin %d must be >= brush %d (resetting)!!\n"
			,oinfo->margin,oinfo->brush);
		oinfo->margin = oinfo->brush;
	}
	return &info;
}

void init_output_function (tUPUnipen *pUnipen
	, FILE *fp_out, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
	switch (oinfo->output_function) {
		case _OUTPUT_UNIPEN:
			init_output_unipen(pUnipen,fp_out,query_info,oinfo);
			break;
		case _OUTPUT_FEATCHAR:
			init_output_featchar(fp_out,query_info,oinfo);
			break;
		case _OUTPUT_IMAGE:
			break;
		case _OUTPUT_PPM:
			break;
		default:
			fprintf (stderr,"requested output function not implemented (yet)!\n");
			exit(1);
	}
}


void handle_potential_unipen_file (char *fname, FILE *fp_out
	, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
	int i;
	tUPEntry **entries;
	sigCharStream **streams;
	int nentries;
	char **names;
	char **level_names;
	char *hierarchy;

	query_info->nfiles = 1;
	strcpy(query_info->files[0],fname);
	if (oinfo->determine_wc) {
#ifndef USE_WID
		oinfo->writer_code = 666;
#else
		oinfo->writer_code = updirFilename2UnipenCode(fname);
#endif
	}

	entries = upsegGetEntriesWithStreams (query_info,&nentries
	                                                ,&names
	                                                ,&streams
																	,&level_names
																	,&hierarchy);

	fprintf (stderr,"got %d entries\n",nentries);
	if (nentries<=0) {
		fprintf (stderr,"no items fulfil your query in '%s'!\n",fname);
		upDelUnipen(query_info->pUnipen);
		return;
	}
	nmatching_entries_found += nentries;

	init_output_function(query_info->pUnipen,fp_out,query_info,oinfo);

	switch (oinfo->output_function) {
		case _OUTPUT_UNIPEN:
			output_unipen(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);
			break;
		case _OUTPUT_FEATCHAR:
			output_featchar(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);
			break;
		case _OUTPUT_IMAGE:
			output_image(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);
			break;
		case _OUTPUT_PPM:
			output_ppm(query_info,oinfo,fp_out,query_info->pUnipen,entries,nentries
				,level_names,names,streams);
			break;
		default:
			break;
	}

	for (i=0;i<nentries;i++) {
		free(level_names[i]);
		free(names[i]);
		sigDeleteCharStream(streams[i]);
		free(entries[i]->Entry);
		free(entries[i]);
	}
	free(streams);
	free(level_names);
	free(names);
	free(entries);
	free(hierarchy);
	upDelUnipen(query_info->pUnipen);
}

void walk_through_directories (FILE *fp_out, char *curpath, char *dirname
	, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
	DIR *curdir;
	struct DIR_DEFINE *dptr;
	char newpath[512];

	if ((curdir=opendir(dirname))==NULL) {
		fprintf (stderr,"    [%s]\n",dirname);
		handle_potential_unipen_file (dirname,fp_out,query_info,oinfo);
		return;
	}
	if (strcmp(curpath,"")!=0)
		sprintf (newpath,"%s/%s",curpath,dirname);
	else
		strcpy(newpath,dirname);
	if (chdir(newpath)!=0) {
		perror(dirname);
		closedir(curdir);
		return;
	}
	fprintf (stderr,"> %s\n",dirname);
	for (dptr=readdir(curdir);dptr!=NULL;dptr=readdir(curdir)) {
		if (dptr->d_name[0]!='.') {
			walk_through_directories(fp_out,newpath,dptr->d_name,query_info,oinfo);
		}
	}
	closedir(curdir);
	if (strcmp(curpath,"")!=0)
		chdir(curpath);
}

void handle_directory (FILE *fp_out, char *path, upsegQueryInfo *query_info, OFunction_Info *oinfo)
{
	char cwd[512];
	char here[512];

	getcwd(here,512);
	/* I need absolute pathnames, so go to the required directory (if it is no file) */
	if (chdir(path)!=0) { /* assuming it is a file */
		handle_potential_unipen_file (path,fp_out,query_info,oinfo);
		return;
	} else if (oinfo->output_function==_OUTPUT_PPM) {
		fprintf (stderr,"UPREAD2 FATAL: for output_ppm I need only one UNIPEN-file!\n");
		exit(1);
	}
	getcwd(cwd,512);
	walk_through_directories(fp_out,"",cwd,query_info,oinfo);
	chdir(here);
}

int main (int argc, char *argv[])
{
	FILE *fp_out;
	upsegQueryInfo *query_info;
	OFunction_Info oinfo;
	char **dirnames;
	int i,ndirs;

	fprintf (stderr,"upread2: %s\n",UR_VERSION);
	oinfo.outfile[0] = '\0';
	query_info = parse_args(argc,argv,oinfo.outfile,&oinfo);
	if (oinfo.output_function!=_OUTPUT_IMAGE &&oinfo.output_function!=_OUTPUT_PPM) {
		if (oinfo.outfile[0]=='\0') {
			fprintf (stderr,"upread: output to stdout\n");
			fp_out = stdout;
		} else if ((fp_out=fopen(oinfo.outfile,"r"))!=NULL) {
			fprintf (stderr,"outfile %s already exists!",oinfo.outfile);
			exit(1);
		} else if ((fp_out=fopen(oinfo.outfile,"w"))==NULL) {
			fprintf (stderr,"unable to open outfile %s!",oinfo.outfile);
			exit(1);
		}
	} else
		fp_out = NULL;
#ifdef USE_WID
	read_unipen_identifications(WRITER_ID_FILE);
#endif
	/* now handle all files and dirnames in query_info as search-points */
	ndirs = query_info->nfiles;
	if (ndirs>1&&oinfo.output_function==_OUTPUT_PPM) {
		fprintf (stderr,"UPREAD2 FATAL: for output_ppm I need only one UNIPEN-file!\n");
		exit(1);
	}
	dirnames = (char **) malloc (ndirs*sizeof(char *));
	for (i=0;i<ndirs;i++) {
		dirnames[i] = strdup(query_info->files[i]);
	}

	for (i=0;i<ndirs;i++)
		handle_directory(fp_out,dirnames[i],query_info,&oinfo);
	if (fp_out!=NULL&&fp_out!=stdout)
		fclose(fp_out);
	fprintf (stderr,"found in total %d matching entries\n",nmatching_entries_found);
	return 0;
}
