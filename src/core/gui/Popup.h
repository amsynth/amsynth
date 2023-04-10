/*
 *  Popup.h
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

#pragma once

#include "Control.h"

class Popup : public Control
{
public:
	Popup(Parameter &parameter, juce::Image image, const LayoutDescription::Resource &r)
	: Control(parameter, std::move(image), r) {}

protected:

	void leftMouseDown(const juce::MouseEvent &event) override
	{
		auto strings = parameter_get_value_strings(parameter.getId());
		auto menu = juce::PopupMenu();
		for (int i = 0; i <= parameter.getSteps(); i++) {
			auto value = parameter.getMin() + parameter.getStep() * float(i);
			auto isTicked = parameter.getValue() == value;
			menu.addItem(strings[i], true, isTicked, [&parameter = parameter, value] {
				parameter.willChange();
				parameter.setValue(value);
			});
		}
		menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
	}
};
