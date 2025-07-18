/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2007             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: utility functions for vorbis codec test suite.
 last mod: $Id: util.c 13293 2007-07-24 00:09:47Z erikd $

 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

#include "write_read.h"

/* The following function is basically a hacked version of the code in
 * examples/encoder_example.c */
void
write_vorbis_data_or_die (const char *filename, int srate, const float * data, int count)
{
  FILE * file ;
  ogg_stream_state os;
  ogg_page         og;
  ogg_packet       op;
  vorbis_info      vi;
  vorbis_comment   vc;
  vorbis_dsp_state vd;
  vorbis_block     vb;

  int eos = 0, ret;

  if ((file = fopen (filename, "wb")) == NULL) {
    printf("\n\nError : fopen failed : %s\n", strerror (errno)) ;
    exit (1) ;
  }

  /********** Encode setup ************/

  vorbis_info_init (&vi);

  ret = vorbis_encode_init_vbr (&vi,1,srate,0.8);
  if (ret) {
    printf ("vorbis_encode_init_vbr return %d\n", ret) ;
    exit (1) ;
  }

  vorbis_comment_init (&vc);
  vorbis_comment_add_tag (&vc,"ENCODER","test/util.c");
  vorbis_analysis_init (&vd,&vi);
  vorbis_block_init (&vd,&vb);

  ogg_stream_init (&os,12345678);

  {
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout (&vd,&vc,&header,&header_comm,&header_code);
    ogg_stream_packetin (&os,&header);
    ogg_stream_packetin (&os,&header_comm);
    ogg_stream_packetin (&os,&header_code);

    /* Ensures the audio data will start on a new page. */
    while (!eos){
        int result = ogg_stream_flush (&os,&og);
        if (result == 0)
            break;
        fwrite (og.header,1,og.header_len,file);
        fwrite (og.body,1,og.body_len,file);
    }

  }

  {
    /* expose the buffer to submit data */
    float **buffer = vorbis_analysis_buffer (&vd,count);

    memcpy (buffer [0], data, count * sizeof (float)) ;

    /* tell the library how much we actually submitted */
    vorbis_analysis_wrote (&vd,count);
    vorbis_analysis_wrote (&vd,0);
  }

  while (vorbis_analysis_blockout (&vd,&vb) == 1) {
    vorbis_analysis (&vb,NULL);
    vorbis_bitrate_addblock (&vb);

    while (vorbis_bitrate_flushpacket (&vd,&op)) {
      ogg_stream_packetin (&os,&op);

      while (!eos) {
          int result = ogg_stream_pageout (&os,&og);
          if (result == 0)
              break;
          fwrite (og.header,1,og.header_len,file);
          fwrite (og.body,1,og.body_len,file);

          if (ogg_page_eos (&og))
              eos = 1;
      }
    }
  }

  ogg_stream_clear (&os);
  vorbis_block_clear (&vb);
  vorbis_dsp_clear (&vd);
  vorbis_comment_clear (&vc);
  vorbis_info_clear (&vi);

 fclose (file) ;
}

/* The following function is basically a hacked version of the code in
 * examples/decoder_example.c */
