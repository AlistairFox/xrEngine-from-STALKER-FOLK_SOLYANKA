/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: residue backend 0, 1 and 2 implementation
 last mod: $Id: res0.c 16227 2009-07-08 06:58:46Z xiphmont $

 ********************************************************************/

/* Slow, slow, slow, simpleminded and did I mention it was slow?  The
   encode/decode loops are coded for clarity and performance is not
   yet even a nagging little idea lurking in the shadows.  Oh and BTW,
   it's slow. */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"
#include "registry.h"
#include "codebook.h"
#include "misc.h"
#include "os.h"

#if defined(TRAIN_RES) || defined (TRAIN_RESAUX)
#include <stdio.h>
#endif

typedef struct {
  vorbis_info_residue0 *info;

  int         parts;
  int         stages;
  codebook   *fullbooks;
  codebook   *phrasebook;
  codebook ***partbooks;

  int         partvals;
  int       **decodemap;

  long      postbits;
  long      phrasebits;
  long      frames;

#if defined(TRAIN_RES) || defined(TRAIN_RESAUX)
  int        train_seq;
  long      *training_data[8][64];
  float      training_max[8][64];
  float      training_min[8][64];
  float     tmin;
  float     tmax;
#endif

} vorbis_look_residue0;

void res0_free_info(vorbis_info_residue *i){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)i;
  if(info){
    memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

void res0_free_look(vorbis_look_residue *i){
  int j;
  if(i){

    vorbis_look_residue0 *look=(vorbis_look_residue0 *)i;

#ifdef TRAIN_RES
    {
      int j,k,l;
      for(j=0;j<look->parts;j++){
        /*fprintf(stderr,"partition %d: ",j);*/
        for(k=0;k<8;k++)
          if(look->training_data[k][j]){
            char buffer[80];
            FILE *of;
            codebook *statebook=look->partbooks[j][k];

            /* long and short into the same bucket by current convention */
            sprintf(buffer,"res_part%d_pass%d.vqd",j,k);
            of=fopen(buffer,"a");

            for(l=0;l<statebook->entries;l++)
              fprintf(of,"%d:%ld\n",l,look->training_data[k][j][l]);

            fclose(of);

            /*fprintf(stderr,"%d(%.2f|%.2f) ",k,
              look->training_min[k][j],look->training_max[k][j]);*/

            _ogg_free(look->training_data[k][j]);
            look->training_data[k][j]=NULL;
          }
        /*fprintf(stderr,"\n");*/
      }
    }
    fprintf(stderr,"min/max residue: %g::%g\n",look->tmin,look->tmax);

    /*fprintf(stderr,"residue bit usage %f:%f (%f total)\n",
            (float)look->phrasebits/look->frames,
            (float)look->postbits/look->frames,
            (float)(look->postbits+look->phrasebits)/look->frames);*/
#endif


    /*vorbis_info_residue0 *info=look->info;

    fprintf(stderr,
            "%ld frames encoded in %ld phrasebits and %ld residue bits "
            "(%g/frame) \n",look->frames,look->phrasebits,
            look->resbitsflat,
            (look->phrasebits+look->resbitsflat)/(float)look->frames);

    for(j=0;j<look->parts;j++){
      long acc=0;
      fprintf(stderr,"\t[%d] == ",j);
      for(k=0;k<look->stages;k++)
        if((info->secondstages[j]>>k)&1){
          fprintf(stderr,"%ld,",look->resbits[j][k]);
          acc+=look->resbits[j][k];
        }

      fprintf(stderr,":: (%ld vals) %1.2fbits/sample\n",look->resvals[j],
              acc?(float)acc/(look->resvals[j]*info->grouping):0);
    }
    fprintf(stderr,"\n");*/

    for(j=0;j<look->parts;j++)
      if(look->partbooks[j])_ogg_free(look->partbooks[j]);
    _ogg_free(look->partbooks);
    for(j=0;j<look->partvals;j++)
      _ogg_free(look->decodemap[j]);
    _ogg_free(look->decodemap);

    memset(look,0,sizeof(*look));
    _ogg_free(look);
  }
}

static int ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static int icount(unsigned int v){
  int ret=0;
  while(v){
    ret+=v&1;
    v>>=1;
  }
  return(ret);
}


void res0_pack(vorbis_info_residue *vr,oggpack_buffer *opb){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)vr;
  int j,acc=0;
  oggpack_write(opb,info->begin,24);
  oggpack_write(opb,info->end,24);

  oggpack_write(opb,info->grouping-1,24);  /* residue vectors to group and
                                             code with a partitioned book */
  oggpack_write(opb,info->partitions-1,6); /* possible partition choices */
  oggpack_write(opb,info->groupbook,8);  /* group huffman book */

  /* secondstages is a bitmask; as encoding progresses pass by pass, a
     bitmask of one indicates this partition class has bits to write
     this pass */
  for(j=0;j<info->partitions;j++){
    if(ilog(info->secondstages[j])>3){
      /* yes, this is a minor hack due to not thinking ahead */
      oggpack_write(opb,info->secondstages[j],3);
      oggpack_write(opb,1,1);
      oggpack_write(opb,info->secondstages[j]>>3,5);
    }else
      oggpack_write(opb,info->secondstages[j],4); /* trailing zero */
    acc+=icount(info->secondstages[j]);
  }
  for(j=0;j<acc;j++)
    oggpack_write(opb,info->booklist[j],8);

}

