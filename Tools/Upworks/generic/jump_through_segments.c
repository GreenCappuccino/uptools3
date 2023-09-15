#include <stdio.h>
#include <stdlib.h>
#include "uplib.h"
#include "upsiglib.h"
#include "uphierarchy.h"

#include "jump_through_segments.h"

void add_sample (int addition, int mark, int nsamples, int *MSTART, int *MEND)
{
	switch (mark) {
		case 0: case 2:       /* more left */
			*MSTART -= addition;
			break;
		case 1: case 3:       /* less left */
			*MSTART += addition;
			if (*MEND<=*MSTART)
				*MSTART = *MEND;
			break;
		case 4: case 6:       /* less right */
			*MEND   -= addition;
			if (*MEND<=*MSTART)
				*MEND = *MSTART;
			break;
		case 5: case 7:       /* more right */
			*MEND   += addition;
			break;
	}
}

void add_y_extremum (sigSignal *sig, int mark, int *MSTART, int *MEND)
{
	int i,y0,y1,y2;
	int m_start,m_end;

	m_start = *MSTART;
	m_end   = *MEND;

	switch (mark) {
		case 0: case 2: /* more left */
			y0 = y1 = sig->y[m_start];
			for (i=m_start-1;i>=0;i--) {
				y2 = sig->y[i];
				while (y2==y1 &&i<m_end) {
					i--;
					y2 = sig->y[i];
				}
				if (i==0) {
					m_start = 0;
					break;
				}
				if (y2<=y1&&y0>=y1) {        /* (still) goin down */
					y0 = y1;
					y1 = y2;
				} else if (y2>=y1&&y0<=y1) { /* (still) goin up */
					y0 = y1;
					y1 = y2;
				} else {                     /* y1 is minimum */
					m_start = i+1;
					break;
				}
			}
			if (i==0)
				m_start = 0;
			break;
		case 1: case 3: /* less left */
			y0 = y1 = sig->y[m_start];
			for (i=m_start+1;i<m_end;i++) {
				y2 = sig->y[i];
				while (y2==y1 &&i<m_end) {
					i++;
					y2 = sig->y[i];
				}
				if (i==m_end) {
					m_start = m_end-1;
					break;
				}
				if (y2<=y1&&y0>=y1) {        /* (still) goin down */
					y0 = y1;
					y1 = y2;
				} else if (y2>=y1&&y0<=y1) { /* (still) goin up */
					y0 = y1;
					y1 = y2;
				} else {                     /* y1 is minimum */
					m_start = i-1;
					break;
				}
			}
			if (i==m_end)
				m_start = m_end-1;
			break;
		case 4: case 6: /* less right */
			y0 = y1 = sig->y[m_end];
			for (i=m_end-1;i>m_start;i--) {
				y2 = sig->y[i];
				while (y2==y1 &&i>m_start) {
					i--;
					y2 = sig->y[i];
				}
				if (i==m_start) {
					m_end = m_start+1;
					break;
				}
				if (y2<=y1&&y0>=y1) {        /* (still) goin down */
					y0 = y1;
					y1 = y2;
				} else if (y2>=y1&&y0<=y1) { /* (still) goin up */
					y0 = y1;
					y1 = y2;
				} else {                     /* y1 is minimum */
					m_end = i+1;
					break;
				}
			}
			if (i==m_start)
				m_end = m_start+1;
			break;
		case 5: case 7: /* more right */
			y0 = y1 = sig->y[m_end];
			for (i=m_end+1;i<sig->nsamples;i++) {
				y2 = sig->y[i];
				while (y2==y1 &&i<sig->nsamples) {
					i++;
					y2 = sig->y[i];
				}
				if (i==sig->nsamples) {
					m_end = sig->nsamples-1;
					break;
				}
				if (y2<=y1&&y0>=y1) {        /* (still) goin down */
					y0 = y1;
					y1 = y2;
				} else if (y2>=y1&&y0<=y1) { /* (still) goin up */
					y0 = y1;
					y1 = y2;
				} else {                     /* y1 is minimum */
					m_end = i-1;
					break;
				}
			}
			if (i==sig->nsamples)
				m_end = sig->nsamples-1;
			break;
	}
	*MSTART = m_start;
	*MEND   = m_end;
}

