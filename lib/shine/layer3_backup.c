//    Shine is an MP3 encoder
//    Copyright (C) 1999-2000  Gabriel Bouvigne
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.


#include <stdio.h>
#include <string.h>

#include "types.h"
#include "wave.h"
#include "error.h"

#include "layer3.h"
#include "l3subband.h"
#include "l3mdct.h"
#include "l3loop.h"
#include "l3bitstream.h"
#include "bitstream.h"

static void update_status(int frames_processed)
{
    printf("\015[Frame %6d of %6ld] (%2.2f%%)", 
            frames_processed,config.mpeg.total_frames,
            (double)((double)frames_processed/config.mpeg.total_frames)*100); 
    fflush(stdout);
}


void compress()
{
    int             frames_processed;
    static short    buffer[2][samp_per_frame];
    int             channel;

    int             i;
    int             gr;
    short           sam[2][1344];
    double           snr32[32];
    side_info_t  side_info;
    short          *buffer_window[2];
    double          win_que[2][HAN_SIZE];
    double          sb_sample[2][3][18][SBLIMIT];
    double          mdct_freq[2][2][samp_per_frame2];
    int             enc[2][2][samp_per_frame2];
    scalefac_t   scalefactor;
    bitstream_t     bs;
 
    double          avg_slots_per_frame;
    double          frac_slots_per_frame;
    long            whole_slots_per_frame;
    double          slot_lag;
    
    int             mean_bits;
    unsigned long   sent_bits  = 0;
    unsigned long   frame_bits = 0;
    int             sideinfo_len;
	
	bs.pt = config.wave.output;
    open_bit_stream_w(&bs, config.outfile, BUFFER_SIZE);
    
    memset((char*)snr32,0,sizeof(snr32));
    memset((char *)sam,0,sizeof(sam));
    memset((char *)&side_info,0,sizeof(side_info_t));

    subband_initialise();
    mdct_initialise();

    config.mpeg.samples_per_frame = samp_per_frame;
    config.mpeg.total_frames      = config.wave.total_samples/config.mpeg.samples_per_frame;
    config.mpeg.bits_per_slot     = 8;
    frames_processed              = 0;

    sideinfo_len = 32;
 
	if(config.wave.channels==1)
			sideinfo_len += 136;
        else
			sideinfo_len += 256;

/* Figure average number of 'slots' per frame. */
    avg_slots_per_frame   = ((double)config.mpeg.samples_per_frame / 
                             ((double)config.wave.samplerate/1000)) *
                            ((double)config.mpeg.bitr /
                             (double)config.mpeg.bits_per_slot);
    whole_slots_per_frame = (int)avg_slots_per_frame;
    frac_slots_per_frame  = avg_slots_per_frame - (double)whole_slots_per_frame;
    slot_lag              = -frac_slots_per_frame;
    if(frac_slots_per_frame==0)
		config.mpeg.padding = 0;

    while(wave_get(buffer))
    {
		if ((frames_processed++)%10==0) {
			update_status(frames_processed);
			//flyield();
		}
		
        buffer_window[0] = buffer[0];
        buffer_window[1] = buffer[1];

        if(frac_slots_per_frame) {
            if(slot_lag>(frac_slots_per_frame-1.0))
            { /* No padding for this frame */
                slot_lag    -= frac_slots_per_frame;
                config.mpeg.padding = 0;
            }
            else 
            { /* Padding for this frame  */
                slot_lag    += (1-frac_slots_per_frame);
                config.mpeg.padding = 1;
            }
		}
       config.mpeg.bits_per_frame = 8*(whole_slots_per_frame + config.mpeg.padding);
       mean_bits = (config.mpeg.bits_per_frame - sideinfo_len)>>1;


/* polyphase filtering */
       for(gr=0;gr<2;gr++)
            for(channel=config.wave.channels; channel--; )
                for(i=0;i<18;i++)
                {
                    window_subband(&buffer_window[channel],
                                      &win_que[channel][0],
                                      channel);
                    filter_subband(&win_que[channel][0],
                                      &sb_sample[channel][gr+1][i][0]);
                }

/* apply mdct to the polyphase output */
        mdct_sub(sb_sample, mdct_freq, &side_info);


/* bit and noise allocation */
        iteration_loop(mdct_freq,&side_info,
                          enc, mean_bits,&scalefactor);


/* write the frame to the bitstream */

       format_bitstream(enc,&side_info,&scalefactor, 
                           &bs,mdct_freq);

       frame_bits = sstell(&bs) - sent_bits;

       sent_bits += frame_bits;
    }    


}


