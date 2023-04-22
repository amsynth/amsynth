/*
 *  Control.cpp
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "Control.h"

#include "core/Configuration.h"
#include "core/synth/Preset.h"

extern void modal_midi_learn(Param);

Control::Control(Parameter &p, juce::Image image, const LayoutDescription::Resource &r)
: parameter(p)
, frame_(0)
, image_(std::move(image))
, width_(r.width)
, height_(r.height)
, frames_(r.frames)
{
	setSize(width_, height_);
	parameter.addUpdateListener(this);
}

Control::~Control()
{
	parameter.removeUpdateListener(this);
}


void Control::showPopupMenu()
{
	if (isPlugin) {
		return;
	}
	auto menu = juce::PopupMenu();
	auto p = parameter.getId();
	menu.addItem(gettext("Assign MIDI Controller..."), true, false, [p] {
		modal_midi_learn(p);
	});
	bool ignored = Preset::shouldIgnoreParameter(p);
	menu.addItem(gettext("Ignore Preset Value"), true, ignored, [ignored, p] {
		Preset::setShouldIgnoreParameter(p, !ignored);
		Configuration &config = Configuration::get();
		config.ignored_parameters = Preset::getIgnoredParameterNames();
		config.save();
	});
	menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
}

void Control::mouseDown(const juce::MouseEvent &event)
{
	if (event.mods.isPopupMenu()) {
		showPopupMenu();
	}
}

void Control::mouseDoubleClick(const juce::MouseEvent &event)
{
	parameter.setValue(parameter.getDefault());
}

void Control::paint(juce::Graphics &g)
{
	int x = 0, y = 0;
	if (image_.getHeight() == height_) {
		x = frame_ * width_;
	} else {
		y = frame_ * height_;
	}
	g.drawImage(image_, 0, 0, width_, height_, x, y, width_, height_, false);
}

void Control::UpdateParameter(Param param, float controlValue)
{
	int frame = int(float(frames_ - 1) * parameter.getNormalisedValue());
	if (frame_ == frame) {
		return;
	}
	frame_ = frame;
	repaintFromAnyThread();
}

void Control::repaintFromAnyThread()
{
	auto mm = juce::MessageManager::getInstanceWithoutCreating();
	if (mm && mm->isThisTheMessageThread()) {
		repaint();
	} else {
		juce::MessageManager::callAsync([this] {
			repaint();
		});
	}
}
