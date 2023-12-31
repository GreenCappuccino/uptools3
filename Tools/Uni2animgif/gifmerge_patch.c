/*
 * gifmerge.c
 *
 * Copyright (C) 1990,1991,1992,1993 by Mark Podlipec. 
 * All rights reserved.
 *
 * This software may be freely copied, modified and redistributed
 * without fee provided that this copyright notice is preserved 
 * intact on all copies and modified copies.
 * 
 * There is no warranty or other guarantee of fitness of this software.
 * It is provided solely "as is". The author(s) disclaim(s) all
 * responsibility and liability with respect to this software's usage
 * or its effect upon hardware or computer systems.
 *
 */
 /*
  * Description:
  *
  * This program reads a gif91(see XAnim docs) file and merges the listed
  * gif files into one gif file. 
  *
  * Eventually, I'd like to have this program compare the current image
  * with the previous image and check to see if only a small section of
  * the screen changed from the previous image. Worth a shot.
  */
 /*
  * Rev 1.00	23Jul91	Mark Podlipec
  *	creation
  * Rev 1.01	08Jan92	Mark Podlipec
  *     use all colormaps, not just 1st.
  *
  * Rev 1.2    20Dec95  Rene Mueller
  *     command-line input (no longer txtfile needed)
  *
  * Rev 1.3    05Feb96  Rene Mueller (kiwi@iis.ee.ethz.ch)
  *     GIF89a transparency, and "Netscape2.0" application extension (looping)
  * Rev 1.31   14May96  Rene Mueller (kiwi@iis.ee.ethz.ch)
  *     disposal selectable
  * Rev 1.32   16Jul96  Rene Mueller (kiwi@iis.ee.ethz.ch)
  *     logical position per image manipulating
  * Rev 1.33   22Jul96  Rene Mueller (kiwi@iis.ee.ethz.ch)
  *     -notransp and -nopos added
  */
/* DECEMBER 1996: added -o<outfile> option (Louis Vuurpijl) */

#define DA_REV 1.33

#include "gifmerge_patch.h"

#define MAXVAL  4100            /* maxval of lzw coding size */
#define MAXVALP 4200

static int debug_flag = 0;  /* make these options */
static int verbose = 0;  /* make these options */
static int imagex = 0;
static int imagey = 0;
static int imagec = 0;

static GIF_Color gif_cmap[256];

static ULONG GIF_Get_Code();
static void GIF_Decompress();
static void GIF_Get_Next_Entry();
static void GIF_Add_To_Table();
static void GIF_Send_Data();
static void GIF_Clear_Table();
static void GIF_Screen_Header();
static void GIF_Image_Header();

/* used extern  */
void GIF_Read_File();

static GIF_Screen_Hdr gifscrn;
static GIF_Image_Hdr gifimage;
static GIF_Table table[MAXVALP];

static ULONG root_code_size,code_size,CLEAR,EOI,INCSIZE;
static ULONG nextab;
static ULONG gif_mask[16] = {1,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,0,0};
static ULONG gif_ptwo[16] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,0,0};
static UBYTE gif_buff[MAXVALP];
static ULONG gif_block_size;
static int num_bits,bits;

static int pic_i;
static char gif_file_name[256];
static int screen_was_last;

static int disposal = 2;
static int repeats = -1;
static int delay = 50;
static int transp = -1;
static int pos_set = 0;
static int xpos = 0, ypos = 0;

static void TheEnd()
{
 exit(0);
}

static void TheEnd1(p)
char *p;
{
 fprintf(stderr,"%s",p);
 TheEnd();
}

void GifMergeInstall (char *fname, FILE *fp_out, int dt, int nloop)
{
	int r,g,b;

	fprintf(stderr,"=== GIFMerge Rev %2.2f (C) 1991,1992 by Mark Podlipec\n",DA_REV);
	fprintf (stderr,"   Improvements by Rene K. Mueller 1996\n");
	fprintf (stderr,"   Patched and stolen by Louis Vuurpijl 1996\n");

	r = g = b = 192;
	transp = r|(g<<8)|(b<<16);
	delay = dt;
	repeats = nloop;
	disposal = 1;
	strcpy(gif_file_name,fname);
	xpos = ypos = 0;
	GIF_Read_File(fp_out,gif_file_name,1);
	pos_set = 1;
}


void GifMergeReset (char *fname, FILE *fp_out)
{
	disposal = 2;
	strcpy(gif_file_name,fname);
	xpos = ypos = 0;
	GIF_Read_File(fp_out,gif_file_name,0);
	disposal = 1;
	GIF_Read_File(fp_out,gif_file_name,0);
}

