// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain

#ifndef _denormals_
#define _denormals_

#define undenormalise(sample) if(((*(unsigned int*)(void *)&sample)&0x7f800000)==0) sample=0.0f

#define nuke_denormals(value) if(((*(unsigned int *)&(value))&0x7f800000)<0x08000000) value=0.0f

#endif//_denormals_

//ends
