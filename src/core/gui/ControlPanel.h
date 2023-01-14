/*
 *  ControlPanel.h
 *
 *  Copyright (c) 2022 - 2023 Nick Dowell
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

#ifndef _CONTROLPANEL_H
#define _CONTROLPANEL_H

#include "core/synth/PresetController.h"

#include <juce_gui_basics/juce_gui_basics.h>

class ControlPanel final : public juce::Component
{
public:
	explicit ControlPanel(PresetController *presetController);

	~ControlPanel() noexcept final;
	
	static std::string skinsDirectory;

	std::function<void(const char *)> loadTuningKbm;
	std::function<void(const char *)> loadTuningScl;

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

#endif //_CONTROLPANEL_H