/* vorbis_info is for range checking */
vorbis_info_residue *res0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int j,acc=0;
  vorbis_info_residue0 *info=_ogg_calloc(1,sizeof(*info));
  codec_setup_info     *ci=vi->codec_setup;

  info->begin=oggpack_read(opb,24);
  info->end=oggpack_read(opb,24);
  info->grouping=oggpack_read(opb,24)+1;
  info->partitions=oggpack_read(opb,6)+1;
  info->groupbook=oggpack_read(opb,8);

  /* check for premature EOP */
  if(info->groupbook<0)goto errout;

  for(j=0;j<info->partitions;j++){
    int cascade=oggpack_read(opb,3);
    int cflag=oggpack_read(opb,1);
    if(cflag<0) goto errout;
    if(cflag){
      int c=oggpack_read(opb,5);
      if(c<0) goto errout;
      cascade|=(c<<3);
    }
    info->secondstages[j]=cascade;

    acc+=icount(cascade);
  }
  for(j=0;j<acc;j++){
    int book=oggpack_read(opb,8);
    if(book<0) goto errout;
    info->booklist[j]=book;
  }

  if(info->groupbook>=ci->books)goto errout;
  for(j=0;j<acc;j++){
    if(info->booklist[j]>=ci->books)goto errout;
    if(ci->book_param[info->booklist[j]]->maptype==0)goto errout;
  }

  /* verify the phrasebook is not specifying an impossible or
     inconsistent partitioning scheme. */
  {
    int entries = ci->book_param[info->groupbook]->entries;
    int dim = ci->book_param[info->groupbook]->dim;
    int partvals = 1;
    while(dim>0){
      partvals *= info->partitions;
      if(partvals > entries) goto errout;
      dim--;
    }
  }

  return(info);
 errout:
  res0_free_info(info);
  return(NULL);
}

vorbis_look_residue *res0_look(vorbis_dsp_state *vd,
                               vorbis_info_residue *vr){
  vorbis_info_residue0 *info=(vorbis_info_residue0 *)vr;
  vorbis_look_residue0 *look=_ogg_calloc(1,sizeof(*look));
  codec_setup_info     *ci=vd->vi->codec_setup;

  int j,k,acc=0;
  int dim;
  int maxstage=0;
  look->info=info;

  look->parts=info->partitions;
  look->fullbooks=ci->fullbooks;
  look->phrasebook=ci->fullbooks+info->groupbook;
  dim=look->phrasebook->dim;

  look->partbooks=_ogg_calloc(look->parts,sizeof(*look->partbooks));

  for(j=0;j<look->parts;j++){
    int stages=ilog(info->secondstages[j]);
    if(stages){
      if(stages>maxstage)maxstage=stages;
      look->partbooks[j]=_ogg_calloc(stages,sizeof(*look->partbooks[j]));
      for(k=0;k<stages;k++)
        if(info->secondstages[j]&(1<<k)){
          look->partbooks[j][k]=ci->fullbooks+info->booklist[acc++];
#ifdef TRAIN_RES
          look->training_data[k][j]=_ogg_calloc(look->partbooks[j][k]->entries,
                                           sizeof(***look->training_data));
#endif
        }
    }
  }

  look->partvals=1;
  for(j=0;j<dim;j++)
      look->partvals*=look->parts;

  look->stages=maxstage;
  look->decodemap=_ogg_malloc(look->partvals*sizeof(*look->decodemap));
  for(j=0;j<look->partvals;j++){
    long val=j;
    long mult=look->partvals/look->parts;
    look->decodemap[j]=_ogg_malloc(dim*sizeof(*look->decodemap[j]));
    for(k=0;k<dim;k++){
      long deco=val/mult;
      val-=deco*mult;
      mult/=look->parts;
      look->decodemap[j][k]=deco;
    }
  }
#if defined(TRAIN_RES) || defined (TRAIN_RESAUX)
  {
    static int train_seq=0;
    look->train_seq=train_seq++;
  }
#endif
  return(look);
}

