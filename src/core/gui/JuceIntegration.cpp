/*
 *  JuceIntegration.cpp
 *
 *  Copyright (c) 2023 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "JuceIntegration.h"

#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1

#include "juce_gui_basics/juce_gui_basics.h"

#if JUCE_LINUX || JUCE_BSD
#include "juce_audio_plugin_client/utility/juce_LinuxMessageThread.h"
#endif

#if JUCE_WINDOWS
#include <Windows.h>
#endif

static int numInstances;

JuceIntegration::JuceIntegration() {
	if (numInstances++ == 0) {
		juce::initialiseJuce_GUI();
	}
}

JuceIntegration::~JuceIntegration() {
	if (--numInstances == 0) {
		juce::shutdownJuce_GUI();
	}
}

double JuceIntegration::getPluginScaleFactor() {
#if JUCE_LINUX || JUCE_BSD
	const char *scale = getenv("GDK_SCALE");
	if (scale) {
		return (double)atoi(scale);
	}

	auto x11 = juce::XWindowSystem::getInstance();
	auto setting = x11->getXSettings()->getSetting("Gdk/WindowScalingFactor");
	if (setting.isValid() && setting.integerValue > 1) {
		return (double)setting.integerValue;
	}

	const char *xres = XGetDefault(x11->getDisplay(), "Xft", "dpi");
	if (xres && atoi(xres) > 96) {
		return atoi(xres) / 96.0;
	}
#elif JUCE_WINDOWS
	int dpi = GetDeviceCaps(GetDC(NULL), LOGPIXELSX);
	return dpi / (double)USER_DEFAULT_SCREEN_DPI;
#endif
	return 1.0;
}

void JuceIntegration::idle() {
#if JUCE_LINUX || JUCE_BSD
	juce::MessageManager::getInstance()->setCurrentThreadAsMessageThread();
	for (;;)
		if (!juce::dispatchNextMessageOnSystemQueue(true))
			return;
#endif
}
