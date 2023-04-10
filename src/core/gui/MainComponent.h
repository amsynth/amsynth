/*
 *  MainComponent.h
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

#ifndef amsynth_MainComponent_h
#define amsynth_MainComponent_h

#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent final : public juce::Component, public juce::ApplicationCommandTarget {
public:
	MainComponent(class PresetController *presetController, class MidiController *midiController = nullptr);
	~MainComponent();

	std::function<void(const char *)> loadTuningKbm;
	std::function<void(const char *)> loadTuningScl;

	ApplicationCommandTarget *getNextCommandTarget() final {return nullptr;}
	void getAllCommands(juce::Array<juce::CommandID> &commands) final;
	void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo &result) final;
	bool perform(const InvocationInfo &info) final;

	void paint(juce::Graphics &g) final;
	void resized() final;

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
	juce::ApplicationCommandManager commandManager;
};

#endif //amsynth_MainComponent_h
