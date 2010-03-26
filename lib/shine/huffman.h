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


#include "bitstream.h"

#define HUFFBITS unsigned long int
#define HTN	34
#define MXOFF	250
 
struct huffcodetab {
  unsigned int xlen; 	/*max. x-index+			      	*/ 
  unsigned int ylen;	/*max. y-index+				*/
  unsigned int linbits; /*number of linbits			*/
  unsigned int linmax;	/*max number to be stored in linbits	*/
  HUFFBITS *table;	/*pointer to array[xlen][ylen]		*/
  unsigned char *hlen;	/*pointer to array[xlen][ylen]		*/
};

extern struct huffcodetab ht[HTN];/* global memory block		*/
				/* array of all huffcodtable headers	*/
				/* 0..31 Huffman code table 0..31	*/
				/* 32,33 count1-tables			*/


