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
#include "error.h"
#include "tables.h"
#include "layer3.h"
#include "l3loop.h"
#include "huffman.h"
#include "l3bitstream.h"
#include "reservoir.h"


int bin_search_StepSize(int desired_rate, double start, int ix[samp_per_frame2],
           double xrs[samp_per_frame2], gr_info * cod_info);

int count_bits();

#define PRECALC_SIZE 1152
#include "sqrttab.h"





/* This is the scfsi_band table from 2.4.2.7 of the IS */
int scfsi_band_long[5] = { 0, 6, 11, 16, 21 };

struct
{
    unsigned region0_count;
    unsigned region1_count;
} subdv_table[ 23 ] =
{
{0, 0}, /* 0 bands */
{0, 0}, /* 1 bands */
{0, 0}, /* 2 bands */
{0, 0}, /* 3 bands */
{0, 0}, /* 4 bands */
{0, 1}, /* 5 bands */
{1, 1}, /* 6 bands */
{1, 1}, /* 7 bands */
{1, 2}, /* 8 bands */
{2, 2}, /* 9 bands */
{2, 3}, /* 10 bands */
{2, 3}, /* 11 bands */
{3, 4}, /* 12 bands */
{3, 4}, /* 13 bands */
{3, 4}, /* 14 bands */
{4, 5}, /* 15 bands */
{4, 5}, /* 16 bands */
{4, 6}, /* 17 bands */
{5, 6}, /* 18 bands */
{5, 6}, /* 19 bands */
{5, 7}, /* 20 bands */
{6, 7}, /* 21 bands */
{6, 7}, /* 22 bands */
};


int *scalefac_band_long  = &sfBandIndex[3].l[0];







int quantanf_init(double xr[samp_per_frame2]);
int part2_length(int gr, int ch, side_info_t *si);

int bin_search_StepSize(int desired_rate, double start, int *ix, double xrs[samp_per_frame2], gr_info * cod_info);
int count_bits(int *ix /*int[samp_per_frame2]*/, gr_info *cod_info);
int count_bit(int ix[samp_per_frame2], unsigned int start, unsigned int end, unsigned int table );
int bigv_bitcount(int ix[samp_per_frame2], gr_info *gi);
int new_choose_table( int ix[samp_per_frame2], unsigned int begin, unsigned int end );
void bigv_tab_select( int ix[samp_per_frame2], gr_info *cod_info );
void subdivide(gr_info *cod_info);
int count1_bitcount( int ix[ samp_per_frame2 ], gr_info *cod_info );
void calc_runlen( int ix[samp_per_frame2], gr_info *cod_info );


void quantize( double xr[samp_per_frame2], int ix[samp_per_frame2], gr_info *cod_info );
int ix_max( int ix[samp_per_frame2], unsigned int begin, unsigned int end );


static int inner_loop(double xr[2][2][samp_per_frame2],  int enc[2][2][samp_per_frame2], 
                      int max_bits, gr_info *cod_info, int gr, int ch )
/***************************************************************************/ 
/* The code selects the best quantizerStepSize for a particular set        */
/* of scalefacs                                                            */
/***************************************************************************/ 
{
    int bits, c1bits, bvbits;
    double *xrs;  /*  double[samp_per_frame2] *xr; */
    int     *ix;  /*  int[samp_per_frame2]    *ix; */

    xrs = &xr[gr][ch][0];
    ix  = enc[gr][ch];

    if(max_bits<0)
    cod_info->quantizerStepSize--;
    do
    {
        do
        {
            cod_info->quantizerStepSize += 1.0;
            quantize(xrs,ix,cod_info);
        }
        while(ix_max(ix,0,samp_per_frame2)>(8205)); //was 8191+14              /* within table range? */

        calc_runlen(ix,cod_info);                        /* rzero,count1,big_values*/
        bits = c1bits = count1_bitcount(ix,cod_info);    /* count1_table selection*/
        subdivide(cod_info);                             /* bigvalues sfb division */
        bigv_tab_select(ix,cod_info);                    /* codebook selection*/
        bits += bvbits = bigv_bitcount( ix, cod_info );  /* bit count */

    }
    while(bits>max_bits);
    return bits;
}



static int outer_loop( double xr[2][2][samp_per_frame2],     /*  magnitudes of the spectral values */
                       int max_bits,
                       int enc[2][2][samp_per_frame2],    /* vector of quantized values ix(0..575) */
                       int gr, int ch, side_info_t *side_info )
