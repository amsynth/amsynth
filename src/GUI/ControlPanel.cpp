/*
 *  ControlPanel.cpp
 *
 *  Copyright (c) 2022 Nick Dowell
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "ControlPanel.h"

#include "LayoutDescription.h"
#include "src/controls.h"
#include "src/Parameter.h"

#include <cassert>

ControlPanel::ControlPanel(PresetController *presetController)
: presetController_(presetController)
{
	setOpaque(true);

	auto skin = juce::File(PKGDATADIR "/skins/default");
	auto layout = LayoutDescription(skin.getChildFile("layout.ini").getFullPathName().toStdString());
	background_ = juce::Drawable::createFromImageFile(skin.getChildFile(layout.background));
	setSize(background_->getWidth(), background_->getHeight());
	background_->setOpaque(true);
	addAndMakeVisible(background_.get());

	for (int i = 0; i < kAmsynthParameterCount; i++) {
		auto &parameter = presetController_->getCurrentPreset().getParameter(i);

		const auto &control = layout.controls.at(parameter.getName());
		juce::Component *component = nullptr;
		if (control.type == "button") {
			auto button = new juce::ToggleButton();
			button->addListener(this);
			component = button;
		}
		if (control.type == "knob") {
			auto slider = new juce::Slider(
					juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
					juce::Slider::TextEntryBoxPosition::NoTextBox
			);
			slider->setRange(parameter.getMin(), parameter.getMax());
			slider->setDoubleClickReturnValue(true, parameter.getDefault());
			char tmp[4];
			if (parameter_get_display(i, 0.f, tmp, sizeof tmp)) {
				slider->setPopupDisplayEnabled(true, true, this);
				slider->textFromValueFunction = [i](double value){
					char buffer[64] = "";
					parameter_get_display(i, (float)value, buffer, sizeof buffer);
					return juce::String(buffer);
				};
			}
			slider->addListener(this);
			component = slider;
		}
		if (control.type == "popup") {
		}
		if (component) {
			component->setSize(control.resource.width, control.resource.height);
			component->setTopLeftPosition(control.x, control.y);
			component->setOpaque(true);
			addAndMakeVisible(component);
		}
		components_.push_back(component);
		parameter.addUpdateListener(this);
	}
}

ControlPanel::~ControlPanel() noexcept
{
	for (int i = 0; i < kAmsynthParameterCount; i++) {
		presetController_->getCurrentPreset().getParameter(i).removeUpdateListener(this);
	}
	std::for_each(components_.begin(), components_.end(), [](auto component){
		delete component;
	});
}

void
ControlPanel::buttonClicked(juce::Button *button)
{
}

void
ControlPanel::sliderValueChanged(juce::Slider *slider)
{
	auto pos = std::find(components_.begin(), components_.end(), slider) - components_.begin();
	presetController_->getCurrentPreset().getParameter((int)pos).setValue((float)slider->getValue());
}

void ControlPanel::UpdateParameter(Param param, float controlValue)
{
	assert(param >= 0);
	assert(param < kAmsynthParameterCount);
	(void)controlValue;

	juce::MessageManager::callAsync([=](){
		if (auto slider = dynamic_cast<juce::Slider *>(components_[param])) {
			auto &parameter = presetController_->getCurrentPreset().getParameter(param);
			slider->setValue(parameter.getValue(), juce::NotificationType::dontSendNotification);
		}
	});
}
