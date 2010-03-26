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


#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <malloc.h>

#include "types.h"
#include "error.h"
#include "bitstream.h"

/*****************************************************************************
*
*  bit_stream.c package
*  Author:  Jean-Georges Fritsch, C-Cube Microsystems
*
*****************************************************************************/

/********************************************************************
  This package provides functions to write (exclusive or read)
  information from (exclusive or to) the bit stream.

  If the bit stream is opened in read mode only the get functions are
  available. If the bit stream is opened in write mode only the put
  functions are available.
********************************************************************/

/*open_bit_stream_w(); open the device to write the bit stream into it    */
/*open_bit_stream_r(); open the device to read the bit stream from it     */
/*close_bit_stream();  close the device containing the bit stream         */
/*alloc_buffer();      open and initialize the buffer;                    */
/*desalloc_buffer();   empty and close the buffer                         */
/*back_track_buffer();     goes back N bits in the buffer                 */
/*unsigned int get1bit();  read 1 bit from the bit stream                 */
/*unsigned long getbits(); read N bits from the bit stream                */
/*unsigned long byte_ali_getbits();   read the next byte aligned N bits from*/
/*                                    the bit stream                        */
/*unsigned long look_ahead(); grep the next N bits in the bit stream without*/
/*                            changing the buffer pointer                   */
/*put1bit(); write 1 bit from the bit stream  */
/*put1bit(); write 1 bit from the bit stream  */
/*putbits(); write N bits from the bit stream */
/*byte_ali_putbits(); write byte aligned the next N bits into the bit stream*/
/*unsigned long sstell(); return the current bit stream length (in bits)    */
/*int end_bs(); return 1 if the end of bit stream reached otherwise 0       */
/*int seek_sync(); return 1 if a sync word was found in the bit stream      */
/*                 otherwise returns 0                                      */

/* refill the buffer from the input device when the buffer becomes empty    */
int refill_buffer(bs)
bitstream_t *bs;   /* bit stream structure */
{
   register int i=bs->buf_size-2-bs->buf_byte_idx;
   register unsigned long n=1;
   register int index=0;
   char val[2];

   while ((i>=0) && (!bs->eob)) {

      if (bs->format == BINARY)
         n = fread(&bs->buf[i--], sizeof(unsigned char), 1, bs->pt);

      else {
         while((index < 2) && n) {
            n = fread(&val[index], sizeof(char), 1, bs->pt);
            switch (val[index]) {
                  case 0x30:
                  case 0x31:
                  case 0x32:
                  case 0x33:
                  case 0x34:
                  case 0x35:
                  case 0x36:
                  case 0x37:
                  case 0x38:
                  case 0x39:
                  case 0x41:
                  case 0x42:
                  case 0x43:
                  case 0x44:
                  case 0x45:
                  case 0x46:
                  index++;
                  break;
                  default: break;
            }
         }

         if (val[0] <= 0x39)   bs->buf[i] = (val[0] - 0x30) << 4;
                 else  bs->buf[i] = (val[0] - 0x37) << 4;
         if (val[1] <= 0x39)   bs->buf[i--] |= (val[1] - 0x30);
                 else  bs->buf[i--] |= (val[1] - 0x37);
         index = 0;
      }

      if (!n) {
         bs->eob= i+1;
      }

    }
   return 0;
}

/* empty the buffer to the output device when the buffer becomes full */
void empty_buffer(bs, minimum)
bitstream_t *bs;   /* bit stream structure */
int minimum;            /* end of the buffer to empty */
{
   register int i;

//#if BS_FORMAT == BINARY
   for (i=bs->buf_size-1;i>=minimum;i--)
      fwrite(&bs->buf[i], sizeof(unsigned char), 1, bs->pt);
//#else
//   for (i=bs->buf_size-1;i>=minimum;i--) {
//       char val[2];
//       val[0] = he[((bs->buf[i] >> 4) & 0x0F)];
//       val[1] = he[(bs->buf[i] & 0x0F)];
//       fwrite(val, sizeof(char), 2, bs->pt);
//   }
//#endif
fflush(bs->pt); /* NEW SS to assist in debugging*/

   for (i=minimum-1; i>=0; i--)
       bs->buf[bs->buf_size - minimum + i] = bs->buf[i];

   bs->buf_byte_idx = bs->buf_size -1 - minimum;
   bs->buf_bit_idx = 8;
}