void GifMergeFile (char *fname, FILE *fp_out, int x, int y)
{
	if (fname[0]=='\0')
		return;
	xpos = x;
	ypos = y;
	strcpy(gif_file_name,fname);
	GIF_Read_File(fp_out,gif_file_name,0);
}


void GifMergeClose (FILE *fp_out)
{
	fputc(';',fp_out); /* image separator */
	fclose(fp_out);
}


/*
 * Read a GIF file, outputting to fname as we go.
 * It would be faster to read and write the individual blocks,
 * but eventually we'd like to optimize based on changes from
 * previous images(ie only a small section of the image changed.
 */
void
GIF_Read_File(fp_out,fname,first_image)
FILE *fp_out;
char *fname;
int first_image;
{
 FILE *fp;
 int i;

 if ( (fp=fopen(fname,"r"))==0)
 { 
  fprintf(stderr,"Can't open %s for reading.\n",fname); 
  TheEnd();
 }

 GIF_Screen_Header(fp,fp_out,first_image);

 /*** read until  ,  separator */
 do
 {
  i=fgetc(fp);
  if ( (i<0) && feof(fp))
  {
   fclose(fp);
   TheEnd1("GIF_Read_Header: Unexpected End of File\n");
  }
 } while(i != ',');

 fputc(',',fp_out); /* image separator */

 GIF_Image_Header(fp,fp_out,first_image);

 /*** Setup ACTION for IMAGE */

 GIF_Decompress(fp,fp_out,0);
 fputc(0,fp_out);  /* block count of zero */

 fclose(fp);
}

void GIF_Decompress(fp,fout)
FILE *fp,*fout;
{
 register ULONG code,old;

 pic_i = 0;
 bits=0;
 num_bits=0;
 gif_block_size=0;
    /* starting code size of LZW */
 root_code_size=(fgetc(fp) & 0xff); fputc(root_code_size,fout);
 GIF_Clear_Table();                /* clear decoding symbol table */

 code=GIF_Get_Code(fp,fout);

 if (code==CLEAR) 
 {
  GIF_Clear_Table(); 
  code=GIF_Get_Code(fp,fout);
 }
 /* write code(or what it currently stands for) to file */
 GIF_Send_Data(code);   
 old=code;
 code=GIF_Get_Code(fp,fout);
 do
 {
  if (table[code].valid==1)    /* if known code */
  {
       /* send it's associated string to file */
    GIF_Send_Data(code);
    GIF_Get_Next_Entry(fp);       /* get next table entry (nextab) */
    GIF_Add_To_Table(old,code,nextab);  /* add old+code to table */
    old=code;
  }
  else      /* code doesn't exist */
  {
    GIF_Add_To_Table(old,old,code);   /* add old+old to table */
    GIF_Send_Data(code);
    old=code;
  }
  code=GIF_Get_Code(fp,fout);
  if (code==CLEAR)
  { 
   GIF_Clear_Table();
   code=GIF_Get_Code(fp,fout);
   GIF_Send_Data(code);
   old=code;
   code=GIF_Get_Code(fp,fout);
  }
 } while(code!=EOI);
}

void GIF_Get_Next_Entry(fp)
FILE *fp;
{
   /* table walk to empty spot */
 while(  (table[nextab].valid==1)
       &&(nextab<MAXVAL)
      ) nextab++;
 /* 
  * Ran out of space??!?  Something's roached 
  */
 if (nextab>=MAXVAL)    
 { 
  fprintf(stderr,"Error: GetNext nextab=%d\n",nextab);
  fclose(fp);
  TheEnd();
 }
 if (nextab==INCSIZE)   /* go to next table size (and LZW code size ) */
 {
   /* fprintf(stderr,"GetNext INCSIZE was %d ",nextab); */
   code_size++; INCSIZE=(INCSIZE*2)+1;
   if (code_size>=12) code_size=12;
/*   fprintf(stderr,"<%ld>",INCSIZE); */
 }

}
/*  body is associated string
    next is code to add to that string to form associated string for
    index
*/     

void GIF_Add_To_Table(body,next,index)
register ULONG body,next,index;
{
 if (index>MAXVAL)
 { 
  fprintf(stderr,"Error index=%d\n",index);
 }
 else
 {
  table[index].valid=1;
  table[index].data=table[next].first;
  table[index].first=table[body].first;
  table[index].last=body;
 }
}