int walk_via_penstream (sigSignal *sig, int curpos, int addition)
{
	int curlift;
	int i;

	if (addition<0) { /* walking leftwards */
		if (curpos<=0)
			return 0;
		curlift = sig->z[curpos-1];
		for (i=curpos-1;i>=0;i--) {
			if (sig->z[i]!=-1)
				if (sig->z[i]!=curlift)
					return i+1;
		}
		if (i<0)
			return 0;
	}
	else { /* walking rightwards */
		if (curpos>=sig->nsamples-1)
			return sig->nsamples-1;
		curlift = sig->z[curpos+1];
		for (i=curpos+1;i<sig->nsamples;i++) {
			if (sig->z[i]!=-1)
				if (sig->z[i]!=curlift)
					return i-1;
		}
		if (i>=sig->nsamples)
			return sig->nsamples;
	}
	return curpos;
}

void add_penstream (sigSignal *sig, int mark, int *MSTART, int *MEND)
{
	switch (mark) {
		case 0: case 2:       /* more left */
			*MSTART = walk_via_penstream(sig,*MSTART,-1);
			break;
		case 1: case 3:       /* less left */
			*MSTART = walk_via_penstream(sig,*MSTART,+1);
			*MSTART += 1;
			if (*MSTART>*MEND)
				*MSTART = *MEND;
			break;
		case 4: case 6:       /* less right */
			*MEND = walk_via_penstream(sig,*MEND,-1);
			if (*MEND<*MSTART)
				*MEND = *MSTART;
			break;
		case 5: case 7:       /* more right */
			*MEND = walk_via_penstream(sig,*MEND,+1);
			break;
	}
}

/**           AND NOW FOR SOMETHING COMPLETELY DIFFERENT                     **/
/**                  ADDING OR DELETING SEGMENTS                             **/

/** This routine is called if a user wants to change the delineation of a segment,
    so his actions <more-right>, <less-right>, <more-left>, <less-left> should
    always have some side-effect, unless they are really not possible.

    Two pointers are used for determining which segment has to be added/deleted,
	 s = start and e = end. It is assured that s<e.

	 <more-left>:  select the closest segment having start ss<s, don't touch e
	 <less-left>:  select the closest segment having start ss>s, don't touch e
	 <more-right>: select the closest segment having end ee>e, don't touch s
	 <less-right>: select the closest segment having end ee<e, don't touch s

**/ 

void add_segment_to_signal (sigSignal *sig, tUPUnipen *p, tUPLevel *hierarchies
	,int mark, int level, int segnr, int childlevel, int *MSTART, int *MEND)
{
	tUPHierarchy *h,*segments,*c;
	int i,offset,nsamples;
	int s,e,ss,ee,si,ei,dmin;

	h = &hierarchies[level].segments[segnr];
	ss = s = *MSTART;
	ee = e = *MEND;
	segments = hierarchies[childlevel].segments;
	dmin = sig->nsamples+1;
	for (i=0;i<hierarchies[childlevel].nsegments;i++) {
		c = &segments[i];
		if (!sigSegmentInSegment(h->del,c->del))
			continue;
		sigSegmentBoundsInDelineation(p,h->del,c->del,&offset,&nsamples);
		si = offset;
		ei = offset+nsamples-1;
		switch (mark) {
			case 0: case 2:       /* more left */
				if (si<s) {
					if (dmin>s-si) {
						ss = si;
						dmin = s-si;
					}
				}
				break;
			case 1: case 3:       /* less left */
				if (si>s) {
					if (dmin>si-s) {
						ss = si;
						dmin = si-s;
					}
				}
				break;
			case 4: case 6:       /* less right */
				if (ei<e) {
					if (dmin>e-ei) {
						ee = ei;
						dmin = e-ei;
					}
				}
				break;
			case 5: case 7:       /* more right */
				if (ei>e) {
					if (dmin>ei-e) {
						ee = ei;
						dmin = ei-e;
					}
				}
				break;
			default:
				fprintf (stderr,"ERROR: direction %d must be in [0-7]!\n",mark);
				return;
		}
	}
	switch (mark) {
		case 0: case 2:
			if (ss==s) ss = 0;
			break;
		case 5: case 7:
			if (ee==e) ee = sig->nsamples-1;
	}
	if (ss<ee) {
		*MSTART = ss;
		*MEND   = ee;
	}
}