void
read_vorbis_data_or_die (const char *filename, int srate, float * data, int count)
{
  ogg_sync_state   oy;
  ogg_stream_state os;
  ogg_page         og;
  ogg_packet       op;

  vorbis_info      vi;
  vorbis_comment   vc;
  vorbis_dsp_state vd;
  vorbis_block     vb;

  FILE *file;
  char *buffer;
  int  bytes;
  int eos = 0;
  int i;
  int read_total = 0 ;

  if ((file = fopen (filename, "rb")) == NULL) {
    printf("\n\nError : fopen failed : %s\n", strerror (errno)) ;
    exit (1) ;
  }

  ogg_sync_init (&oy);

  {
    buffer = ogg_sync_buffer (&oy,4096);
    bytes = fread (buffer,1,4096,file);
    ogg_sync_wrote (&oy,bytes);

    if(ogg_sync_pageout (&oy,&og) != 1) {
      if(bytes < 4096) {
        printf ("Out of data.\n") ;
          goto done_decode ;
      }

      fprintf (stderr,"Input does not appear to be an Ogg bitstream.\n");
      exit (1);
    }

    ogg_stream_init (&os,ogg_page_serialno(&og));

    vorbis_info_init (&vi);
    vorbis_comment_init (&vc);
    if (ogg_stream_pagein (&os,&og) < 0) {
      fprintf (stderr,"Error reading first page of Ogg bitstream data.\n");
      exit (1);
    }

    if (ogg_stream_packetout(&os,&op) != 1) {
      fprintf (stderr,"Error reading initial header packet.\n");
      exit (1);
    }

    if (vorbis_synthesis_headerin (&vi,&vc,&op) < 0) {
      fprintf (stderr,"This Ogg bitstream does not contain Vorbis "
          "audio data.\n");
      exit (1);
    }

    i = 0;
    while ( i < 2) {
      while (i < 2) {
        int result = ogg_sync_pageout (&oy,&og);
        if(result == 0)
          break;
        if(result==1) {
          ogg_stream_pagein(&os,&og);

        while (i < 2) {
          result = ogg_stream_packetout (&os,&op);
          if (result == 0)
            goto done_decode;
          if (result < 0) {
            fprintf (stderr,"Corrupt secondary header.  Exiting.\n");
            exit(1);
          }
        vorbis_synthesis_headerin (&vi,&vc,&op);
        i++;
        }
      }
    }

    buffer = ogg_sync_buffer (&oy,4096);
    bytes = fread (buffer,1,4096,file);
    if (bytes == 0 && i < 2) {
      fprintf (stderr,"End of file before finding all Vorbis headers!\n");
      exit (1);
    }

    ogg_sync_wrote (&oy,bytes);
  }

  if (vi.rate != srate) {
    printf ("\n\nError : File '%s' has sample rate of %ld when it should be %d.\n\n", filename, vi.rate, srate);
    exit (1) ;
  }
  if (vi.channels != 1) {
    printf ("\n\nError : File '%s' has %d channels, but should be mono.\n\n", filename, vi.channels);
    exit (1) ;
  }

  vorbis_synthesis_init (&vd,&vi);
  vorbis_block_init (&vd,&vb);

  while(!eos) {
    while (!eos) {
      int result = ogg_sync_pageout (&oy,&og);
      if (result == 0)
        break;
      if (result < 0) {
        fprintf (stderr,"Corrupt or missing data in bitstream; "
            "continuing...\n");
      } else {
        ogg_stream_pagein (&os,&og);
        while (1) {
          result = ogg_stream_packetout (&os,&op);

          if (result == 0)
            break;
          if (result < 0) {
            /* no reason to complain; already complained above */
          } else {
              float **pcm;
            int samples;

            if (vorbis_synthesis (&vb,&op) == 0)
              vorbis_synthesis_blockin(&vd,&vb);
            while ((samples = vorbis_synthesis_pcmout (&vd,&pcm)) > 0 && read_total < count) {
              int bout = samples < count ? samples : count;
              bout = read_total + bout > count ? count - read_total : bout;

              memcpy (data + read_total, pcm[0], bout * sizeof (float)) ;

              vorbis_synthesis_read (&vd,bout);
              read_total += bout ;
            }
          }
        }

        if (ogg_page_eos (&og)) eos = 1;
      }
    }

      if (!eos) {
        buffer = ogg_sync_buffer (&oy,4096);
        bytes = fread (buffer,1,4096,file);
        ogg_sync_wrote (&oy,bytes);
        if (bytes == 0) eos = 1;
      }
    }

    ogg_stream_clear (&os);

    vorbis_block_clear (&vb);
    vorbis_dsp_clear (&vd);
    vorbis_comment_clear (&vc);
    vorbis_info_clear (&vi);

  }
done_decode:

  /* OK, clean up the framer */
  ogg_sync_clear (&oy);

  fclose (file) ;
}