void GIF_Send_Data(index)
register int index;
{
 register int i,j;
 i=0;
 do         /* table walk to retrieve string associated with index */
 { 
  gif_buff[i]=table[index].data; 
  i++;
  index=table[index].last;
  if (i>MAXVAL)
  { 
   fprintf(stderr,"Error: Sending i=%d index=%d\n",i,index);
   TheEnd();
  }
 } while(index>=0);

 /* now invert that string since we retreived it backwards */
 i--;
 for(j=i;j>=0;j--)
 {
  /*pic[pic_i] = gif_buff[j] | gif_pix_offset;*/
  pic_i++;
 }
}


/* 
 * initialize string table 
 */
void GIF_Init_Table()       
{
 register int maxi,i;

if (debug_flag) fprintf(stderr,"Initing Table...");
 maxi=gif_ptwo[root_code_size];
 for(i=0; i<maxi; i++)
 {
  table[i].data=i;   
  table[i].first=i;
  table[i].valid=1;  
  table[i].last = -1;
 }
 CLEAR=maxi; 
 EOI=maxi+1; 
 nextab=maxi+2;
 INCSIZE = (2*maxi)-1;
 code_size=root_code_size+1;
}


/* 
 * clear table 
 */
void GIF_Clear_Table()   
{
 register int i;
if (debug_flag) fprintf(stderr,"Clearing Table...\n");
 for(i=0;i<MAXVAL;i++) table[i].valid=0;
 GIF_Init_Table();
}

/*CODE*/
ULONG GIF_Get_Code(fp,fout) /* get code depending of current LZW code size */
FILE *fp,*fout;
{
 ULONG code;
 register int tmp;

 while(num_bits < code_size)
 {
  /**** if at end of a block, start new block */
  if (gif_block_size==0) 
  {
   tmp = fgetc(fp);
   if (tmp >= 0 )
   {
    fputc(tmp,fout);
    gif_block_size=(ULONG)(tmp);
   }
   else TheEnd1("EOF in data stream\n");
  }

  tmp = fgetc(fp);   gif_block_size--;
  if (tmp >= 0)
  {
   fputc(tmp,fout);
   bits |= ( ((ULONG)(tmp) & 0xff) << num_bits );
   num_bits+=8;
  }
  else TheEnd1("EOF in data stream\n");
 }
  
 code = bits & gif_mask[code_size];
 bits >>= code_size;
 num_bits -= code_size; 


 if (code>MAXVAL)
 { 
  fprintf(stderr,"\nError! in stream=%x \n",code); 
  fprintf(stderr,"CLEAR=%x INCSIZE=%x EOI=%x code_size=%x \n",
                                           CLEAR,INCSIZE,EOI,code_size); 
  code=EOI;
 }

 if (code==INCSIZE)
 {
  if (code_size<12)
  {
   code_size++; INCSIZE=(INCSIZE*2)+1;
  }
  else if (debug_flag) fprintf(stderr,"<13?>"); 
 }

 return(code);
}


int GIF_Get_Short(fp,fout,first_time)
FILE *fp,*fout;
int first_time;
{
 register int temp,tmp1;
 temp=fgetc(fp);	 if (first_time==TRUE) fputc(temp,fout);
 tmp1=fgetc(fp);	 if (first_time==TRUE) fputc(tmp1,fout);
 return(temp|( (tmp1) << 8 ));
}

void GIF_Put_Short(fout,data)
FILE *fout; unsigned int data;
{
 fputc((unsigned char)(data&255),fout);      /* lo */
 fputc((unsigned char)((data>>8)&255),fout); /* hi */
}

/* 
 * read GIF header 
 */