int do_get_child_markers (int mark, int jumpsize, int level, int segnr, int childlevel
	,int *MSTART, int *MEND
	,tUPLevel *hierarchies
	,sigSignal *sig, tUPUnipen *p, char *delbuf)
{
	sigDelineation *del;
	sigDelineation *hdel;

	del = hierarchies[level].segments[segnr].del;
	hdel = (sigDelineation *) malloc (sizeof(sigDelineation));
	hdel->delineations = (sigSampleDelineation *) malloc (sizeof(sigSampleDelineation));
	hdel->ndels = 1;
	hdel->delineations[0].first_streamnr = del->delineations[0].first_streamnr;
	hdel->delineations[0].first_samplenr = del->delineations[0].first_samplenr;
	hdel->delineations[0].last_streamnr = del->delineations[del->ndels-1].last_streamnr;
	hdel->delineations[0].last_samplenr = del->delineations[del->ndels-1].last_samplenr;
	sig = sigDelineation2Signal(p,hdel);
	switch (jumpsize) {
		case JUMP_ONE_SAMPLE:
			add_sample(1,mark,sig->nsamples,MSTART,MEND);
			break;
		case JUMP_TEN_SAMPLES:
			add_sample(10,mark,sig->nsamples,MSTART,MEND);
			break;
		case JUMP_HUNDRED_SAMPLES:
			add_sample(100,mark,sig->nsamples,MSTART,MEND);
			break;
		case JUMP_THOUSEND_SAMPLES:
			add_sample(1000,mark,sig->nsamples,MSTART,MEND);
			break;
		case JUMP_Y_EXTREMA:
			add_y_extremum(sig,mark,MSTART,MEND);
			break;
		case JUMP_PEN_STREAM:
			add_penstream(sig,mark,MSTART,MEND);
			break;
		default:
			add_segment_to_signal(sig,p,hierarchies,mark,level,segnr,childlevel,MSTART,MEND);
	}
	if (*MEND>=sig->nsamples)
		*MEND = sig->nsamples-1;
	if (*MSTART<0)
		*MSTART = 0;

/*
	del = sigBounds2Delineation(p,hierarchies[level].segments[segnr].entry,sig,*MSTART,*MEND);
*/
	del = sigDelBounds2Delineation(p,hdel,sig,*MSTART,*MEND);
	sigFreeDelineation(hdel);
	sprintf (delbuf,"%d:%d-%d:%d"
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[del->ndels-1].last_streamnr
		,del->delineations[del->ndels-1].last_samplenr);
	sigDeleteSignal(sig);
	sigFreeDelineation(del);
	return 1;
}

/* and something to edit segments without a CONCRETE WALL */

static sigDelineation *segment_del = NULL;
static int segment_firststr,segment_laststr,
	segment_firstsam,segment_lastsam,max_streams;

