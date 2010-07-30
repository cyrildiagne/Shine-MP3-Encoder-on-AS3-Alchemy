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


#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include "types.h"
#include "bitstream.h"
#include "layer3.h"

void flush_bitstream();

void format_bitstream(int enc[2][2][samp_per_frame2],
                         side_info_t  *side,
			 scalefac_t   *scalefac,
			 bitstream_t  *in_bs,
			 double           (*xr)[2][samp_per_frame2]);

int HuffmanCode(int table_select, int x, int y, unsigned *code,
                unsigned int *extword, int *codebits, int *extbits);

int abs_and_sign(int *x); /* returns signx and changes *x to abs(*x) */

#endif