/* open the device to write the bit stream into it */
void open_bit_stream_w(bs, bs_filenam, size)
bitstream_t *bs;   /* bit stream structure */
char *bs_filenam;       /* name of the bit stream file */
int size;               /* size of the buffer */
{
   
   if ((bs->pt = fopen(bs_filenam, "wb")) == NULL) {
      printf("Could not create \"%s\".\n", bs_filenam);
      exit(1);
   }
	
   alloc_buffer(bs, size);
   bs->buf_byte_idx = size-1;
   bs->buf_bit_idx=8;
   bs->totbit=0;
   bs->mode = WRITE_MODE;
   bs->eob = FALSE;
   bs->eobs = FALSE;
}

void open_bit_stream_ba(bs, size)
bitstream_t *bs;   /* bit stream structure */
int size;               /* size of the buffer */
{
	
	alloc_buffer(bs, size);
	bs->buf_byte_idx = size-1;
	bs->buf_bit_idx=8;
	bs->totbit=0;
	bs->mode = WRITE_MODE;
	bs->eob = FALSE;
	bs->eobs = FALSE;
}

/* open the device to read the bit stream from it */
void open_bit_stream_r(bs, bs_filenam, size)
bitstream_t *bs;   /* bit stream structure */
char *bs_filenam;       /* name of the bit stream file */
int size;               /* size of the buffer */
{
   register unsigned long n;
   register unsigned char flag = 1;
   unsigned char val;
	
   if ((bs->pt = fopen(bs_filenam, "rb")) == NULL) {
      printf("Could not find \"%s\".\n", bs_filenam);
      exit(1);
   }

   do {
     n = fread(&val, sizeof(unsigned char), 1, bs->pt);
     switch (val) {
      case 0x30:
      case 0x31:
      case 0x32:
      case 0x33:
      case 0x34:
      case 0x35:
      case 0x36:
      case 0x37:
      case 0x38:
      case 0x39:
      case 0x41:
      case 0x42:
      case 0x43:
      case 0x44:
      case 0x45:
      case 0x46:
      case 0xa:  /* \n */
      case 0xd:  /* cr */
      case 0x1a:  /* sub */
          break;

      default: /* detection of an binary character */
          flag--;
          break;
     }

   } while (flag & n);

   if (flag) {
      printf ("the bit stream file %s is an ASCII file\n", bs_filenam);
      bs->format = ASCII;
   }
   else {
      bs->format = BINARY;
      printf ("the bit stream file %s is a BINARY file\n", bs_filenam);
   }

   fclose(bs->pt);

   if ((bs->pt = fopen(bs_filenam, "rb")) == NULL) {
      printf("Could not find \"%s\".\n", bs_filenam);
      exit(1);
   }

   alloc_buffer(bs, size);
   bs->buf_byte_idx=0;
   bs->buf_bit_idx=0;
   bs->totbit=0;
   bs->mode = READ_MODE;
   bs->eob = FALSE;
   bs->eobs = FALSE;
}

/*close the device containing the bit stream after a read process*/
void close_bit_stream_r(bs)
bitstream_t *bs;   /* bit stream structure */
{
   fclose(bs->pt);
   desalloc_buffer(bs);
}


/*open and initialize the buffer; */
void alloc_buffer(bs, size)
bitstream_t *bs;   /* bit stream structure */
int size;
{
   bs->buf = (unsigned char *)malloc(size*sizeof(unsigned char));
   bs->buf_size = size;
}

/*empty and close the buffer */
void desalloc_buffer(bs)
bitstream_t *bs;   /* bit stream structure */
{
   free(bs->buf);
}