/* break an abstraction and copy some code for performance purposes */
static int local_book_besterror(codebook *book,float *a){
  int dim=book->dim,i,k,o;
  int best=0;
  encode_aux_threshmatch *tt=book->c->thresh_tree;

  /* find the quant val of each scalar */
  for(k=0,o=dim;k<dim;++k){
    float val=a[--o];
    i=tt->threshvals>>1;

    if(val<tt->quantthresh[i]){
      if(val<tt->quantthresh[i-1]){
        for(--i;i>0;--i)
          if(val>=tt->quantthresh[i-1])
            break;
      }
    }else{

      for(++i;i<tt->threshvals-1;++i)
        if(val<tt->quantthresh[i])break;

    }

    best=(best*tt->quantvals)+tt->quantmap[i];
  }
  /* regular lattices are easy :-) */

  if(book->c->lengthlist[best]<=0){
    const static_codebook *c=book->c;
    int i,j;
    float bestf=0.f;
    float *e=book->valuelist;
    best=-1;
    for(i=0;i<book->entries;i++){
      if(c->lengthlist[i]>0){
        float this=0.f;
        for(j=0;j<dim;j++){
          float val=(e[j]-a[j]);
          this+=val*val;
        }
        if(best==-1 || this<bestf){
          bestf=this;
          best=i;
        }
      }
      e+=dim;
    }
  }

  if(best>-1){
    float *ptr=book->valuelist+best*dim;
    for(i=0;i<dim;i++)
      *a++ -= *ptr++;
  }

  return(best);
}

static int _encodepart(oggpack_buffer *opb,float *vec, int n,
                       codebook *book,long *acc){
  int i,bits=0;
  int dim=book->dim;
  int step=n/dim;

  for(i=0;i<step;i++){
    int entry=local_book_besterror(book,vec+i*dim);

#ifdef TRAIN_RES
    if(entry>0)
      acc[entry]++;
#endif

    bits+=vorbis_book_encode(book,entry,opb);

  }

  return(bits);
}

static long **_01class(vorbis_block *vb,vorbis_look_residue *vl,
                       float **in,int ch){
  long i,j,k;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long **partword=_vorbis_block_alloc(vb,ch*sizeof(*partword));
  float scale=100./samples_per_partition;

  /* we find the partition type for each partition of each
     channel.  We'll go back and do the interleaved encoding in a
     bit.  For now, clarity */

  for(i=0;i<ch;i++){
    partword[i]=_vorbis_block_alloc(vb,n/samples_per_partition*sizeof(*partword[i]));
    memset(partword[i],0,n/samples_per_partition*sizeof(*partword[i]));
  }

  for(i=0;i<partvals;i++){
    int offset=i*samples_per_partition+info->begin;
    for(j=0;j<ch;j++){
      float max=0.;
      float ent=0.;
      for(k=0;k<samples_per_partition;k++){
        if(fabs(in[j][offset+k])>max)max=fabs(in[j][offset+k]);
        ent+=fabs(rint(in[j][offset+k]));
      }
      ent*=scale;

      for(k=0;k<possible_partitions-1;k++)
        if(max<=info->classmetric1[k] &&
           (info->classmetric2[k]<0 || (int)ent<info->classmetric2[k]))
          break;

      partword[j][i]=k;
    }
  }

#ifdef TRAIN_RESAUX
  {
    FILE *of;
    char buffer[80];

    for(i=0;i<ch;i++){
      sprintf(buffer,"resaux_%d.vqd",look->train_seq);
      of=fopen(buffer,"a");
      for(j=0;j<partvals;j++)
        fprintf(of,"%ld, ",partword[i][j]);
      fprintf(of,"\n");
      fclose(of);
    }
  }
#endif
  look->frames++;

  return(partword);
}

/* designed for stereo or other modes where the partition size is an
   integer multiple of the number of channels encoded in the current
   submap */
