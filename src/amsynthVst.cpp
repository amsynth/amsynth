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

#pragma mark - OS X specific stuff

#if (__APPLE__ && __MACH__)

extern "C" {
#include <mach-o/dyld.h>
#include <mach-o/ldsyms.h>
}
#include <CoreFoundation/CFBundle.h>

CFBundleRef g_BundleRef = NULL;

// this code is originally from VSTGUI

static CFBundleRef _CFXBundleCreateFromImageName (CFAllocatorRef allocator, const char* image_name)
{
	CFURLRef myBundleExecutableURL = CFURLCreateFromFileSystemRepresentation (allocator, (const unsigned char*)image_name, strlen (image_name), false);
	if (myBundleExecutableURL == 0)
		return 0;
	
	CFURLRef myBundleContentsMacOSURL = CFURLCreateCopyDeletingLastPathComponent (allocator, myBundleExecutableURL); // Delete Versions/Current/Executable
	CFRelease (myBundleExecutableURL);
	if (myBundleContentsMacOSURL == 0)
		return 0;
	
	CFURLRef myBundleContentsURL = CFURLCreateCopyDeletingLastPathComponent (allocator, myBundleContentsMacOSURL); // Delete Current
	CFRelease (myBundleContentsMacOSURL);
	if (myBundleContentsURL == 0)
		return 0;
	
	CFURLRef theBundleURL = CFURLCreateCopyDeletingLastPathComponent (allocator, myBundleContentsURL); // Delete Versions
	CFRelease (myBundleContentsURL);
	if (theBundleURL == 0)
		return 0;
	
	CFBundleRef result = CFBundleCreate (allocator, theBundleURL);
	CFRelease (theBundleURL);
	
	return result;
}

void InitMachOLibrary () __attribute__ ((constructor));
void InitMachOLibrary ()
{
	const mach_header* header = &_mh_bundle_header;
	
	const char* imagename = 0;
	/* determine the image name, TODO: ther have to be a better way */
	int cnt = _dyld_image_count();
	for (int idx1 = 1; idx1 < cnt; idx1++) 
	{
		if (_dyld_get_image_header(idx1) == header)
		{
			imagename = _dyld_get_image_name(idx1);
			break;
		}
	}
	if (imagename == 0)
		return;
	/* get the bundle of a header, TODO: ther have to be a better way */
	g_BundleRef = _CFXBundleCreateFromImageName (NULL, imagename);
}

bool AMSynthVst::getPresetsFilename(char * filename, size_t maxLen)
{
	CFURLRef resURL = CFBundleCopyResourceURL(g_BundleRef, CFSTR("presets"), NULL, NULL);
	if (!resURL)
		return false;
	
	Boolean result = CFURLGetFileSystemRepresentation(resURL, FALSE, (UInt8 *)filename, maxLen);
	CFRelease(resURL);
	return result;
}

#endif
