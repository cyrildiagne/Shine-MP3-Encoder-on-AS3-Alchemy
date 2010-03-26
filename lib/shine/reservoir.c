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
#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "error.h"
#include "layer3.h"
#include "l3loop.h"
#include "huffman.h"
#include "l3bitstream.h"
#include "reservoir.h"

/* Layer3 bit reservoir: Described in C.1.5.4.2.2 of the IS */
static int ResvSize = 0; /* in bits */






/*
  ResvAdjust:
  Called after a granule's bit allocation. Readjusts the size of
  the reservoir to reflect the granule's usage.
*/
void ResvAdjust(gr_info *gi, int mean_bits )
{
    ResvSize += (mean_bits / config.wave.channels) - gi->part2_3_length;

}



/*
  ResvFrameEnd:
  Called after all granules in a frame have been allocated. Makes sure
  that the reservoir size is within limits, possibly by adding stuffing
  bits. Note that stuffing bits are added by increasing a granule's
  part2_3_length. The bitstream formatter will detect this and write the
  appropriate stuffing bits to the bitstream.
*/
void ResvFrameEnd(side_info_t *side)
{
    gr_info *gi;

	gi = (gr_info *) &(side->gr[0].ch[0]);	
	gi->part2_3_length += ResvSize;
    ResvSize = 0;
}