static long **_2class(vorbis_block *vb,vorbis_look_residue *vl,float **in,
                      int ch){
  long i,j,k,l;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long **partword=_vorbis_block_alloc(vb,sizeof(*partword));

#if defined(TRAIN_RES) || defined (TRAIN_RESAUX)
  FILE *of;
  char buffer[80];
#endif

  partword[0]=_vorbis_block_alloc(vb,n*ch/samples_per_partition*sizeof(*partword[0]));
  memset(partword[0],0,n*ch/samples_per_partition*sizeof(*partword[0]));

  for(i=0,l=info->begin/ch;i<partvals;i++){
    float magmax=0.f;
    float angmax=0.f;
    for(j=0;j<samples_per_partition;j+=ch){
      if(fabs(in[0][l])>magmax)magmax=fabs(in[0][l]);
      for(k=1;k<ch;k++)
        if(fabs(in[k][l])>angmax)angmax=fabs(in[k][l]);
        l++;
    }

    for(j=0;j<possible_partitions-1;j++)
      if(magmax<=info->classmetric1[j] &&
         angmax<=info->classmetric2[j])
        break;

    partword[0][i]=j;

  }

#ifdef TRAIN_RESAUX
  sprintf(buffer,"resaux_%d.vqd",look->train_seq);
  of=fopen(buffer,"a");
  for(i=0;i<partvals;i++)
    fprintf(of,"%ld, ",partword[0][i]);
  fprintf(of,"\n");
  fclose(of);
#endif

  look->frames++;

  return(partword);
}

static int _01forward(oggpack_buffer *opb,
                      vorbis_block *vb,vorbis_look_residue *vl,
                      float **in,int ch,
                      long **partword,
                      int (*encode)(oggpack_buffer *,float *,int,
                                    codebook *,long *)){
  long i,j,k,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int possible_partitions=info->partitions;
  int partitions_per_word=look->phrasebook->dim;
  int n=info->end-info->begin;

  int partvals=n/samples_per_partition;
  long resbits[128];
  long resvals[128];

#ifdef TRAIN_RES
  for(i=0;i<ch;i++)
    for(j=info->begin;j<end;j++){
      if(in[i][j]>look->tmax)look->tmax=in[i][j];
      if(in[i][j]<look->tmin)look->tmin=in[i][j];
    }
#endif

  memset(resbits,0,sizeof(resbits));
  memset(resvals,0,sizeof(resvals));

  /* we code the partition words for each channel, then the residual
     words for a partition per channel until we've written all the
     residual words for that partition word.  Then write the next
     partition channel words... */

  for(s=0;s<look->stages;s++){

    for(i=0;i<partvals;){

      /* first we encode a partition codeword for each channel */
      if(s==0){
        for(j=0;j<ch;j++){
          long val=partword[j][i];
          for(k=1;k<partitions_per_word;k++){
            val*=possible_partitions;
            if(i+k<partvals)
              val+=partword[j][i+k];
          }

          /* training hack */
          if(val<look->phrasebook->entries)
            look->phrasebits+=vorbis_book_encode(look->phrasebook,val,opb);
#if 0 /*def TRAIN_RES*/
          else
            fprintf(stderr,"!");
#endif

        }
      }

      /* now we encode interleaved residual values for the partitions */
      for(k=0;k<partitions_per_word && i<partvals;k++,i++){
        long offset=i*samples_per_partition+info->begin;

        for(j=0;j<ch;j++){
          if(s==0)resvals[partword[j][i]]+=samples_per_partition;
          if(info->secondstages[partword[j][i]]&(1<<s)){
            codebook *statebook=look->partbooks[partword[j][i]][s];
            if(statebook){
              int ret;
              long *accumulator=NULL;

#ifdef TRAIN_RES
              accumulator=look->training_data[s][partword[j][i]];
              {
                int l;
                float *samples=in[j]+offset;
                for(l=0;l<samples_per_partition;l++){
                  if(samples[l]<look->training_min[s][partword[j][i]])
                    look->training_min[s][partword[j][i]]=samples[l];
                  if(samples[l]>look->training_max[s][partword[j][i]])
                    look->training_max[s][partword[j][i]]=samples[l];
                }
              }
#endif

              ret=encode(opb,in[j]+offset,samples_per_partition,
                         statebook,accumulator);

              look->postbits+=ret;
              resbits[partword[j][i]]+=ret;
            }
          }
        }
      }
    }
  }

  /*{
    long total=0;
    long totalbits=0;
    fprintf(stderr,"%d :: ",vb->mode);
    for(k=0;k<possible_partitions;k++){
    fprintf(stderr,"%ld/%1.2g, ",resvals[k],(float)resbits[k]/resvals[k]);
    total+=resvals[k];
    totalbits+=resbits[k];
    }

    fprintf(stderr,":: %ld:%1.2g\n",total,(double)totalbits/total);
    }*/

  return(0);
}