static void jump_samples (tUPUnipen *p, int addition, int mark)
{
	int fsam,lsam,fstr,lstr,n;

	fstr = segment_del->delineations[0].first_streamnr;
	fsam = segment_del->delineations[0].first_samplenr;
	lstr = segment_del->delineations[0].last_streamnr;
	lsam = segment_del->delineations[0].last_samplenr;

	switch (mark) {
		case 0: case 2:       /* more left */
			while (fsam<addition) {
				if (fstr==0) {
					fprintf (stderr,"already at start of UNIPEN file!\n");
					return;
				}
				addition -= fsam;
				fsam = p->stream_sequence[--fstr].nsamples-1;
			}
			fsam -= addition;
			segment_del->delineations[0].first_streamnr = fstr;
			segment_del->delineations[0].first_samplenr = fsam;
			break;
		case 1: case 3:       /* less left */
			n = p->stream_sequence[fstr].nsamples-1;
			while (fsam+addition>n) {
				addition -= n-fsam;
				fsam = 0;
				n = p->stream_sequence[++fstr].nsamples-1;
			}
			fsam += addition;
			segment_del->delineations[0].first_streamnr = fstr;
			segment_del->delineations[0].first_samplenr = fsam;
			break;
		case 4: case 6:       /* less right */
			while (lsam<addition) {
				if (lstr==0) {
					fprintf (stderr,"already at start of UNIPEN file!\n");
					return;
				}
				addition -= lsam;
				lsam = p->stream_sequence[--lstr].nsamples-1;
			}
			lsam -= addition;
			segment_del->delineations[0].last_streamnr = lstr;
			segment_del->delineations[0].last_samplenr = lsam;
			break;
		case 5: case 7:       /* more right */
			n = p->stream_sequence[lstr].nsamples-1;
			while (lsam+addition>n) {
				addition -= n-lsam;
				lsam = 0;
				n = p->stream_sequence[++lstr].nsamples-1;
			}
			lsam += addition;
			segment_del->delineations[0].last_streamnr = lstr;
			segment_del->delineations[0].last_samplenr = lsam;
			break;
	}
	if (segment_del->delineations[0].first_streamnr==segment_del->delineations[0].last_streamnr) {
		if (segment_del->delineations[0].first_samplenr==segment_del->delineations[0].last_samplenr) {
			segment_del->delineations[0].first_streamnr  = segment_del->delineations[0].last_streamnr;
			segment_del->delineations[0].first_samplenr = segment_del->delineations[0].last_samplenr;
		}
	}
	if (segment_del->delineations[0].first_streamnr>segment_del->delineations[0].last_streamnr) {
		segment_del->delineations[0].first_streamnr  = segment_del->delineations[0].last_streamnr;
		segment_del->delineations[0].first_samplenr = segment_del->delineations[0].last_samplenr;
	}
}

static void jump_y_extremum (tUPUnipen *p, int addition, int mark)
{
}

static void jump_pen_stream (tUPUnipen *p, int mark)
{
	int fsam,lsam,fstr,lstr;

	fstr = segment_del->delineations[0].first_streamnr;
	fsam = segment_del->delineations[0].first_samplenr;
	lstr = segment_del->delineations[0].last_streamnr;
	lsam = segment_del->delineations[0].last_samplenr;

	switch (mark) {
		case 0: case 2:       /* more left */
			if (fsam!=0)
				segment_del->delineations[0].first_samplenr = 0;
			else if (segment_del->delineations[0].first_streamnr>0)
				segment_del->delineations[0].first_streamnr = 0;
			break;
		case 1: case 3:       /* less left */
			if  (fstr<lstr){
				segment_del->delineations[0].first_streamnr++;
				segment_del->delineations[0].first_samplenr = 0;
			} else if (fstr==lstr) {
				segment_del->delineations[0].first_samplenr = lsam;
			}
			break;
		case 4: case 6:       /* less right */
			if (lstr>fstr) {
				segment_del->delineations[0].last_streamnr--;
				segment_del->delineations[0].last_samplenr = p->stream_sequence[lstr-1].nsamples-1;
			} else if (fstr==lstr) {
				segment_del->delineations[0].last_samplenr = lsam;
			}
			break;
		case 5: case 7:       /* more right */
			if (lsam!=p->stream_sequence[lstr].nsamples-1)
				segment_del->delineations[0].last_samplenr = p->stream_sequence[lstr].nsamples-1;
			else if (segment_del->delineations[0].last_streamnr<max_streams-1)
				segment_del->delineations[0].last_streamnr++;
				segment_del->delineations[0].last_samplenr = p->stream_sequence[lstr+1].nsamples-1;
			break;
	}
}

