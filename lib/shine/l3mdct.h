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


#ifndef _MDCT_H
#define _MDCT_H

#include "types.h"
#include "layer3.h"

void mdct_initialise();
void mdct_sub(double sb_sample[2][3][18][SBLIMIT], 
                 double (*mdct_freq)[2][samp_per_frame2], 
                 side_info_t *side_info);

#endif