/************************************************************************/
/*  Function: The outer iteration loop controls the masking conditions  */
/*  of all scalefactorbands. It computes the best scalefac and          */
/*  global gain. This module calls the inner iteration loop             */
/************************************************************************/
{
    int bits, huff_bits;

    double  *xrs; 
    int     *ix;  
    gr_info *cod_info = &side_info->gr[gr].ch[ch].tt;

    xrs = (double *) &(xr[gr][ch][0]); 
    ix  = (int *) &(enc[gr][ch][0]);

           bin_search_StepSize(max_bits,cod_info->quantizerStepSize,
                               ix,xrs,cod_info); // speeds things up a bit 

		cod_info->part2_length = part2_length(gr,ch,side_info);
        huff_bits = max_bits - cod_info->part2_length;

        bits = inner_loop( xr, enc, huff_bits, cod_info, gr, ch );


    cod_info->part2_length   = part2_length(gr,ch,side_info);
    cod_info->part2_3_length = cod_info->part2_length + bits;

    return cod_info->part2_3_length;
}




void iteration_loop(double          mdct_freq_org[2][2][samp_per_frame2], 
       		       side_info_t *side_info, 
                       int enc[2][2][samp_per_frame2],
		       int             mean_bits, 
		       scalefac_t  *scalefactor) 
/************************************************************************/
/*  iteration_loop()                                                    */
/************************************************************************/
{
    gr_info *cod_info;
    int *main_data_begin;

    int max_bits;
    int ch, gr, i;
    static int firstcall = 1;

    double xr[2][2][samp_per_frame2];

    main_data_begin = &side_info->main_data_begin;

    if ( firstcall )
    {
	*main_data_begin = 0;
        firstcall=0;
    }

    scalefac_band_long  = &sfBandIndex[config.mpeg.samplerate_index + (config.mpeg.type * 3)].l[0];


    for ( gr = 2; gr--; )
    {
        for ( ch= config.wave.channels; ch--; )
        {
            for ( i = samp_per_frame2; i--;  ) 
                xr[gr][ch][i] = mdct_freq_org[gr][ch][i];
        }
    }

    for(gr=2; gr--; )
    {
        for(ch=config.wave.channels; ch--; )
        {
            cod_info = (gr_info *) &(side_info->gr[gr].ch[ch]);	    
	    
/* calculation of number of available bit( per granule ) */
	    max_bits = mean_bits / config.wave.channels;
	    
/* reset of iteration variables */
		memset(scalefactor->l[gr][ch],0,22);
		memset(scalefactor->s[gr][ch],0,14);


	    for ( i=4; i--; )
			cod_info->slen[i] = 0;
            cod_info->part2_3_length    = 0;
            cod_info->big_values        = 0;
            cod_info->count1            = 0;
            cod_info->scalefac_compress = 0;
            cod_info->table_select[0]   = 0;
            cod_info->table_select[1]   = 0;
            cod_info->table_select[2]   = 0;
            cod_info->region0_count     = 0;
            cod_info->region1_count     = 0;
            cod_info->part2_length      = 0;
            cod_info->preflag           = 0;
            cod_info->scalefac_scale    = 0;
            cod_info->quantizerStepSize = 0.0;
            cod_info->count1table_select= 0;
            
            cod_info->quantizerStepSize = (double) quantanf_init(xr[gr][ch]);
            cod_info->part2_3_length    = outer_loop(xr, max_bits,
                                                     enc,
                                                     gr, ch,side_info );
	    ResvAdjust(cod_info, mean_bits );

	    cod_info->global_gain = cod_info->quantizerStepSize+210;

        } /* for ch */
    } /* for gr */

    ResvFrameEnd(side_info);
}



/************************************************************************/
/*  quantanf_init                                                       */
/************************************************************************/
int quantanf_init(double xr[samp_per_frame2])
/* Function: Calculate the first quantization step quantanf.       */
{
    int i, tp;
    double system_const=8;
    double sfm=0, sum1=0, sum2=0;
    

    for ( i=samp_per_frame2; i--; )
        if ( xr[i] )
	{
            double tpd = xr[i]*xr[i];
            sum1 += log(tpd);
            sum2 += tpd;
        }

    if ( sum2 )
    {
        sfm = exp(sum1/samp_per_frame2)/(sum2/samp_per_frame2);
        tp  = (int)(system_const*log(sfm));
	if (tp<-100)
		tp = -100;
    }

    return(tp-70); /* SS 19-12-96. Starting value of
                        global_gain or quantizerStepSize 
                        has to be reduced for iteration_loop
                     */
}








