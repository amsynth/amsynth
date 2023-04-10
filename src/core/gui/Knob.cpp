/*
 *  Knob.cpp
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

#include "Knob.h"

Knob::Knob(Parameter &parameter, juce::Image image, const LayoutDescription::Resource &r, Label *label)
: Control(parameter, std::move(image), r)
, label_(label)
{}

juce::String Knob::getLabelText()
{
	char text[32] = "";
	return parameter_get_display(parameter.getId(), parameter.getValue(), text, sizeof text) ? text : "";
}

void Knob::mouseEnter(const juce::MouseEvent &event)
{
	label_->show(this, getLabelText());
}

void Knob::mouseExit(const juce::MouseEvent &event)
{
	label_->hide();
}

void Knob::leftMouseDown(const juce::MouseEvent &event)
{
	referenceVal_ = parameter.getNormalisedValue();
	referenceY_ = event.y;
	parameter.willChange();
}

void Knob::mouseDrag(const juce::MouseEvent &event)
{
	Control::mouseDrag(event);
	if (event.mods.isPopupMenu()) {
		return;
	}
	float sensitivity;
	if (parameter.getStep() == 0.f) {
		sensitivity = 300
		* (event.mods.isCtrlDown() ? 4.f : 1.f)
		* (event.mods.isShiftDown() ? 4.f : 1.f);
	} else {
		sensitivity = std::min(40 * float(parameter.getSteps()), 480.f);
	}

	auto offset = float(referenceY_ - event.y) / sensitivity;
	auto newVal = referenceVal_ + offset;
	if (newVal != referenceVal_) {
		parameter.setNormalisedValue(referenceVal_ + offset);
		label_->show(this, getLabelText());
		referenceVal_ = newVal;
		referenceY_ = event.y;
	}
}

void Knob::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
	if (event.mouseWasDraggedSinceMouseDown()) {
		return;
	}
	auto delta = (wheel.deltaY / 2.f
				  / (event.mods.isCtrlDown() ? 4.f : 1.f)
				  / (event.mods.isShiftDown() ? 4.f : 1.f));
	parameter.willChange();
	parameter.setNormalisedValue(parameter.getNormalisedValue() + delta);
	label_->show(this, getLabelText());
}

Knob::Label::Label(juce::Component *parent) : parent_(parent)
{
	setAlwaysOnTop(true);
	setOpaque(true);
	setVisible(false);
	parent->addChildComponent(this);
}

void Knob::Label::show(juce::Component *control, juce::String text)
{
	if (control_ && control_ != control) return;
	control_ = control;

	text_ = text;
	if (text.isEmpty())
	{
		setVisible(false);
		return;
	}

	auto ttb = getLookAndFeel().getTooltipBounds(text, control->getPosition(), parent_->getLocalBounds());
	auto width = std::max(control->getWidth(), ttb.getWidth());
	auto inset = (control->getWidth() - width) / 2;
	setBounds(control->getX() + inset, control->getBottom() - yInset, width, ttb.getHeight());
	setVisible(true);
	repaint();
}

void Knob::Label::hide()
{
	setVisible(false);
	control_ = nullptr;
	text_ = "";
}

void Knob::Label::paint(juce::Graphics &graphics)
{
	getLookAndFeel().drawTooltip(graphics, text_, getWidth(), getHeight());
}
