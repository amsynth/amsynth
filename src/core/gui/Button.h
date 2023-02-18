/*
 *  Button.h
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

class Button : public Control
{
public:
	Button(Parameter &parameter, juce::Image image, const LayoutDescription::Resource &r)
	: Control(parameter, std::move(image), r) {}

protected:
	void mouseDown(const juce::MouseEvent &event) override
	{
		if (event.mods.isPopupMenu()) {
			showPopupMenu();
			return;
		}
		parameter.willChange();
		parameter.setNormalisedValue(parameter.getNormalisedValue() > 0.f ? 0.f : 1.f);
	}
};
