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


#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <time.h>
#include "portableio.h"



#define samp_per_frame  1152
#define samp_per_frame2  576

#define sq(a) ((a)*(a))

#define         PI                      3.14159265358979
#define         PI4                     .78539816339745
#define         PI12                     .26179938779915
#define         PI36                    .087266462599717
#define         PI64                    .049087385212
#define SQRT2       1.41421356237
#define LN_TO_LOG10 0.2302585093
#define BLKSIZE     1024
#define HAN_SIZE    512
#define SCALE_BLOCK 12
#define SCALE_RANGE 64
#define SCALE       32768
#define SBLIMIT     32



typedef struct {
    FILE *file;
	FILE *output;
    int  type;
    int  channels;
    int  bits;
    long samplerate;
    long total_samples;
    long length;
} wave_t;

typedef struct {
    FILE *file;
    int  type;
    int  layr;
    int  mode;
    int  bitr;
    int  psyc;
    int  emph;
    int  padding;
    long samples_per_frame;
    long bits_per_frame;
    long bits_per_slot;
    long total_frames;
    int  bitrate_index;
    int  samplerate_index;
    int crc;
    int ext;
    int mode_ext;
    int copyright;
    int original;
} mpeg_t;

typedef struct {
	
    time_t start_time;
    enum e_byte_order byte_order; 

    wave_t wave;
    mpeg_t mpeg;
	
} config_t;
extern config_t config;


#ifndef bool
typedef unsigned char bool;
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define WAVE_RIFF_PCM 0
#define WAVE_PCM_LOHI 1
#define WAVE_PCM_HILO 2
#define WAVE_PCM_AIFF 3

#define MODE_STEREO   0
#define MODE_MONO     3


#endif
