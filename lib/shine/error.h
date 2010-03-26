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


#ifndef _OUR_ERROR_H
#define _OUR_ERROR_H

#include <stdlib.h>
#include "AS3.h"

//#define ERROR(X) {fprintf(stderr,"[ERROR] %s\n",X);exit(-1);}
#define ERROR(X) { AS3_Release(AS3_CallTS("shineError", flashErrorsRef, "StrType", X)); }

AS3_Val flashErrorsRef;

#endif