int part2_length(int gr, int ch, 
                 side_info_t *si)
/***************************************************************************/ 
/* calculates the number of bits needed to encode the scalefacs in the     */
/* main data block                                                         */
/***************************************************************************/ 
{
    int slen1, slen2, bits;
    gr_info *gi = &si->gr[gr].ch[ch].tt;

    bits = 0;

    {
	static int slen1_tab[16] = { 0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 };
	static int slen2_tab[16] = { 0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3 };

	slen1 = slen1_tab[ gi->scalefac_compress ];
	slen2 = slen2_tab[ gi->scalefac_compress ];

	
	    if ( !gr || !(si->scfsi[ch][0]) )
			bits += (6 * slen1);

	    if ( !gr || !(si->scfsi[ch][1]) )
			bits += (5 * slen1);

	    if ( !gr || !(si->scfsi[ch][2]) )
			bits += (5 * slen2);

	    if ( !gr || !(si->scfsi[ch][3]) )
			bits += (5 * slen2);
	
    }

    return bits;
}



void quantize( double xr[samp_per_frame2], int ix[samp_per_frame2], gr_info *cod_info )
/*************************************************************************/
/*   Function: Quantization of the vector xr ( -> ix)                    */
/*************************************************************************/
{
    register int i;
    int          idx;
    double       step;
    double dbl;
    long ln;

	step = pow(2.0, (cod_info->quantizerStepSize)/4 );

    for(i=samp_per_frame2; i--; )
    {
        dbl = fabs(xr[i])/step;

        ln = (long)dbl;
        if(ln<10000)
        {
            idx=int2idx[ln];
            if(dbl<idx2dbl[idx+1])
				ix[i] = idx;
            else
				ix[i] = idx+1;
        }
    }    
 }




int ix_max( int ix[samp_per_frame2], unsigned int begin, unsigned int end )
/*************************************************************************/
/*  Function: Calculate the maximum of ix from 0 to 575                  */
/*************************************************************************/
{
    register int i;
    register int x;
    register int max = 0;

    for(i=begin;i<end;i++)
    { 
        x = abs(ix[i]);
        if(x>max)
			max=x;
    }
    return max;
}



void calc_runlen( int ix[samp_per_frame2], gr_info *cod_info )
/*************************************************************************/
/* Function: Calculation of rzero, count1, big_values                    */
/* (Partitions ix into big values, quadruples and zeros).                */
/*************************************************************************/
{
    int i;
    int rzero = 0; 

        for ( i = samp_per_frame2; i > 1; i -= 2 )
            if ( !ix[i-1] && !ix[i-2] )
                rzero++;
            else
                break;
        
        cod_info->count1 = 0 ;
        for ( ; i > 3; i -= 4 )
            if ( abs(ix[i-1]) <= 1
              && abs(ix[i-2]) <= 1
              && abs(ix[i-3]) <= 1
              && abs(ix[i-4]) <= 1 )
                cod_info->count1++;
            else
                break;
        
        cod_info->big_values = i>>1;

}




int count1_bitcount(int ix[samp_per_frame2], gr_info *cod_info)
/*************************************************************************/
/* Determines the number of bits to encode the quadruples.               */
/*************************************************************************/
{
    int p, i, k;
    int v, w, x, y, signbits;
    int sum0 = 0,
        sum1 = 0;

    for(i=cod_info->big_values<<1, k=0; k<cod_info->count1; i+=4, k++)
    {
        v = abs(ix[i]);
        w = abs(ix[i+1]);
        x = abs(ix[i+2]);
        y = abs(ix[i+3]);

        p = v + (w<<1) + (x<<2) + (y<<3);
        
        signbits = 0;
        if(v!=0) signbits++;
        if(w!=0) signbits++;
        if(x!=0) signbits++;
        if(y!=0) signbits++;

        sum0 += signbits;
        sum1 += signbits;

        sum0 += ht[32].hlen[p];
        sum1 += ht[33].hlen[p];
    }

    if(sum0<sum1)
    {
        cod_info->count1table_select = 0;
        return sum0;
    }
    else
    {
        cod_info->count1table_select = 1;
        return sum1;
    }
}






