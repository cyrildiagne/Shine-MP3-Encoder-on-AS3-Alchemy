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


#ifndef PORTABLEIO_H__
#define PORTABLEIO_H__
/* Copyright (C) 1988-1991 Apple Computer, Inc.
 * All Rights Reserved.
 *
 * Warranty Information
 * Even though Apple has reviewed this software, Apple makes no warranty
 * or representation, either express or implied, with respect to this
 * software, its quality, accuracy, merchantability, or fitness for a 
 * particular purpose.  As a result, this software is provided "as is,"
 * and you, its user, are assuming the entire risk as to its quality
 * and accuracy.
 *
 * This code may be used and freely distributed as long as it includes
 * this copyright notice and the warranty information.
 *
 * Machine-independent I/O routines for 8-, 16-, 24-, and 32-bit integers.
 *
 * Motorola processors (Macintosh, Sun, Sparc, MIPS, etc)
 * pack bytes from high to low (they are big-endian).
 * Use the HighLow routines to match the native format
 * of these machines.
 *
 * Intel-like machines (PCs, Sequent)
 * pack bytes from low to high (the are little-endian).
 * Use the LowHigh routines to match the native format
 * of these machines.
 *
 * These routines have been tested on the following machines:
 *	Apple Macintosh, MPW 3.1 C compiler
 *	Apple Macintosh, THINK C compiler
 *	Silicon Graphics IRIS, MIPS compiler
 *	Cray X/MP and Y/MP
 *	Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 *
 * $Id: portableio.h,v 2.6 1991/04/30 17:06:02 malcolm Exp $
 *
 * $Log: portableio.h,v $
 * Revision 2.6  91/04/30  17:06:02  malcolm
 */

#include	<stdio.h>
#include	"ieeefloat.h"

#ifndef	__cplusplus
# define	CLINK	
#else
# define	CLINK "C"
#endif

extern CLINK int ReadByte(FILE *fp);
extern CLINK int Read16BitsLowHigh(FILE *fp);
extern CLINK int Read16BitsHighLow(FILE *fp);
extern CLINK void Write8Bits(FILE *fp, int i);
extern CLINK void Write16BitsLowHigh(FILE *fp, int i);
extern CLINK void Write16BitsHighLow(FILE *fp, int i);
extern CLINK int Read24BitsHighLow(FILE *fp);
extern CLINK int Read32Bits(FILE *fp);
extern CLINK int Read32BitsHighLow(FILE *fp);
extern CLINK void Write32Bits(FILE *fp, int i);
extern CLINK void Write32BitsLowHigh(FILE *fp, int i);
extern CLINK void Write32BitsHighLow(FILE *fp, int i);
extern CLINK void ReadBytes(FILE *fp, char *p, int n);
extern CLINK void ReadBytesSwapped(FILE *fp, char *p, int n);
extern CLINK void WriteBytes(FILE *fp, char *p, int n);
extern CLINK void WriteBytesSwapped(FILE *fp, char *p, int n);
extern CLINK defdouble ReadIeeeFloatHighLow(FILE *fp);
extern CLINK defdouble ReadIeeeFloatLowHigh(FILE *fp);
extern CLINK defdouble ReadIeeeDoubleHighLow(FILE *fp);
extern CLINK defdouble ReadIeeeDoubleLowHigh(FILE *fp);
extern CLINK defdouble ReadIeeeExtendedHighLow(FILE *fp);
extern CLINK defdouble ReadIeeeExtendedLowHigh(FILE *fp);
extern CLINK void WriteIeeeFloatLowHigh(FILE *fp, defdouble num);
extern CLINK void WriteIeeeFloatHighLow(FILE *fp, defdouble num);
extern CLINK void WriteIeeeDoubleLowHigh(FILE *fp, defdouble num);
extern CLINK void WriteIeeeDoubleHighLow(FILE *fp, defdouble num);
extern CLINK void WriteIeeeExtendedLowHigh(FILE *fp, defdouble num);
extern CLINK void WriteIeeeExtendedHighLow(FILE *fp, defdouble num);

enum e_byte_order {order_unknown,order_bigEndian,order_littleEndian};
enum e_byte_order DetermineByteOrder();
void SwapBytesInWords(short *loc,int words);


#define	Read32BitsLowHigh(f)	Read32Bits(f)
#define WriteString(f,s)	fwrite(s,strlen(s),sizeof(char),f)
#endif