int putmask[9]={0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
int clearmask[9]={0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x0};

void back_track_buffer(bs, N) /* goes back N bits in the buffer */
bitstream_t *bs;   /* bit stream structure */
int N;
{
   int tmp =0;
   register int i;

   bs->totbit -= N;
   for (i=bs->buf_byte_idx;i< bs->buf_byte_idx+(N>>3)-1;i++)
	   bs->buf[i] = 0;
   bs->buf_byte_idx += N>>3;
   if ( (tmp + bs->buf_bit_idx) <= 8) {
      bs->buf_bit_idx += tmp;
   }
   else {
      bs->buf_byte_idx ++;
      bs->buf_bit_idx += (tmp - 8);
   }
   bs->buf[bs->buf_byte_idx] &= clearmask[bs->buf_bit_idx];
}

int mask[8]={0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

/*read 1 bit from the bit stream */
unsigned int get1bit(bs)
bitstream_t *bs;   /* bit stream structure */
{
   unsigned int bit;
   register int i;

   bs->totbit++;

   if (!bs->buf_bit_idx) {
        bs->buf_bit_idx = 8;
        bs->buf_byte_idx--;
        if ((bs->buf_byte_idx < MINIMUM) || (bs->buf_byte_idx < bs->eob)) {
             if (bs->eob)
                bs->eobs = TRUE;
             else {
                for (i=bs->buf_byte_idx; i>=0;i--)
                  bs->buf[bs->buf_size-1-bs->buf_byte_idx+i] = bs->buf[i];
                refill_buffer(bs);
                bs->buf_byte_idx = bs->buf_size-1;
             }
        }
   }
   bit = bs->buf[bs->buf_byte_idx]&mask[bs->buf_bit_idx-1];
   bit = bit >> (bs->buf_bit_idx-1);
   bs->buf_bit_idx--;
   return(bit);
}

/*write 1 bit from the bit stream */
void put1bit(bs, bit)
bitstream_t *bs;   /* bit stream structure */
int bit;                /* bit to write into the buffer */
{
   bs->totbit++;

   bs->buf[bs->buf_byte_idx] |= (bit&0x1) << (bs->buf_bit_idx-1);
   bs->buf_bit_idx--;
   if (!bs->buf_bit_idx) {
       bs->buf_bit_idx = 8;
       bs->buf_byte_idx--;
       if (bs->buf_byte_idx < 0)
          empty_buffer(bs, MINIMUM);
       bs->buf[bs->buf_byte_idx] = 0;
   }
}

/*look ahead for the next N bits from the bit stream */
unsigned long look_ahead(bs, N)
bitstream_t *bs;   /* bit stream structure */
int N;                  /* number of bits to read from the bit stream */
{
 unsigned long val=0;
 register int j = N;
 register int k, tmp;
 register int bit_idx = bs->buf_bit_idx;
 register int byte_idx = bs->buf_byte_idx;

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);

 while (j > 0) {
    if (!bit_idx) {
        bit_idx = 8;
        byte_idx--;
    }
    k = MIN (j, bit_idx);
    tmp = bs->buf[byte_idx]&putmask[bit_idx];
    tmp = tmp >> (bit_idx-k);
    val |= tmp << (j-k);
    bit_idx -= k;
    j -= k;
 }
 return(val);
}

/*read N bit from the bit stream */
unsigned long getbits(bs, N)
bitstream_t *bs;   /* bit stream structure */
int N;                  /* number of bits to read from the bit stream */
{
 unsigned long val=0;
 register int i;
 register int j = N;
 register int k, tmp;

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);

 bs->totbit += N;
 while (j > 0) {
   if (!bs->buf_bit_idx) {
        bs->buf_bit_idx = 8;
        bs->buf_byte_idx--;
        if ((bs->buf_byte_idx < MINIMUM) || (bs->buf_byte_idx < bs->eob)) {
             if (bs->eob)
                bs->eobs = TRUE;
             else {
                for (i=bs->buf_byte_idx; i>=0;i--)
                   bs->buf[bs->buf_size-1-bs->buf_byte_idx+i] = bs->buf[i];
                refill_buffer(bs);
                bs->buf_byte_idx = bs->buf_size-1;
             }
        }
   }
   k = MIN (j, bs->buf_bit_idx);
   tmp = bs->buf[bs->buf_byte_idx]&putmask[bs->buf_bit_idx];
   tmp = tmp >> (bs->buf_bit_idx-k);
   val |= tmp << (j-k);
   bs->buf_bit_idx -= k;
   j -= k;
 }
 return(val);
}

