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


#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "types.h"

typedef struct  bit_stream_struc {
    FILE        *pt;            /* pointer to bit stream device */
    unsigned char *buf;         /* bit stream buffer */
    int         buf_size;       /* size of buffer (in number of bytes) */
    long        totbit;         /* bit counter of bit stream */
    int         buf_byte_idx;   /* pointer to top byte in buffer */
    int         buf_bit_idx;    /* pointer to top bit of top byte in buffer */
    int         mode;           /* bit stream open in read or write mode */
    int         eob;            /* end of buffer index */
    int         eobs;           /* end of bit stream flag */
    char        format;
    
    /* format of file in rd mode (BINARY/ASCII) */
} bitstream_t;

/* "bit_stream.h" Definitions */

#define         MINIMUM         4    /* Minimum size of the buffer in bytes */
#define         MAX_LENGTH      32   /* Maximum length of word written or
                                        read from bit stream */
#define         READ_MODE       0
#define         WRITE_MODE      1
#define         ALIGNING        8
#define         BINARY          0
#define         ASCII           1

#define         TRUE            1
#define         FALSE           0

#ifndef BS_FORMAT
#define         BS_FORMAT       ASCII /* BINARY or ASCII = 2x bytes */
#endif

#define         BUFFER_SIZE     4096

#define         MIN(A, B)       ((A) < (B) ? (A) : (B))
#define         MAX(A, B)       ((A) > (B) ? (A) : (B))


int refill_buffer(bitstream_t *bs);
void empty_buffer(bitstream_t *bs,int minimum);
void open_bit_stream_w(bitstream_t *bs,char *bs_filenam,int size);
void open_bit_stream_r(bitstream_t *bs,char *bs_filenam,int size);
void open_bit_stream_ba(bitstream_t *bs, int size);
void close_bit_stream_r(bitstream_t *bs);
void alloc_buffer(bitstream_t *bs,int size);
void desalloc_buffer(bitstream_t *bs);
void back_track_buffer(bitstream_t *bs,int N);
unsigned int get1bit(bitstream_t *bs);
void put1bit(bitstream_t *bs,int bit);
unsigned long look_ahead(bitstream_t *bs,int N);
unsigned long getbits(bitstream_t *bs,int N);
void putbits(bitstream_t *bs,unsigned int val,int N);
void byte_ali_putbits(bitstream_t *bs,unsigned int val,int N);
unsigned long byte_ali_getbits(bitstream_t *bs,int N);
unsigned long sstell(bitstream_t *bs);
int end_bs(bitstream_t *bs);
int seek_sync(bitstream_t *bs,long sync,int N);

unsigned long hgetbits(int N);
#define  hget1bit() hgetbits(1)

#endif
