#include <stdio.h>
#include <stdlib.h>

#include "examine_gif.h"

static int get_int (FILE *fp)
{
	register int temp,tmp1;
	temp=fgetc(fp);
	tmp1=fgetc(fp);
	return(temp|( (tmp1) << 8 ));
}

int examine_gif (char *fname, int *width, int *height)
{
	char header[128];
	FILE *fp;

	fp=fopen(fname,"rb");
	if (fp==NULL) {
		fprintf (stderr,"unable to open gif '%s'!!\n",fname);
		fclose(fp);
		return 0;
	}
	fread(header,6,1,fp);
	header[6]='\0';
	if ( (strcmp(header,"GIF87a")!=0) && (strcmp(header,"GIF89a")!=0) ) {
		fprintf (stderr,"expecting gifformat [GIF89a|GIF87a], got '%s'!!\n",header);
		fclose(fp);
		return 0;
	}
	*width  = get_int(fp);
	*height = get_int(fp);
	printf ("got %s %dx%d\n",header,*width,*height);
	fclose(fp);
	return 1;
}
