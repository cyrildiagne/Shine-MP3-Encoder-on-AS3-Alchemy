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


#include <math.h>

#include "types.h"
#include "layer3.h"
#include "l3mdct.h"


/* This is table B.9: coefficients for aliasing reduction */
static double c[8] = { -0.6,-0.535,-0.33,-0.185,-0.095,-0.041,-0.0142, -0.0037 };
static double ca[8];
static double cs[8];

static double win[36];
static double cos_l[18][36];


void mdct_initialise()
{
    int i,m,k; //N;
    double sq;

/* prepare the aliasing reduction butterflies */
    for (i=8; i--; )
    {
        sq = sqrt(1.0 + sq(c[i]));
        ca[i] = c[i] / sq;
        cs[i] = 1.0  / sq;
    }

    for(i=36; i--; )
		win[i] = sin( PI36 * (i + 0.5) );


    //N = 36;
    for (m = 0; m < 18; m++ )
      for (k = 0; k < 36; k++ )
        cos_l[m][k] = cos( (PI / (72)) * (2 * k +19) *
                     (2 * m + 1) ) / (9);

}


static void mdct( double *in, double *out )
/*-------------------------------------------------------------------*/
/*   Function: Calculation of the MDCT                               */
/*   In the case of long blocks ( block_type 0,1,3 ) there are       */
/*   36 coefficents in the time domain and 18 in the frequency       */
/*   domain.                                                         */
/*-------------------------------------------------------------------*/
{
    int k,m;

        for(m=18; m--; )
        {
            out[m]= win[35] * in[35] * cos_l[m][35];
            for(k=35; k--; )
                out[m] += win[k] * in[k] * cos_l[m][k];
        }
}


void mdct_sub(double sb_sample[2][3][18][SBLIMIT], 
                 double (*mdct_freq)[2][samp_per_frame2], 
                 side_info_t *side_info)
{

    double (*mdct_enc)[2][32][18] = (double (*)[2][32][18]) mdct_freq;

    int      ch,gr,band,k,j;
    gr_info *cod_info;
    double   mdct_in[36];
    double   bu,bd;
    
    for(gr=0; gr<2; gr++)
        for(ch=config.wave.channels; ch--; )
        {
	    cod_info = (gr_info*) &(side_info->gr[gr].ch[ch]) ;
	    
/* Compensate for inversion in the analysis filter */
	    for(band=32; band--; )
		for(k=18; k--; )
		    if((band&1) && (k&1))
				sb_sample[ch][gr+1][k][band] *= -1.0;
	    
/* Perform imdct of 18 previous subband samples + 18 current subband samples */
	   for(band=32; band--; )
	    {
		for(k=18; k--; )
		{
		    mdct_in[k]    = sb_sample[ch][ gr ][k][band];
		    mdct_in[k+18] = sb_sample[ch][gr+1][k][band];
		}
		
		mdct(mdct_in,&mdct_enc[gr][ch][band][0]);
	    }
	    
/* Perform aliasing reduction butterfly*/
		for(band=31; band--; )
		    for(k=8; k--; )
		    {
			bu = mdct_enc[gr][ch][band][17-k] * cs[k] + mdct_enc[gr][ch][band+1][k] * ca[k];
			bd = mdct_enc[gr][ch][band+1][k] * cs[k] - mdct_enc[gr][ch][band][17-k] * ca[k];
			mdct_enc[gr][ch][band][17-k] = bu;
			mdct_enc[gr][ch][band+1][k]  = bd;
		    }
	}
    
/* Save latest granule's subband samples to be used in the next mdct call */
    for(ch=config.wave.channels ;ch--; )
	for(j=18; j--; )
	    for(band=32; band--; )
			sb_sample[ch][0][j][band] = sb_sample[ch][2][j][band];
}

