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
