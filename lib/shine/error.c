/*
 *  error.c
 *  shine
 *
 *  Created by kikko on 3/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "error.h"

void errorToFlash(const char* message)
{	
	AS3_Val trace = AS3_NSGetS(NULL, "trace");
	AS3_Val params = AS3_Array("StrType", message);
	AS3_Release(AS3_Call(trace, AS3_Undefined(), params));
}
