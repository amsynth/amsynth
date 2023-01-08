
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1

#include "juce_x11.h"

#include <juce_gui_basics/juce_gui_basics.h>

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
