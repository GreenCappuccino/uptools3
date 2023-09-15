#ifdef USE_WID

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uplib.h>

#include "unipen_identifications.h"

/* unipen_identifications.c
   - Reads a 'labeled writer-code file', containing entries
     'number writer-name dataset path', like, e.g.,
00241                 teun    plucoll teun/set3.dat
00292              mariska    papyrus NIC-papyrus-mariska.dat
21284              wcdr451      nist7 8/bbd/wcdr/wcdr451.dat
*/

#define NDATASETS   6
#define PLUCOLL_SET 0
#define PAPYRUS_SET 1
#define NIST7_SET   2

typedef struct {
	char *name;
	char *path;
} DataSet;

static DataSet datasets[NDATASETS] = {
	{ "plucoll", "/fxt/projects/unipen/plucoll" },
	{ "papyrus", "/ext/projects/nici-unipen/unipen_files" },
	{ "nist7"  , "/fxt/users/vuurpijl/train_r01_v07/data" },
	{ "NIC"    , "/u6/nici/software/pak/unipen/train_nici/NIC" },
	{ "HAND"   , "/u6/nici/software/pak/unipen/train_nici/hand_labeled" },
	{ "HPB0VHS", "/u6/nici/software/pak/unipen/train_nici/hpb0_vhs_labeled" },
};

int dataset (char *dset)
{
	int i;

	for (i=0;i<NDATASETS;i++)
		if (strcmp(datasets[i].name,dset)==0) {
			return i;
		}
	fprintf (stderr,"dataset '%s' unknown!!\n",dset);
	return -1;
}

static UnipenIdentification *unipen_identifications = NULL;
static int nunipen_ids = 0;

char *construct_path (char *fname)
{
	static char path[512];
	static char here[512];

	getcwd(here,512);
	/* if here is part of fname, we know fname is an absolute path */
	if (strstr(fname,here)!=NULL)
		return fname;
	sprintf (path,"%s/%s",here,fname);
	return path;
}

static int unipen_code (char *fname)
{
	int i;
	char *the_path;

	the_path = construct_path(fname);
	
	for (i=0;i<nunipen_ids;i++) {
		if (strstr(the_path,unipen_identifications[i].path)!=NULL)
				return i;
	}
	fprintf (stderr,"file [%s] unknown (path constructed was '%s')!!\n"
		,fname,the_path);
	return -1;
}

void insert_unipen_identification (int idx, char *wname, char *dset, char *path)
{
	int i;

	if (nunipen_ids==idx) {
		if (nunipen_ids==0) {
			unipen_identifications = (UnipenIdentification *) malloc (sizeof(UnipenIdentification));
		}
		else {
			unipen_identifications = (UnipenIdentification *) realloc (unipen_identifications,
				(nunipen_ids+1)*sizeof(UnipenIdentification));
		}
	} else if (nunipen_ids<idx) {
		fprintf (stderr,"strange.... requesting to insert %d in array [0-%d]!\n"
			,idx,nunipen_ids);
		if (nunipen_ids==0) {
			unipen_identifications = (UnipenIdentification *) malloc ((idx+1)*sizeof(UnipenIdentification));
		}
		else {
			unipen_identifications = (UnipenIdentification *) realloc (unipen_identifications,
				(idx+1)*sizeof(UnipenIdentification));
		}
		for (i=nunipen_ids;i<idx;i++)
			unipen_identifications[i].idx = -1;
		nunipen_ids = idx;
	} else {
		if (unipen_identifications[idx].idx != -1) {
			fprintf (stderr,"request to fill %s at %d, but it already contains %s!\n"
				,wname,idx,unipen_identifications[idx].writer_name);
			return;
		}
	}
	unipen_identifications[nunipen_ids].idx         = idx;
	unipen_identifications[nunipen_ids].writer_name = strdup(wname);
	unipen_identifications[nunipen_ids].dataset     = dataset(dset);
	unipen_identifications[nunipen_ids].path        = strdup(path);
	nunipen_ids                                    += 1;
}

int  read_unipen_identifications (char *fname)
{
	FILE *fp;
	int n,idx;
	char wname[64],dset[64],path[256];
	char *env_file;

	if ((env_file=getenv(WID_ENV))!=NULL) {
		if ((fp=fopen(env_file,"r"))==NULL) {
			fprintf (stderr,"unable to open unipen_identifications '%s'!!\n",env_file);
			exit(1);
		}
	} else {
		if ((fp=fopen(fname,"r"))==NULL) {
			fprintf (stderr,"unable to open unipen_identifications '%s'!!\n",fname);
			exit(1);
		}
	}
	n = 0;
	while (fscanf(fp,"%d%s%s%s",&idx,wname,dset,path)==4) {
		insert_unipen_identification(idx,wname,dset,path);
		n++;
	}
	fclose(fp);
	return n;
}