void jump_segment_to_signal (tUPUnipen *p, tUPLevel *hierarchies
	,int mark, int level, int segnr, int childlevel)
{
	tUPHierarchy *h,*segments,*c;
	int i,min_str,min_sam;
	int fsam,lsam,fstr,lstr;
	int fsm_i,lsm_i,fst_i,lst_i;
	int fsm,lsm,fst,lst;
	sigDelineation *del;

	fst = fstr = segment_del->delineations[0].first_streamnr;
	fsm = fsam = segment_del->delineations[0].first_samplenr;
	lst = lstr = segment_del->delineations[0].last_streamnr;
	lsm = lsam = segment_del->delineations[0].last_samplenr;

	h = &hierarchies[level].segments[segnr];
	segments = hierarchies[childlevel].segments;
	min_str = 9999999;
	min_sam = 9999999;
	for (i=0;i<hierarchies[childlevel].nsegments;i++) {
		c = &segments[i];
		del = sigEntry2Delineation(p,c->entry);
		fst_i = del->delineations[0].first_streamnr;
		fsm_i = del->delineations[0].first_samplenr;
		lst_i = del->delineations[del->ndels-1].last_streamnr;
		lsm_i = del->delineations[del->ndels-1].last_samplenr;
		switch (mark) {
			case 0: case 2:       /* more left */
				if (fst_i<fstr) {
					if (min_str>fstr-fst_i) {
						fst = fst_i;
						min_str = fstr-fst_i;
						min_sam = fsam-fsm_i;
						fsm = fsm_i;
					} else if (min_str==fstr-fst_i) {
						if (min_sam>fsam-fsm_i) {
							fsm = fsm_i;
							min_sam = fsam-fsm_i;
						}
					}
				}
				break;
			case 1: case 3:       /* less left */
				if (fst_i>fstr) {
					if (min_str>fst_i-fstr) {
						fst = fst_i;
						min_str = fst_i-fstr;
						min_sam = fsm_i-fsam;
						fsm = fsm_i;
					} else if (min_str==fst_i-fstr) {
						if (min_sam>fsm_i-fsam) {
							fsm = fsm_i;
							min_sam = fsm_i-fsam;
						}
					}
				}
				break;
			case 4: case 6:       /* less right */
				if (lst_i<lstr) {
					if (min_str>lstr-lst_i) {
						lst = lst_i;
						min_str = lstr-lst_i;
						min_sam = lsam-lsm_i;
						lsm = lsm_i;
					} else if (min_str==lstr-lst_i) {
						if (min_sam>lsam-lsm_i) {
							lsm = lsm_i;
							min_sam = lsam-lsm_i;
						}
					}
				}
				break;
			case 5: case 7:       /* more right */
				if (lst_i>lstr) {
					if (min_str>lst_i-lstr) {
						lst = lst_i;
						min_str = lst_i-lstr;
						min_sam = lsm_i-lsam;
						lsm = lsm_i;
					} else if (min_str==lst_i-lstr) {
						if (min_sam>lsm_i-lsam) {
							lsm = lsm_i;
							min_sam = lsm_i-lsam;
						}
					}
				}
				break;
			default:
				fprintf (stderr,"ERROR: direction %d must be in [0-7]!\n",mark);
				return;
		}
		sigFreeDelineation(del);
	}
	segment_del->delineations[0].first_streamnr = fst;
	segment_del->delineations[0].first_samplenr = fsm;
	segment_del->delineations[0].last_streamnr  = lst;
	segment_del->delineations[0].last_samplenr  = lsm;
}