void subdivide(gr_info *cod_info)
/*************************************************************************/
/* presumable subdivides the bigvalue region which will use separate Huffman tables.  */
/*************************************************************************/
{
    int scfb_anz = 0;
    int bigvalues_region;
    
    if ( !cod_info->big_values)
    { /* no big_values region */
        cod_info->region0_count = 0;
        cod_info->region1_count = 0;
    }
    else
    {
        bigvalues_region = 2 * cod_info->big_values;

        { 
            int thiscount, index;
            /* Calculate scfb_anz */
            while ( scalefac_band_long[scfb_anz] < bigvalues_region )
                scfb_anz++;

            cod_info->region0_count = subdv_table[scfb_anz].region0_count;
            thiscount = cod_info->region0_count;
            index = thiscount + 1;
            while ( thiscount && (scalefac_band_long[index] > bigvalues_region) )
            {
                thiscount--;
                index--;
            }
            cod_info->region0_count = thiscount;

            cod_info->region1_count = subdv_table[scfb_anz].region1_count;
            index = cod_info->region0_count + cod_info->region1_count + 2;
            thiscount = cod_info->region1_count;
            while ( thiscount && (scalefac_band_long[index] > bigvalues_region) )
            {
                thiscount--;
                index--;
            }
            cod_info->region1_count = thiscount;
            cod_info->address1 = scalefac_band_long[cod_info->region0_count+1];
            cod_info->address2 = scalefac_band_long[cod_info->region0_count
                                                    + cod_info->region1_count + 2 ];
            cod_info->address3 = bigvalues_region;
        }
    }
}






void bigv_tab_select( int ix[samp_per_frame2], gr_info *cod_info )
/*************************************************************************/
/*   Function: Select huffman code tables for bigvalues regions */
/*************************************************************************/
{
    cod_info->table_select[0] = 0;
    cod_info->table_select[1] = 0;
    cod_info->table_select[2] = 0;
    
    {
        if ( cod_info->address1 > 0 )
            cod_info->table_select[0] = new_choose_table( ix, 0, cod_info->address1 );

        if ( cod_info->address2 > cod_info->address1 )
            cod_info->table_select[1] = new_choose_table( ix, cod_info->address1, cod_info->address2 );

        if ( cod_info->big_values<<1 > cod_info->address2 )
            cod_info->table_select[2] = new_choose_table( ix, cod_info->address2, cod_info->big_values<<1 );
    }
}





int new_choose_table( int ix[samp_per_frame2], unsigned int begin, unsigned int end )
/*************************************************************************/
/*  Choose the Huffman table that will encode ix[begin..end] with             */
/*  the fewest bits.                                                          */
/*  Note: This code contains knowledge about the sizes and characteristics    */
/*  of the Huffman tables as defined in the IS (Table B.7), and will not work */
/*  with any arbitrary tables.                                                */
/*************************************************************************/
{
    int i, max;
    int choice[2];
    int sum[2];

    max = ix_max(ix,begin,end);
    if(!max)
		return 0;

    choice[0] = 0;
    choice[1] = 0;

    if(max<15)
    {
	/* try tables with no linbits */
        for ( i =14; i--; )
            if ( ht[i].xlen > max )
	    {
		choice[ 0 ] = i;
                break;
	    }

	sum[ 0 ] = count_bit( ix, begin, end, choice[0] );

	switch ( choice[0] )
	{
	  case 2:
	    sum[ 1 ] = count_bit( ix, begin, end, 3 );
	    if ( sum[1] <= sum[0] )
		choice[ 0 ] = 3;
	    break;

	  case 5:
	    sum[ 1 ] = count_bit( ix, begin, end, 6 );
	    if ( sum[1] <= sum[0] )
		choice[ 0 ] = 6;
	    break;

	  case 7:
	    sum[ 1 ] = count_bit( ix, begin, end, 8 );
	    if ( sum[1] <= sum[0] )
	    {
		choice[ 0 ] = 8;
		sum[ 0 ] = sum[ 1 ];
	    }
	    sum[ 1 ] = count_bit( ix, begin, end, 9 );
	    if ( sum[1] <= sum[0] )
		choice[ 0 ] = 9;
	    break;

	  case 10:
	    sum[ 1 ] = count_bit( ix, begin, end, 11 );
	    if ( sum[1] <= sum[0] )
	    {
		choice[ 0 ] = 11;
		sum[ 0 ] = sum[ 1 ];
	    }
	    sum[ 1 ] = count_bit( ix, begin, end, 12 );
	    if ( sum[1] <= sum[0] )
		choice[ 0 ] = 12;
	    break;

	  case 13:
	    sum[ 1 ] = count_bit( ix, begin, end, 15 );
	    if ( sum[1] <= sum[0] )
		choice[ 0 ] = 15;
	    break;
	}
    }
    else
    {
	/* try tables with linbits */
	max -= 15;
	
	for(i=15;i<24;i++)
	{
	    if(ht[i].linmax>=max)
	    {
		choice[ 0 ] = i;
		break;
	    }
	}
	for(i=24;i<32;i++)
	{
	    if(ht[i].linmax>=max)
	    {
		choice[ 1 ] = i;
		break;
	    }
	}
	
	sum[0] = count_bit(ix,begin,end,choice[0]);
	sum[1] = count_bit(ix,begin,end,choice[1]);
	if (sum[1]<sum[0])
		choice[0] = choice[1];
    }
    return choice[0];
}



