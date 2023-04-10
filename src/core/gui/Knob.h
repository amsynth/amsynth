/*
 *  Knob.h
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

class Knob : public Control
{
public:
	class Label : public juce::Component
	{
	public:
		Label(juce::Component *parent);

		int yInset {0};

		void show(juce::Component *control, juce::String text);
		void hide();

	private:
		void paint(juce::Graphics &graphics);

		juce::Component *parent_{nullptr};
		juce::Component *control_{nullptr};
		juce::String text_;
	};

	Knob(Parameter &parameter, juce::Image image, const LayoutDescription::Resource &r, Label *label);

protected:
	void mouseEnter(const juce::MouseEvent &event) override;
	void mouseExit(const juce::MouseEvent &event) override;
	void leftMouseDown(const juce::MouseEvent &event) override;
	void mouseDrag(const juce::MouseEvent &event) override;
	void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

	juce::String getLabelText();

private:
	float referenceVal_{0.f};
	int referenceY_{0};
	Label *label_;
};
