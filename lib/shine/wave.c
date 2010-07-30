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


#include <stdio.h>
#include "portableio.h"

#include "types.h"
#include "error.h"
#include "wave.h"

static bool checkString(FILE *file,char *string)
{
/*    char temp[1024];
    int  length = strlen(string);
    if(fread(temp,1,length,file)!=length) ERROR("Premature EOF in input !?!");
    temp[length] = (char)0;
    return !strcmp(temp,string);*/

    

	char temp[1024];
    int  length = strlen(string);
	while (true)
	{
		if(fread(temp,1,length,file))
		{
			temp[length] = (char)0;
			if (!strcmp(temp,string))
				return !strcmp(temp,string);
		}
		if (temp[0]==EOF)
			break;
		fseek(file, -3, 1);
	}
	return 0;
}


void wave_close()
{
    fclose(config.wave.file);
}


bool wave_open()
{
    static char    *channel_mappings[] = {NULL,"mono","stereo"};
    unsigned short  wFormatTag;
    unsigned long   dAvgBytesPerSec;
    unsigned short  wBlockAlign;
    long            filesize;
    long            header_size;
    unsigned long	data_size;

    //if((config.wave.file = fopen(config.infile,"rb")) == NULL) ERROR("Unable to open file");

    if(!checkString(config.wave.file,"RIFF")) ERROR("Input not a MS-RIFF file");
    filesize = Read32BitsLowHigh(config.wave.file); /* complete wave chunk size */
    fprintf(stderr, "Microsoft RIFF, ");

    if(!checkString(config.wave.file,"WAVE")) ERROR("Input not WAVE audio");
    fprintf(stderr, "WAVE audio, ");

/* WAVE FMT format chunk */
    if(!checkString(config.wave.file,"fmt "))  ERROR("Can't find format chunk");
/* my total header size calculations don't work, so this is bogus... */
    header_size = Read32BitsLowHigh(config.wave.file); /* chunk size */

    wFormatTag = Read16BitsLowHigh(config.wave.file);
    if(wFormatTag!=0x0001) ERROR("Unknown WAVE format");
    config.wave.type = WAVE_RIFF_PCM;

    config.wave.channels   = Read16BitsLowHigh(config.wave.file);
    config.wave.samplerate = Read32BitsLowHigh(config.wave.file);
    dAvgBytesPerSec        = Read32BitsLowHigh(config.wave.file);
    wBlockAlign            = Read16BitsLowHigh(config.wave.file);

/* PCM specific */
    if(config.wave.channels>2) ERROR("More than 2 channels\n");
   fprintf(stderr, "PCM, %s %ldHz ", 
           channel_mappings[config.wave.channels], config.wave.samplerate);
    config.wave.bits       = Read16BitsLowHigh(config.wave.file);
    if(config.wave.bits!=16) ERROR("NOT 16 Bit !!!\n");
    fprintf(stderr, "%dbit, ", config.wave.bits);

    if(!checkString(config.wave.file,"data")) ERROR("Can't find data chunk");

	data_size = Read32BitsLowHigh(config.wave.file);
    header_size = ftell(config.wave.file);
    fseek(config.wave.file, 0, SEEK_END);
    filesize = ftell(config.wave.file);
    fseek(config.wave.file, header_size, SEEK_SET);

    config.wave.total_samples = (filesize-header_size)/(2*config.wave.channels);
    config.wave.length = config.wave.total_samples/config.wave.samplerate;
    fprintf(stderr, "Length: %2ld:%2ld:%2ld\n", (config.wave.length/3600), 
                                       (config.wave.length/60)%60,
                                       (config.wave.length%60));
    return true;
}

static int read_samples(short *sample_buffer,
                        int    frame_size)
{
    int samples_read;

    switch(config.wave.type)
    {
        default          :
            ERROR("[read_samples], wave filetype not supported");

        case WAVE_RIFF_PCM :
            samples_read = fread(sample_buffer,sizeof(short),frame_size, config.wave.file);
    
            /* Microsoft PCM Samples are little-endian, */
            /* we must swap if this is a big-endian machine */

           if(config.byte_order==order_bigEndian) 
               SwapBytesInWords(sample_buffer,samples_read);

           if(samples_read<frame_size && samples_read>0) 
           { /* Pad sample with zero's */
               while(samples_read<frame_size) sample_buffer[samples_read++] = 0;
           }

           break;
    }
    return samples_read;
}


int wave_get(short buffer[2][samp_per_frame])
/* expects an interleaved 16bit pcm stream from read_samples, which it */
/* de-interleaves into buffer                                          */
{
    static short temp_buf[2304];
    int          samples_read;
    int          j;

    switch(config.mpeg.mode)
    {
        case MODE_MONO  :
            samples_read = read_samples(temp_buf,config.mpeg.samples_per_frame);
            for(j=0;j<samp_per_frame;j++)
            {
                buffer[0][j] = temp_buf[j];
                buffer[1][j] = 0;
            }
            break;
		default:
            samples_read = read_samples(temp_buf,config.mpeg.samples_per_frame<<1);
            for(j=0;j<samp_per_frame;j++)
            {
                buffer[0][j] = temp_buf[2*j];
                buffer[1][j] = temp_buf[2*j+1];
            }

    }
    return samples_read;
}
 
