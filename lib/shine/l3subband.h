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


#ifndef SUBBAND_H
#define SUBBAND_H

#include "types.h"

void subband_initialise();
void window_subband(short **buffer, double z[HAN_SIZE], int k);
void filter_subband(double z[HAN_SIZE], double s[SBLIMIT]);

#endif
