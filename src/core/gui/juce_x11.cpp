
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1

#include "juce_x11.h"

#include <juce_gui_basics/juce_gui_basics.h>

#if JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS

namespace juce {
// Implemented in juce_linux_Messaging.cpp / juce_win32_Messaging.cpp
	extern bool dispatchNextMessageOnSystemQueue(bool returnIfNoPendingMessages);
}

float getGlobalScaleFactor() {
	if (auto scale = getenv("GDK_SCALE")) {
		return (float)atoi(scale);
	}

	if (auto *xSettings = juce::XWindowSystem::getInstance()->getXSettings()) {
		auto windowScalingFactorSetting = xSettings->getSetting(
				juce::XWindowSystem::getWindowScalingFactorSettingName());
		if (windowScalingFactorSetting.isValid() && windowScalingFactorSetting.integerValue > 0)
			return (float)windowScalingFactorSetting.integerValue;
	}

	return 1.f;
}

#endif

static bool isInitialized;

void juceInit() {
	if (isInitialized) return;
	juce::initialiseJuce_GUI();
#if JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS
	auto scaleFactor = getGlobalScaleFactor();
	juce::Desktop::getInstance().setGlobalScaleFactor(scaleFactor);
#endif
	isInitialized = true;
}

void juceIdle() {
	if (!isInitialized)
		return;
#if JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS
	juce::dispatchNextMessageOnSystemQueue(true);
#endif
}