/*write N bits into the bit stream */
void putbits(bs, val, N)
bitstream_t *bs;   /* bit stream structure */
unsigned int val;       /* val to write into the buffer */
int N;                  /* number of bits of val */
{
 register int j = N;
 register int k, tmp;

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);

 bs->totbit += N;
 while (j > 0) {
   k = MIN(j, bs->buf_bit_idx);
   tmp = val >> (j-k);
   bs->buf[bs->buf_byte_idx] |= (tmp&putmask[k]) << (bs->buf_bit_idx-k);
   bs->buf_bit_idx -= k;
   if (!bs->buf_bit_idx) {
       bs->buf_bit_idx = 8;
       bs->buf_byte_idx--;
       if (bs->buf_byte_idx < 0)
          empty_buffer(bs, MINIMUM);
       bs->buf[bs->buf_byte_idx] = 0;
   }
   j -= k;
 }
}

/*write N bits byte aligned into the bit stream */
void byte_ali_putbits(bs, val, N)
bitstream_t *bs;   /* bit stream structure */
unsigned int val;       /* val to write into the buffer */
int N;                  /* number of bits of val */
{
 unsigned long aligning, sstell();

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);
 aligning = sstell(bs)%8;
 if (aligning)
     putbits(bs, (unsigned int)0, (int)(8-aligning)); 

 putbits(bs, val, N);
}

/*read the next bute aligned N bits from the bit stream */
unsigned long byte_ali_getbits(bs, N)
bitstream_t *bs;   /* bit stream structure */
int N;                  /* number of bits of val */
{
 unsigned long aligning, sstell();

 if (N > MAX_LENGTH)
    printf("Cannot read or write more than %d bits at a time.\n", MAX_LENGTH);
 aligning = sstell(bs)%8;
 if (aligning)
    getbits(bs, (int)(8-aligning));

 return(getbits(bs, N));
}

/*return the current bit stream length (in bits)*/
unsigned long sstell(bs)
bitstream_t *bs;   /* bit stream structure */
{
  return(bs->totbit);
}

/*return the status of the bit stream*/
/* returns 1 if end of bit stream was reached */
/* returns 0 if end of bit stream was not reached */
int end_bs(bs)
bitstream_t *bs;   /* bit stream structure */
{
  return(bs->eobs);
}

/*this function seeks for a byte aligned sync word in the bit stream and
  places the bit stream pointer right after the sync.
  This function returns 1 if the sync was found otherwise it returns 0  */
int seek_sync(bs, sync, N)
bitstream_t *bs;   /* bit stream structure */
long sync;      /* sync word maximum 32 bits */
int N;          /* sync word length */
{
#if defined(MACINTOSH) && !defined(__powerc)
 double pow();
#endif
 unsigned long aligning, stell();
 unsigned long val;
 long maxi = (int)pow(2.0, (float)N) - 1;

 aligning = sstell(bs)%ALIGNING;
 if (aligning)
    getbits(bs, (int)(ALIGNING-aligning));

  val = getbits(bs, N);
  while (((val&maxi) != sync) && (!end_bs(bs))) {
        val <<= ALIGNING;
        val |= getbits(bs, ALIGNING);
  }

 if (end_bs(bs)) return(0);
 else return(1);
}



#define BUFSIZE 4096
static unsigned long offset,totbit=0, buf_byte_idx=0;
static unsigned int buf[BUFSIZE];
static unsigned int buf_bit_idx=8;

unsigned long hgetbits(int N)
{
 unsigned long val=0;
 register int j = N;
 register int k, tmp;

 totbit += N;
 while (j > 0) {
   if (!buf_bit_idx) {
        buf_bit_idx = 8;
        buf_byte_idx++;
   }
   k = MIN (j, buf_bit_idx);
   tmp = buf[buf_byte_idx%BUFSIZE]&putmask[buf_bit_idx];
   tmp = tmp >> (buf_bit_idx-k);
   val |= tmp << (j-k);
   buf_bit_idx -= k;
   j -= k;
 }
 return(val);
}

/*****************************************************************************
*
*  End of bit_stream.c package
*
*****************************************************************************/
