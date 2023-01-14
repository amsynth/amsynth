
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1

#include "juce_x11.h"

#include <juce_gui_basics/juce_gui_basics.h>

#if JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS
namespace juce {
// Implemented in juce_linux_Messaging.cpp / juce_win32_Messaging.cpp
	extern bool dispatchNextMessageOnSystemQueue(bool returnIfNoPendingMessages);
}
#endif

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

void juceInit() {
	static bool once;
	if (once) return; else once = true;
	juce::initialiseJuce_GUI();
	auto scaleFactor = getGlobalScaleFactor();
	juce::Desktop::getInstance().setGlobalScaleFactor(scaleFactor);
}

void juceIdle() {
	juce::dispatchNextMessageOnSystemQueue(true);
}
