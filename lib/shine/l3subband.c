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
#include "tables.h" 
#include "l3subband.h"


static off[2]    = {0,0};
static double x[2][HAN_SIZE];
static double filter[SBLIMIT][64];

void subband_initialise()
{
    int i,j,k;

    for(i=2; i-- ; ) 
        for(j=HAN_SIZE; j--; )
			x[i][j] = 0;
        
/* create_ana_filter */
/************************************************************************
* PURPOSE:  Calculates the analysis filter bank coefficients
* SEMANTICS:
* Calculates the analysis filterbank coefficients and rounds to the
* 9th decimal place accuracy of the filterbank tables in the ISO
* document.  The coefficients are stored in #filter#
************************************************************************/
   for (i=32; i--; )
      for (k=64; k--; ) 
      {
          if ((filter[i][k] = 1e9*cos((double)((2*i+1)*(16-k)*PI64))) >= 0)
               modf(filter[i][k]+0.5, &filter[i][k]);
          else
			  modf(filter[i][k]-0.5, &filter[i][k]);
          filter[i][k] *= 1e-9;
      } 
}


void window_subband(short **buffer, double z[HAN_SIZE], int k)
/************************************************************************
* PURPOSE:  Overlapping window on PCM samples
* SEMANTICS:
* 32 16-bit pcm samples are scaled to fractional 2's complement and
* concatenated to the end of the window buffer #x#. The updated window
* buffer #x# is then windowed by the analysis window #enwindow# to produce the
* windowed sample #z#
************************************************************************/
{
    int i;

    /* replace 32 oldest samples with 32 new samples */
    for (i=0;i<32;i++)
		x[k][31-i+off[k]] = (double)*(*buffer)++/SCALE;

    /* shift samples into proper window positions */
    for (i=HAN_SIZE; i--; )
		z[i] = x[k][(i+off[k])&(HAN_SIZE-1)] * enwindow[i];

    off[k] += 480;              /* offset is modulo (HAN_SIZE)*/
    off[k] &= HAN_SIZE-1;
}
 

void filter_subband(double z[HAN_SIZE], double s[SBLIMIT])
 /************************************************************************
* PURPOSE:  Calculates the analysis filter bank coefficients
* SEMANTICS:
*      The windowed samples #z# is filtered by the digital filter matrix #filter#
* to produce the subband samples #s#. This done by first selectively
* picking out values from the windowed samples, and then multiplying
* them by the filter matrix, producing 32 subband samples.
*
************************************************************************/
{
   double y[64];
   int i,j;

   for (i=64; i--; ) 
       for (j=8, y[i] = 0; j--; )
		   y[i] += z[i+(j<<6)];

   for (i=SBLIMIT; i--; )
       for (j=64, s[i]= 0; j--; )
		   s[i] += filter[i][j] * y[j];
}

