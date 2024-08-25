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

#if JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS

namespace juce {
// Implemented in juce_linux_Messaging.cpp / juce_win32_Messaging.cpp
extern bool dispatchNextMessageOnSystemQueue(bool returnIfNoPendingMessages);
} // namespace juce

float getGlobalScaleFactor() {
	auto gdkScale = getenv("GDK_SCALE");
	if (gdkScale) {
		return (float)atoi(gdkScale);
	}

	auto *xSettings = juce::XWindowSystem::getInstance()->getXSettings();
	if (xSettings) {
		auto windowScalingFactorSetting = xSettings->getSetting(juce::XWindowSystem::getWindowScalingFactorSettingName());
		if (windowScalingFactorSetting.isValid() && windowScalingFactorSetting.integerValue > 0)
			return (float)windowScalingFactorSetting.integerValue;
	}

	return 1.f;
}

#endif

static int numInstances;

JuceIntegration::JuceIntegration() {
	if (numInstances++ == 0) {
		juce::initialiseJuce_GUI();
#if JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS
		auto scaleFactor = getGlobalScaleFactor();
		juce::Desktop::getInstance().setGlobalScaleFactor(scaleFactor);
#endif
	}
}

JuceIntegration::~JuceIntegration() {
	if (--numInstances == 0) {
		juce::shutdownJuce_GUI();
	}
}

void JuceIntegration::idle() {
#if JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS
	juce::dispatchNextMessageOnSystemQueue(true);
#endif
}
