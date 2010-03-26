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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "error.h"
#include "wave.h"
#include "layer3.h"

#include "AS3.h"

config_t config;

static void print_header()
{
    fprintf(stderr,"Shine 0.1.4");
}

static void set_defaults()
{
    config.byte_order = DetermineByteOrder();
    if(config.byte_order==order_unknown) ERROR("Can't determine byte order");

    config.mpeg.type = 1;
    config.mpeg.layr = 2;
    config.mpeg.mode = (config.wave.channels==1) ? MODE_MONO :MODE_STEREO;
    config.mpeg.bitr = 128;
    config.mpeg.psyc = 2;
    config.mpeg.emph = 0; 
    config.mpeg.crc  = 0;
    config.mpeg.ext  = 0;
    config.mpeg.mode_ext  = 0;
    config.mpeg.copyright = 0;
    config.mpeg.original  = 1;
}

static int find_samplerate_index(long freq)
{
    static long mpeg1[3] = {44100, 48000, 32000};
    int i;

    for(i=0;i<3;i++)
        if(freq==mpeg1[i]) return i;

    ERROR("Invalid samplerate");
    return -1;
}

static int find_bitrate_index(int bitr)
{
    static long mpeg1[15] = {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320};
    int i;

    for(i=0;i<15;i++)
        if(bitr==mpeg1[i]) return i;

    ERROR("Invalid bitrate");
    return -1;
}

static void check_config()
{
    static char *mode_names[4]    = { "stereo", "j-stereo", "dual-ch", "mono" };
    static char *layer_names[3]   = { "I", "II", "III" };
    static char *version_names[2] = { "MPEG-II (LSF)", "MPEG-I" };
    static char *psy_names[3]     = { "", "MUSICAM", "Shine" };
    static char *demp_names[4]    = { "none", "50/15us", "", "CITT" };

    config.mpeg.samplerate_index = find_samplerate_index(config.wave.samplerate);
    config.mpeg.bitrate_index    = find_bitrate_index(config.mpeg.bitr);

    fprintf(stderr, "%s layer %s, %s  Psychoacoustic Model: %s",
           version_names[config.mpeg.type],
           layer_names[config.mpeg.layr], 
           mode_names[config.mpeg.mode],
           psy_names[config.mpeg.psyc]);
    fprintf(stderr, "Bitrate=%d kbps  ",config.mpeg.bitr );
    fprintf(stderr, "De-emphasis: %s   %s %s\n",
          demp_names[config.mpeg.emph], 
          ((config.mpeg.original)?"Original":""),
          ((config.mpeg.copyright)?"(C)":""));

}

// ALCHEMY UTILS


int readByteArray(void *cookie, char *dst, int size)
{
	return AS3_ByteArray_readBytes(dst, (AS3_Val)cookie, size);
}

int writeByteArray(void *cookie, const char *src, int size)
{
	return AS3_ByteArray_writeBytes((AS3_Val)cookie, (char *)src, size);
}

fpos_t seekByteArray(void *cookie, fpos_t offs, int whence)
{
	return AS3_ByteArray_seek((AS3_Val)cookie, offs, whence);
}

int closeByteArray(void * cookie)
{
	AS3_Val zero = AS3_Int(0);
	
	/* just reset the position */
	AS3_SetS((AS3_Val)cookie, "position", zero);
	AS3_Release(zero);
	return 0;
}

static AS3_Val init(void * self, AS3_Val args)
{
	void * ref;
	void * src;
	void * dest;
	
	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType", &ref, &src, &dest);
	
	flashErrorsRef = (AS3_Val)ref;
	
	config.wave.file	= funopen((void *)src, readByteArray, writeByteArray, seekByteArray, closeByteArray);
	config.wave.output	= funopen((void *)dest, readByteArray, writeByteArray, seekByteArray, closeByteArray);
	
	if (config.wave.file == NULL || config.wave.output == NULL) {
		ERROR("Unable to set bytes arrays");
	}
	
	wave_open();
	
	set_defaults();
    check_config();
		
	start_compress();
	
	return AS3_Int(1);
}

static AS3_Val update(void* self, AS3_Val args) {
	
	int statut = update_compress();
	
	if(statut==100) wave_close();
	
	return AS3_Int(statut);
}


int main(int argc, char **argv)
{
    print_header();
	
	// set convert & update methods visible to flash
	AS3_Val convertMethod = AS3_Function( NULL, init );
	AS3_Val updateMethod = AS3_Function( NULL, update );
	
	AS3_Val result = AS3_Object( "init:AS3ValType, update: AS3ValType", convertMethod, updateMethod );
	
	AS3_Release( convertMethod );
	AS3_Release( updateMethod );
	
	// notify that we initialized -- THIS DOES NOT RETURN!
	AS3_LibInit( result );
	
    return 0;
	 
} 

