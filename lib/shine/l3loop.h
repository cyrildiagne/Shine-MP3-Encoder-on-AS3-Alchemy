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


#ifndef L3LOOP_H
#define L3LOOP_H

#include "types.h"
#include "layer3.h"

#define e              2.71828182845


void iteration_loop(double mdct_freq_org[2][2][samp_per_frame2], 
       		       side_info_t *side_info, 
                       int enc[2][2][samp_per_frame2],
		       int mean_bits, 
		       scalefac_t  *scalefacitor );

#endif