int bigv_bitcount(int ix[samp_per_frame2], gr_info *gi)
/*************************************************************************/
/* Function: Count the number of bits necessary to code the bigvalues region.  */
/*************************************************************************/
{
    int bits = 0;
    
        unsigned int table;
        
        if( (table=gi->table_select[0]))  /* region0 */ 
            bits += count_bit(ix, 0, gi->address1, table );
        if( (table=gi->table_select[1]))  /* region1 */ 
            bits += count_bit(ix, gi->address1, gi->address2, table );
        if( (table=gi->table_select[2]))  /* region2 */ 
            bits += count_bit(ix, gi->address2, gi->address3, table );
    return bits;
}



int count_bit(int ix[samp_per_frame2], 
              unsigned int start, 
              unsigned int end,
              unsigned int table )
/*************************************************************************/
/* Function: Count the number of bits necessary to code the subregion. */
/*************************************************************************/
{
    unsigned            linbits, ylen;
    register int        i, sum;
    register int        x,y;
    struct huffcodetab *h;

    if(!table)
		return 0;

    h   = &(ht[table]);
    sum = 0;

    ylen    = h->ylen;
    linbits = h->linbits;

    if(table>15)
    { // ESC-table is used 
        for(i=start;i<end;i+=2)
        {
            x = ix[i];
            y = ix[i+1];
            if(x>14)
            {
                x = 15;
                sum += linbits;
            }
            if(y>14)
            {
                y = 15;
                sum += linbits;
            }

            sum += h->hlen[(x*ylen)+y];

            if(x)
				sum++;
            if(y)
				sum++;
        }
    }
    else
    { /* No ESC-words */
        for(i=start;i<end;i+=2)
        {
            x = ix[i];
            y = ix[i+1];

            sum  += h->hlen[(x*ylen)+y];

            if(x!=0) sum++;
            if(y!=0) sum++;
        }
    }

    return sum;
}









/* The following optional code written by Seymour Shlien
   will speed up the outer_loop code which is called
   by iteration_loop. When BIN_SEARCH is defined, the
   outer_loop function precedes the call to the function inner_loop
   with a call to bin_search gain defined below, which
   returns a good starting quantizerStepSize.
*/

int count_bits(int *ix /*int[samp_per_frame2]*/, gr_info *cod_info)
{
    int bits,max;

    calc_runlen(ix,cod_info);		  /* rzero,count1,big_values*/

    max = ix_max( ix, 0,samp_per_frame2);
    if(max > 8192)
		return 100000;         /* report unsuitable quantizer */

    bits = count1_bitcount(ix, cod_info); /* count1_table selection*/
    subdivide(cod_info);		  /* bigvalues sfb division */
    bigv_tab_select(ix,cod_info);         /* codebook selection*/
    bits += bigv_bitcount(ix,cod_info);	  /* bit count */
    return bits;
}



int bin_search_StepSize(int desired_rate, double start, int *ix,
                        double xrs[samp_per_frame2], gr_info * cod_info)
{
    int top,bot,next,last;
    int bit;

    top  = start;
    next = start;
    bot  = 200;

    do
    {
        last = next;
        next = ((long)(top+bot) >> 1);

        cod_info->quantizerStepSize = next;
        quantize(xrs,ix,cod_info);
        bit = count_bits(ix,cod_info);

        if (bit>desired_rate)
			top = next;
        else
			bot = next;
    }
    while((bit!=desired_rate) && abs(last-next)>1);

    return next;
}