/* a truncated packet here just means 'stop working'; it's not an error */
static int _01inverse(vorbis_block *vb,vorbis_look_residue *vl,
                      float **in,int ch,
                      long (*decodepart)(codebook *, float *,
                                         oggpack_buffer *,int)){

  long i,j,k,l,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int max=vb->pcmend>>1;
  int end=(info->end<max?info->end:max);
  int n=end-info->begin;

  if(n>0){
    int partvals=n/samples_per_partition;
    int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
    int ***partword=alloca(ch*sizeof(*partword));

    for(j=0;j<ch;j++)
      partword[j]=_vorbis_block_alloc(vb,partwords*sizeof(*partword[j]));

    for(s=0;s<look->stages;s++){

      /* each loop decodes on partition codeword containing
         partitions_per_word partitions */
      for(i=0,l=0;i<partvals;l++){
        if(s==0){
          /* fetch the partition word for each channel */
          for(j=0;j<ch;j++){
            int temp=vorbis_book_decode(look->phrasebook,&vb->opb);

            if(temp==-1)goto eopbreak;
            partword[j][l]=look->decodemap[temp];
            if(partword[j][l]==NULL)goto errout;
          }
        }

        /* now we decode residual values for the partitions */
        for(k=0;k<partitions_per_word && i<partvals;k++,i++)
          for(j=0;j<ch;j++){
            long offset=info->begin+i*samples_per_partition;
            if(info->secondstages[partword[j][l][k]]&(1<<s)){
              codebook *stagebook=look->partbooks[partword[j][l][k]][s];
              if(stagebook){
                if(decodepart(stagebook,in[j]+offset,&vb->opb,
                              samples_per_partition)==-1)goto eopbreak;
              }
            }
          }
      }
    }
  }
 errout:
 eopbreak:
  return(0);
}

#if 0
/* residue 0 and 1 are just slight variants of one another. 0 is
   interleaved, 1 is not */
long **res0_class(vorbis_block *vb,vorbis_look_residue *vl,
                  float **in,int *nonzero,int ch){
  /* we encode only the nonzero parts of a bundle */
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    /*return(_01class(vb,vl,in,used,_interleaved_testhack));*/
    return(_01class(vb,vl,in,used));
  else
    return(0);
}

int res0_forward(vorbis_block *vb,vorbis_look_residue *vl,
                 float **in,float **out,int *nonzero,int ch,
                 long **partword){
  /* we encode only the nonzero parts of a bundle */
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      if(out)
        for(j=0;j<n;j++)
          out[i][j]+=in[i][j];
      in[used++]=in[i];
    }
  if(used){
    int ret=_01forward(vb,vl,in,used,partword,
                      _interleaved_encodepart);
    if(out){
      used=0;
      for(i=0;i<ch;i++)
        if(nonzero[i]){
          for(j=0;j<n;j++)
            out[i][j]-=in[used][j];
          used++;
        }
    }
    return(ret);
  }else{
    return(0);
  }
}
#endif

int res0_inverse(vorbis_block *vb,vorbis_look_residue *vl,
                 float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01inverse(vb,vl,in,used,vorbis_book_decodevs_add));
  else
    return(0);
}

int res1_forward(oggpack_buffer *opb,vorbis_block *vb,vorbis_look_residue *vl,
                 float **in,float **out,int *nonzero,int ch,
                 long **partword){
  int i,j,used=0,n=vb->pcmend/2;
  for(i=0;i<ch;i++)
    if(nonzero[i]){
      if(out)
        for(j=0;j<n;j++)
          out[i][j]+=in[i][j];
      in[used++]=in[i];
    }

  if(used){
    int ret=_01forward(opb,vb,vl,in,used,partword,_encodepart);
    if(out){
      used=0;
      for(i=0;i<ch;i++)
        if(nonzero[i]){
          for(j=0;j<n;j++)
            out[i][j]-=in[used][j];
          used++;
        }
    }
    return(ret);
  }else{
    return(0);
  }
}

long **res1_class(vorbis_block *vb,vorbis_look_residue *vl,
                  float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01class(vb,vl,in,used));
  else
    return(0);
}