int do_set_segm_markers (int mark, int jumpsize, int level, int segnr, int childlevel
	,tUPLevel *hierarchies
	, tUPUnipen *p)
{
	int fsam,lsam,fstr,lstr;

	fstr = segment_del->delineations[0].first_streamnr;
	fsam = segment_del->delineations[0].first_samplenr;
	lstr = segment_del->delineations[0].last_streamnr;
	lsam = segment_del->delineations[0].last_samplenr;

	switch (jumpsize) {
		case JUMP_ONE_SAMPLE:
			jump_samples(p,1,mark);
			break;
		case JUMP_TEN_SAMPLES:
			jump_samples(p,10,mark);
			break;
		case JUMP_HUNDRED_SAMPLES:
			jump_samples(p,100,mark);
			break;
		case JUMP_THOUSEND_SAMPLES:
			jump_samples(p,1000,mark);
			break;
		case JUMP_Y_EXTREMA:
			break;
		case JUMP_PEN_STREAM:
			jump_pen_stream(p,mark);
			break;
		default:
			jump_segment_to_signal (p,hierarchies,mark,level,segnr,childlevel);
			break;
	}
	if (fstr == segment_del->delineations[0].first_streamnr  &&
		fsam == segment_del->delineations[0].first_samplenr  &&
		lstr == segment_del->delineations[0].last_streamnr   &&
		lsam == segment_del->delineations[0].last_samplenr)
		return 0;
	else
		return 1;
}

void initiate_segment_walk (tUPUnipen *p, tUPLevel *hierarchies, int l, int s)
{
	tUPHierarchy *h;
/*	char level[128],delineation[4096]; */
	sigDelineation *del;
	int n;

	if (segment_del!=NULL) {
		sigFreeDelineation(segment_del);
	}
	h = &hierarchies[l].segments[s];
/*	if (sscanf(h->entry->Entry,".SEGMENT%s%s",level,delineation)!=2) {
		fprintf (stderr,"unable to scan level and delineation from entry:\n%s\n",h->entry->Entry);
	}
	del = sigParseDelineation(delineation); */

	del = h->del;

	n = del->ndels - 1;
	segment_firststr = del->delineations[0].first_streamnr;
	segment_laststr  = del->delineations[n].last_streamnr;
	segment_firstsam = del->delineations[0].first_samplenr;
	segment_lastsam  = del->delineations[n].last_samplenr;
	segment_del = (sigDelineation *) malloc (sizeof(sigDelineation));
	segment_del->delineations = (sigSampleDelineation *) malloc (sizeof(sigSampleDelineation));
	segment_del->ndels = 1;
	segment_del->delineations[0].first_streamnr = segment_firststr;
	segment_del->delineations[0].last_streamnr  = segment_laststr;
	if (segment_firstsam<0) segment_firstsam = 0;
	segment_del->delineations[0].first_samplenr = segment_firstsam;
	if (segment_lastsam<0) segment_lastsam = p->stream_sequence[segment_laststr].nsamples-1;;
	segment_del->delineations[0].last_samplenr  = segment_lastsam;
/*	sigFreeDelineation(del); */
	max_streams = p->NrOfPenStreams;
}

sigSignal *do_get_edited_segment (tUPUnipen *p, char *delbuf)
{
	sprintf (delbuf,"%d:%d-%d:%d"
		,segment_del->delineations[0].first_streamnr
		,segment_del->delineations[0].first_samplenr
		,segment_del->delineations[0].last_streamnr
		,segment_del->delineations[0].last_samplenr);
	return sigDelineation2Signal (p,segment_del);
}

sigDelineation *copy_current_delineation (void)
{
	return sigCopyDelineation(segment_del);
}

int do_get_segment_delineation (tUPUnipen *p, int s_start, int s_end, char *delbuf)
{
	sigDelineation *del;
	sigSignal *sig;
	tUPEntry segm_entry;
	char entrybuf[256];

	sig = sigDelineation2Signal (p,segment_del);
	sprintf (entrybuf,".SEGMENT ?? %d:%d-%d:%d OK \"something\""
		,segment_del->delineations[0].first_streamnr
		,segment_del->delineations[0].first_samplenr
		,segment_del->delineations[0].last_streamnr
		,segment_del->delineations[0].last_samplenr);
	segm_entry.Entry = &entrybuf[0];
	del = sigBounds2Delineation(p,&segm_entry,sig,s_start,s_end);
	sprintf (delbuf,"%d:%d-%d:%d"
		,del->delineations[0].first_streamnr
		,del->delineations[0].first_samplenr
		,del->delineations[0].last_streamnr
		,del->delineations[0].last_samplenr);
	sigFreeDelineation(del);
	sigDeleteSignal(sig);
	return 1;
}
