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

 function: random psychoacoustics (not including preecho)
 last mod: $Id: psy.h 16227 2009-07-08 06:58:46Z xiphmont $

 ********************************************************************/

#ifndef _V_PSY_H_
#define _V_PSY_H_
#include "smallft.h"

#include "backends.h"
#include "envelope.h"

#ifndef EHMER_MAX
#define EHMER_MAX 56
#endif

/* psychoacoustic setup ********************************************/
#define P_BANDS 17      /* 62Hz to 16kHz */
#define P_LEVELS 8      /* 30dB to 100dB */
#define P_LEVEL_0 30.    /* 30 dB */
#define P_NOISECURVES 3

#define NOISE_COMPAND_LEVELS 40
typedef struct vorbis_info_psy{
  int   blockflag;

  float ath_adjatt;
  float ath_maxatt;

  float tone_masteratt[P_NOISECURVES];
  float tone_centerboost;
  float tone_decay;
  float tone_abs_limit;
  float toneatt[P_BANDS];

  int noisemaskp;
  float noisemaxsupp;
  float noisewindowlo;
  float noisewindowhi;
  int   noisewindowlomin;
  int   noisewindowhimin;
  int   noisewindowfixed;
  float noiseoff[P_NOISECURVES][P_BANDS];
  float noisecompand[NOISE_COMPAND_LEVELS];

  float max_curve_dB;

  int normal_channel_p;
  int normal_point_p;
  int normal_start;
  int normal_partition;
  double normal_thresh;
} vorbis_info_psy;

typedef struct{
  int   eighth_octave_lines;

  /* for block long/short tuning; encode only */
  float preecho_thresh[VE_BANDS];
  float postecho_thresh[VE_BANDS];
  float stretch_penalty;
  float preecho_minenergy;

  float ampmax_att_per_sec;

  /* channel coupling config */
  int   coupling_pkHz[PACKETBLOBS];
  int   coupling_pointlimit[2][PACKETBLOBS];
  int   coupling_prepointamp[PACKETBLOBS];
  int   coupling_postpointamp[PACKETBLOBS];
  int   sliding_lowpass[2][PACKETBLOBS];

} vorbis_info_psy_global;

typedef struct {
  float ampmax;
  int   channels;

  vorbis_info_psy_global *gi;
  int   coupling_pointlimit[2][P_NOISECURVES];
} vorbis_look_psy_global;


typedef struct {
  int n;
  struct vorbis_info_psy *vi;

  float ***tonecurves;
  float **noiseoffset;

  float *ath;
  long  *octave;             /* in n.ocshift format */
  long  *bark;

  long  firstoc;
  long  shiftoc;
  int   eighth_octave_lines; /* power of two, please */
  int   total_octave_lines;
  long  rate; /* cache it */

  float m_val; /* Masking compensation value */

} vorbis_look_psy;

extern void   _vp_psy_init(vorbis_look_psy *p,vorbis_info_psy *vi,
                           vorbis_info_psy_global *gi,int n,long rate);
extern void   _vp_psy_clear(vorbis_look_psy *p);
extern void  *_vi_psy_dup(void *source);

extern void   _vi_psy_free(vorbis_info_psy *i);
extern vorbis_info_psy *_vi_psy_copy(vorbis_info_psy *i);

extern void _vp_remove_floor(vorbis_look_psy *p,
                             float *mdct,
                             int *icodedflr,
                             float *residue,
                             int sliding_lowpass);

extern void _vp_noisemask(vorbis_look_psy *p,
                          float *logmdct,
                          float *logmask);

extern void _vp_tonemask(vorbis_look_psy *p,
                         float *logfft,
                         float *logmask,
                         float global_specmax,
                         float local_specmax);

extern void _vp_offset_and_mix(vorbis_look_psy *p,
                               float *noise,
                               float *tone,
                               int offset_select,
                               float *logmask,
                               float *mdct,
                               float *logmdct);

extern float _vp_ampmax_decay(float amp,vorbis_dsp_state *vd);

extern float **_vp_quantize_couple_memo(vorbis_block *vb,
                                        vorbis_info_psy_global *g,
                                        vorbis_look_psy *p,
                                        vorbis_info_mapping0 *vi,
                                        float **mdct);

extern void _vp_couple(int blobno,
                       vorbis_info_psy_global *g,
                       vorbis_look_psy *p,
                       vorbis_info_mapping0 *vi,
                       float **res,
                       float **mag_memo,
                       int   **mag_sort,
                       int   **ifloor,
                       int   *nonzero,
                       int   sliding_lowpass);

extern void _vp_noise_normalize(vorbis_look_psy *p,
                                float *in,float *out,int *sortedindex);

extern void _vp_noise_normalize_sort(vorbis_look_psy *p,
                                     float *magnitudes,int *sortedindex);

extern int **_vp_quantize_couple_sort(vorbis_block *vb,
                                      vorbis_look_psy *p,
                                      vorbis_info_mapping0 *vi,
                                      float **mags);

extern void hf_reduction(vorbis_info_psy_global *g,
                         vorbis_look_psy *p,
                         vorbis_info_mapping0 *vi,
                         float **mdct);


#endif