int res1_inverse(vorbis_block *vb,vorbis_look_residue *vl,
                 float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])
      in[used++]=in[i];
  if(used)
    return(_01inverse(vb,vl,in,used,vorbis_book_decodev_add));
  else
    return(0);
}

long **res2_class(vorbis_block *vb,vorbis_look_residue *vl,
                  float **in,int *nonzero,int ch){
  int i,used=0;
  for(i=0;i<ch;i++)
    if(nonzero[i])used++;
  if(used)
    return(_2class(vb,vl,in,ch));
  else
    return(0);
}

/* res2 is slightly more different; all the channels are interleaved
   into a single vector and encoded. */

int res2_forward(oggpack_buffer *opb,
                 vorbis_block *vb,vorbis_look_residue *vl,
                 float **in,float **out,int *nonzero,int ch,
                 long **partword){
  long i,j,k,n=vb->pcmend/2,used=0;

  /* don't duplicate the code; use a working vector hack for now and
     reshape ourselves into a single channel res1 */
  /* ugly; reallocs for each coupling pass :-( */
  float *work=_vorbis_block_alloc(vb,ch*n*sizeof(*work));
  for(i=0;i<ch;i++){
    float *pcm=in[i];
    if(nonzero[i])used++;
    for(j=0,k=i;j<n;j++,k+=ch)
      work[k]=pcm[j];
  }

  if(used){
    int ret=_01forward(opb,vb,vl,&work,1,partword,_encodepart);
    /* update the sofar vector */
    if(out){
      for(i=0;i<ch;i++){
        float *pcm=in[i];
        float *sofar=out[i];
        for(j=0,k=i;j<n;j++,k+=ch)
          sofar[j]+=pcm[j]-work[k];

      }
    }
    return(ret);
  }else{
    return(0);
  }
}

/* duplicate code here as speed is somewhat more important */
int res2_inverse(vorbis_block *vb,vorbis_look_residue *vl,
                 float **in,int *nonzero,int ch){
  long i,k,l,s;
  vorbis_look_residue0 *look=(vorbis_look_residue0 *)vl;
  vorbis_info_residue0 *info=look->info;

  /* move all this setup out later */
  int samples_per_partition=info->grouping;
  int partitions_per_word=look->phrasebook->dim;
  int max=(vb->pcmend*ch)>>1;
  int end=(info->end<max?info->end:max);
  int n=end-info->begin;

  if(n>0){
    int partvals=n/samples_per_partition;
    int partwords=(partvals+partitions_per_word-1)/partitions_per_word;
    int **partword=_vorbis_block_alloc(vb,partwords*sizeof(*partword));

    for(i=0;i<ch;i++)if(nonzero[i])break;
    if(i==ch)return(0); /* no nonzero vectors */

    for(s=0;s<look->stages;s++){
      for(i=0,l=0;i<partvals;l++){

        if(s==0){
          /* fetch the partition word */
          int temp=vorbis_book_decode(look->phrasebook,&vb->opb);
          if(temp==-1)goto eopbreak;
          partword[l]=look->decodemap[temp];
          if(partword[l]==NULL)goto errout;
        }

        /* now we decode residual values for the partitions */
        for(k=0;k<partitions_per_word && i<partvals;k++,i++)
          if(info->secondstages[partword[l][k]]&(1<<s)){
            codebook *stagebook=look->partbooks[partword[l][k]][s];

            if(stagebook){
              if(vorbis_book_decodevv_add(stagebook,in,
                                          i*samples_per_partition+info->begin,ch,
                                          &vb->opb,samples_per_partition)==-1)
                goto eopbreak;
            }
          }
      }
    }
  }
 errout:
 eopbreak:
  return(0);
}


const vorbis_func_residue residue0_exportbundle={
  NULL,
  &res0_unpack,
  &res0_look,
  &res0_free_info,
  &res0_free_look,
  NULL,
  NULL,
  &res0_inverse
};

const vorbis_func_residue residue1_exportbundle={
  &res0_pack,
  &res0_unpack,
  &res0_look,
  &res0_free_info,
  &res0_free_look,
  &res1_class,
  &res1_forward,
  &res1_inverse
};

const vorbis_func_residue residue2_exportbundle={
  &res0_pack,
  &res0_unpack,
  &res0_look,
  &res0_free_info,
  &res0_free_look,
  &res2_class,
  &res2_forward,
  &res2_inverse
};