void GIF_Screen_Header(fp,fout,first_time)
FILE *fp,*fout;
int first_time;
{
 int temp,i;

 for(i=0;i<6;i++) {
  temp = fgetc(fp);
/*   if (first_time==TRUE) fputc(temp,fout); */
 }
 if(first_time) 
   fputs("GIF89a",fout);
 gifscrn.width  = GIF_Get_Short(fp,fout,first_time);
 gifscrn.height = GIF_Get_Short(fp,fout,first_time);
 temp=fgetc(fp);		 if (first_time==TRUE) fputc(temp,fout);
 gifscrn.m       =  temp & 0x80;
 gifscrn.cres    = (temp & 0x70) >> 4;
 gifscrn.pixbits =  temp & 0x07;
 gifscrn.bc  = fgetc(fp);	 if (first_time==TRUE) fputc(gifscrn.bc,fout);
 temp=fgetc(fp);		 if (first_time==TRUE) fputc(temp,fout);
 imagec=gif_ptwo[(1+gifscrn.pixbits)];

 if (verbose)
  fprintf(stderr,"Screen: %dx%dx%d m=%d cres=%d bkgnd=%d pix=%d\n",
    gifscrn.width,gifscrn.height,imagec,gifscrn.m,gifscrn.cres,
    gifscrn.bc,gifscrn.pixbits);

 if (gifscrn.m)
 {
  for(i=0;i<imagec;i++)
  {
   gif_cmap[i].cmap.red   = temp = fgetc(fp); 
           if (first_time==TRUE) fputc(temp,fout);
   gif_cmap[i].cmap.green = temp = fgetc(fp); 
           if (first_time==TRUE) fputc(temp,fout);
   gif_cmap[i].cmap.blue  = temp = fgetc(fp); 
           if (first_time==TRUE) fputc(temp,fout);
  }
 }
 screen_was_last = TRUE;
	if(gifscrn.m&&(transp>=0||delay>=0)) {
		int ix = 0, max_dist = 3*256;
		if(transp>=0) {
			for(i=0; i<imagec; i++) {
				int dist = 
					abs(gif_cmap[i].cmap.red-(transp&255))+
					abs(gif_cmap[i].cmap.green-((transp>>8)&255))+
					abs(gif_cmap[i].cmap.blue-(transp>>16));
				if(dist<max_dist) 
					ix = i, max_dist = dist;
			} 
#ifdef _DEBUG_COL
			if(max_dist!=0)
         	fprintf(stderr,"Transparent not matched fully, col #%d (%d,%d,%d) used now\n"
					,ix,gif_cmap[ix].cmap.red,gif_cmap[ix].cmap.green,gif_cmap[ix].cmap.blue);
#endif
   }
   fputc(0x21,fout);
   fputc(0xF9,fout);
   fputc(0x04,fout);
   fputc((transp>=0?0x01:0x00)|(disposal<<2),fout);
   fputc(delay&255,fout);
   fputc((unsigned)delay>>8,fout);
   fputc(ix,fout);
   fputc(0x00,fout);
 }
 if(first_time&&repeats>=0) { /* documentation look at */
   fputc(0x21,fout);          /* http://www.reiworld.com/royalef/gifabout.htm */
   fputc(0xFF,fout);
   fputc(0x0B,fout);
   fputs("NETSCAPE2.0",fout);
   fputc(0x03,fout);
   fputc(0x01,fout);
   fputc(repeats&255,fout);
   fputc((unsigned)repeats>>8,fout);
   fputc(0x00,fout);
 }
}

void GIF_Image_Header(fp,fout,first_time)
FILE *fp,*fout;
int first_time;
{
 int temp,tnum,i,tmp;

 tmp = GIF_Get_Short(fp,fout,0); if(!pos_set) xpos = tmp;
 gifimage.left   = xpos; GIF_Put_Short(fout,xpos);
 tmp = GIF_Get_Short(fp,fout,0); if(!pos_set) ypos = tmp;
 gifimage.top    = ypos; GIF_Put_Short(fout,ypos);
 gifimage.width  = GIF_Get_Short(fp,fout,1);
 gifimage.height = GIF_Get_Short(fp,fout,1);
 temp=fgetc(fp); 

 gifimage.m        = temp & 0x80;
 gifimage.i        = temp & 0x40;
 gifimage.pixbits  = temp & 0x07;

 if (screen_was_last && (first_time==FALSE)) temp |= 0x80;
 temp &= 0xf8;
 temp |= gifscrn.pixbits;
 fputc(temp,fout);

 imagex=gifimage.width;
 imagey=gifimage.height;
 tnum=gif_ptwo[(1+gifimage.pixbits)];
 if (verbose)
  fprintf(stderr,"Image: %dx%dx%d m=%d i=%d pix=%d \n",
    imagex,imagey,tnum,gifimage.m,gifimage.i,gifimage.pixbits);

 /* if there is an image cmap, then use it */
 if (gifimage.m)
 {
  for(i=0;i<tnum;i++)
  {
   gif_cmap[i].cmap.red   = temp = fgetc(fp); fputc(temp,fout);
   gif_cmap[i].cmap.green = temp = fgetc(fp); fputc(temp,fout);
   gif_cmap[i].cmap.blue  = temp = fgetc(fp); fputc(temp,fout);
  }
 }  /* else if screen was last not 1st time */
 else if (screen_was_last && (first_time==FALSE))
 {
  for(i=0;i<imagec;i++)
  {
   fputc(gif_cmap[i].cmap.red  ,fout);
   fputc(gif_cmap[i].cmap.green,fout);
   fputc(gif_cmap[i].cmap.blue ,fout);
  }
 }
 screen_was_last = FALSE; 
}
