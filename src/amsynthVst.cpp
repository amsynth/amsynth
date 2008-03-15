/*
 *  amsynthVst.cpp
 *  amsynth
 *
 *  Created by Nicolas Dowell on 15/03/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "amsynthVst.h"

extern AudioEffect * createEffectInstance( audioMasterCallback callback )
{
	return new AMSynthVst( callback );
}

#if linux

extern "C" AEffect * main_plugin( audioMasterCallback audioMaster ) asm ("main");

#define main main_plugin

extern "C" AEffect * main( audioMasterCallback audioMaster )
{ 
	AudioEffect * effect = createEffectInstance( audioMaster );
	return effect ? effect->getAeffect() : NULL;
}

#endif