int find_index (tUPEntry *entry, char *name_requested, tUPEntry **entries, int nentries)
{
	int i,idx;
	char *nxt_name,nxt_hierarchy[64],hierarchy[64];

	if (sscanf(entry->Entry,".SEGMENT %s",hierarchy)!=1) {
		fprintf (stderr,"unable to scan hierarchy from entry [%s]\n!!"
			,entry->Entry);
		return -1;
	}

	idx = 0;
	for (i=0;i<nentries;i++) {
		if (sscanf(entries[i]->Entry,".SEGMENT %s",nxt_hierarchy)!=1) {
			fprintf (stderr,"unable to scan hierarchy from entry %d=[%s]\n!!"
				,i,entries[i]->Entry);
			return -1;
		}
		if (strcmp(hierarchy,nxt_hierarchy)==0) {
			if (strcmp(entry->Entry,entries[i]->Entry)==0)
				return idx;
			else {
				nxt_name = upEntryName (entries[i]);
				/*
				if (strcmp(name_requested,nxt_name)==0)
					idx++;
				*/
				if (name_requested[0]==nxt_name[0])
					idx++;
				free(nxt_name);
			}
		}
	}
	return -1;
}

int updirFilename2UnipenCode (char *fname)
{
	return unipen_code(fname);
}

char *updirEntry2UnipenCode (tUPEntry *entry, tUPEntry **entries
	, int nentries, int wcode, int char_only)
{
	static char result[256];
	char hierarchy[64],*name;
	static int unknown = 0;
	int idx;

	if (sscanf(entry->Entry,".SEGMENT %s %s %s \"%s\""
			,hierarchy,result,result,result)!=4) {
		fprintf (stderr,"unable to scan hierarchy from entry [%s]\n!!"
			,entry->Entry);
	}
	name = upEntryName(entry);
	idx   = find_index (entry,name,entries,nentries);
	if (char_only) {
		if (wcode<0) {
			sprintf (result,"%c/?????/%c/%d"
				,name[0]
				,hierarchy[0]
				,idx);
		} else {
			sprintf (result,"%c/%05d/%c/%d"
				,name[0]
				,wcode
				,hierarchy[0]
				,idx);
		}
	} else {
		if (wcode<0) {
			sprintf (result,"%s/????%d/%c/%d"
				,name
				,unknown++
				,hierarchy[0]
				,idx);
		} else {
			sprintf (result,"%s/%05d/%c/%d"
				,name
				,wcode
				,hierarchy[0]
				,idx);
		}
	}
	free(name);
	return result;
}

void parse_writercode (char *wcode, UnipenIdentification *unipen_id)
{
	char *wsrc,*ptr;
	int idx;

	wsrc = wcode;
	ptr = strchr(wcode,'/');
	if (ptr==NULL) {
		if (sscanf(wcode,"%d",&idx)!=1) {
			fprintf (stderr,"error in wcode '%s'\n",wcode);
			exit(1);
		}
		unipen_id->idx         = idx;
		unipen_id->writer_name = unipen_identifications[idx].writer_name;
		unipen_id->dataset     = unipen_identifications[idx].dataset;
		unipen_id->path        = unipen_identifications[idx].path;
		return;
	}
	ptr[0] = '\0';
	strcpy(unipen_id->seg_name,wcode);

	wcode = ptr+1;
	ptr = strchr(wcode,'/');
	ptr[0] = '\0';
	if (sscanf(wcode,"%d",&idx)!=1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wsrc);
		exit(1);
	}
	if (idx==-1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wsrc);
		exit(1);
	}
	unipen_id->idx         = idx;
	unipen_id->writer_name = unipen_identifications[idx].writer_name;
	unipen_id->dataset     = unipen_identifications[idx].dataset;
	unipen_id->path        = unipen_identifications[idx].path;

	wcode = ptr+1;
	switch (wcode[0]) {
		case 'C':
			strcpy(unipen_id->level,"CHARACTER");
			break;
		case 'W':
			strcpy(unipen_id->level,"WORD");
			break;
		default:
			fprintf (stderr,"unipen_code '%s' contains unknown hierarchy '%c'!!\n"
				,wsrc,wcode[0]);
			exit(1);
	}
	idx = -1;
	if (sscanf(wcode+2,"%d",&idx)!=1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wcode);
		exit(1);
	}
	if (idx==-1) {
		fprintf (stderr,"unable to determine index from '%s'!!\n",wcode);
		exit(1);
	}
	unipen_id->seg_idx = idx;
	fprintf (stderr,"%d=%s lev=%s idx=%d label=%s [%s] %s\n"
		,unipen_id->idx
		,unipen_id->writer_name
		,unipen_id->level
		,unipen_id->seg_idx
		,unipen_id->seg_name
		,datasets[unipen_id->dataset].name
		,unipen_id->path);
}

char *writerid_2_upfile (UnipenIdentification *unipen_id)
{
	static char result[512];

	sprintf (result,"%s/%s"
		,datasets[unipen_id->dataset].path
		,unipen_id->path);
	return result;
}

#endif
