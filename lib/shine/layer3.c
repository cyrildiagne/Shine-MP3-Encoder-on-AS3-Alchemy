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

/*
static void update_status(int frames_processed)
{
    printf("\015[Frame %6d of %6ld] (%2.2f%%)", 
            frames_processed,config.mpeg.total_frames,
            (double)((double)frames_processed/config.mpeg.total_frames)*100); 
    fflush(stdout);
}
 */

void start_compress()
{
	
	l3_bs.pt = config.wave.output;
    open_bit_stream_ba(&l3_bs, BUFFER_SIZE);
    
    memset((char*)l3.snr32,0,sizeof(l3.snr32));
    memset((char *)l3.sam,0,sizeof(l3.sam));
    memset((char *)&l3.side_info,0,sizeof(l3.side_info));

    subband_initialise();
    mdct_initialise();

    config.mpeg.samples_per_frame = samp_per_frame;
    config.mpeg.total_frames      = config.wave.total_samples/config.mpeg.samples_per_frame;
    config.mpeg.bits_per_slot     = 8;
    l3.frames_processed           = 0;

    l3.sideinfo_len = 32;
 
	if(config.wave.channels==1)
			l3.sideinfo_len += 136;
        else
			l3.sideinfo_len += 256;

/* Figure average number of 'slots' per frame. */
    l3.avg_slots_per_frame   = ((double)config.mpeg.samples_per_frame / 
                               ((double)config.wave.samplerate/1000)) *
                               ((double)config.mpeg.bitr /
                               (double)config.mpeg.bits_per_slot);
    l3.whole_slots_per_frame = (int)l3.avg_slots_per_frame;
    l3.frac_slots_per_frame  = l3.avg_slots_per_frame - (double)l3.whole_slots_per_frame;
    l3.slot_lag              = -l3.frac_slots_per_frame;
    if(l3.frac_slots_per_frame==0)
		config.mpeg.padding = 0;


}

int update_compress() {
	
	int count = 0;
	
    static short    buffer[2][samp_per_frame];
    int             channel;
	
    int             i;
    int             gr;
    short          *buffer_window[2];
    double          win_que[2][HAN_SIZE];
    double          sb_sample[2][3][18][SBLIMIT];
    double          mdct_freq[2][2][samp_per_frame2];
    int             enc[2][2][samp_per_frame2];
    scalefac_t		scalefactor;
    
    int             mean_bits;
    unsigned long   sent_bits  = 0;
    unsigned long   frame_bits = 0;
	
	while(wave_get(buffer)) {
	
		buffer_window[0] = buffer[0];
		buffer_window[1] = buffer[1];

		if(l3.frac_slots_per_frame) {
			if(l3.slot_lag>(l3.frac_slots_per_frame-1.0))
			{ /* No padding for this frame */
				l3.slot_lag    -= l3.frac_slots_per_frame;
				config.mpeg.padding = 0;
			}
			else 
			{ /* Padding for this frame  */
				l3.slot_lag    += (1-l3.frac_slots_per_frame);
				config.mpeg.padding = 1;
			}
		}
		config.mpeg.bits_per_frame = 8*(l3.whole_slots_per_frame + config.mpeg.padding);
		mean_bits = (config.mpeg.bits_per_frame - l3.sideinfo_len)>>1;


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
		mdct_sub(sb_sample, mdct_freq, &l3.side_info);

		
		/* bit and noise allocation */
		iteration_loop(mdct_freq,&l3.side_info,
					   enc, mean_bits,&scalefactor);


		/* write the frame to the bitstream */

		format_bitstream(enc,&l3.side_info,&scalefactor, 
						 &l3_bs,mdct_freq);

		frame_bits = sstell(&l3_bs) - sent_bits;

		sent_bits += frame_bits;
		
		count ++;
		l3.frames_processed++;
		
		if (count==30) {
			return ((double)l3.frames_processed/config.mpeg.total_frames)*100;
		}
	}
	
	flush_bitstream();
	
	return 100;
	//update_status(frames_processed);
	
}


