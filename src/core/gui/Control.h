/*
 *  Control.h
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

#include "core/synth/Parameter.h"
#include "LayoutDescription.h"

#include <juce_gui_basics/juce_gui_basics.h>

class Control : public juce::Component, protected UpdateListener
{
public:
	Control(Parameter &p, juce::Image image, const LayoutDescription::Resource &r);
	~Control();

	Parameter &parameter;
	bool isPlugin {false};

	void showPopupMenu();

protected:
	void mouseDown(const juce::MouseEvent &event) override;
	void mouseDoubleClick(const juce::MouseEvent &event) override;
	void paint(juce::Graphics &g) override;
	void UpdateParameter(Param param, float controlValue) override;
	void repaintFromAnyThread();

private:
	std::atomic_int frame_;
	juce::Image image_;
	int width_, height_;
	int frames_;
};
