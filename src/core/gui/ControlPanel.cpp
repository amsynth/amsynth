/*
 *  ControlPanel.cpp
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "ControlPanel.h"

#include "Button.h"
#include "Knob.h"
#include "LayoutDescription.h"
#include "Popup.h"
#include "core/Configuration.h"
#include "core/controls.h"
#include "core/gettext.h"
#include "core/synth/MidiController.h"
#include "core/synth/PresetController.h"

#include <seq24/controllers.h>
#include <utility>
#include <vector>

class Skin
{
public:
	explicit Skin(const std::string& path)
	: directory(path)
	, layout(directory.getChildFile("layout.ini").getFullPathName().toStdString())
	{}

	juce::File directory;
	LayoutDescription layout;

	juce::File getBackground() const
	{
		return directory.getChildFile(layout.background);
	}

	juce::Image getImage(const LayoutDescription::Resource& resource)
	{
		if (images_.find(resource.file) != images_.end()) {
			return images_.at(resource.file);
		} else {
			auto path = directory.getChildFile(resource.file).getFullPathName();
			return images_[resource.file] = juce::ImageFileFormat::loadFrom(path);
		}
	}

private:
	std::unordered_map<std::string, juce::Image> images_;
};

struct ControlPanel::Impl final : juce::MouseListener
{
	Impl(ControlPanel *controlPanel, MidiController *midiController, PresetController *presetController)
	: controlPanel_(controlPanel)
	, presetController_(presetController)
	, midiController_(midiController)
	, label_(controlPanel)
	{
		auto skin = Skin(ControlPanel::skinsDirectory + "/default");
		label_.yInset = 6;
//		auto skin = Skin(ControlPanel::skinsDirectory + "/Etna");
//		label_.yOffset = 0;

		auto background = juce::Drawable::createFromImageFile(skin.getBackground());
		background->setOpaque(true);
		controlPanel->setOpaque(true);
		controlPanel->addAndMakeVisible(background.get());
		controlPanel->setSize(background->getWidth(), background->getHeight());
		components_.push_back(std::move(background));

		for (int i = 0; i < kAmsynthParameterCount; i++) {
			auto &parameter = presetController_->getCurrentPreset().getParameter(i);

			const auto &control = skin.layout.controls.at(parameter.getName());
			const auto &resource = control.resource;
			auto image = skin.getImage(resource);

			std::unique_ptr<Control> component;
			if (control.type == "button") {
				component = std::make_unique<Button>(parameter, image, resource);
			}
			if (control.type == "knob") {
				component = std::make_unique<Knob>(parameter, image, resource, &label_);
			}
			if (control.type == "popup") {
				component = std::make_unique<Popup>(parameter, image, resource);
			}
			if (component) {
				component->setTopLeftPosition(control.x, control.y);
				component->addMouseListener(this, false);
				controlPanel->addAndMakeVisible(component.get());
				components_.push_back(std::move(component));
			}
		}
	}

	void mouseDown(const juce::MouseEvent& event) final {
		if (event.mods.isPopupMenu() && midiController_) {
			showPopupMenu(dynamic_cast<Control *>(event.originalComponent));
		}
	}

	void showPopupMenu(Control *control) {
		auto paramId = control->parameter.getId();
		juce::PopupMenu menu;

		juce::PopupMenu ccSubmenu;
		int cc = midiController_->getControllerForParameter(paramId);
		for (int i = 0; i < 128; i++) {
			ccSubmenu.addItem(c_controller_names[i], true, i == cc, [this, paramId, i] {
				midiController_->setControllerForParameter(paramId, i);
			});
		}
		ccSubmenu.addItem(gettext("None"), [this, paramId] {
			midiController_->setControllerForParameter(paramId, -1);
		});
		menu.addSubMenu(gettext("Assign MIDI CC"), ccSubmenu, true, nullptr, cc != -1);

		bool isIgnored = Preset::shouldIgnoreParameter(paramId);
		menu.addItem(gettext("Ignore Preset Value"), true, isIgnored, [isIgnored, paramId] {
			Preset::setShouldIgnoreParameter(paramId, !isIgnored);
			Configuration &config = Configuration::get();
			config.ignored_parameters = Preset::getIgnoredParameterNames();
			config.save();
		});
		menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(control));
	}
	
	static void openFile(const char *title, const char *filters, const std::function<void(const char *)> &handler) {
		auto cwd = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
		auto chooser = new juce::FileChooser(title, cwd, filters);
		chooser->launchAsync(juce::FileBrowserComponent::openMode, [chooser, handler] (const auto &ignored) {
			auto results = chooser->getResults();
			if (results.isEmpty())
				return;
			handler(results[0].getFullPathName().toRawUTF8());
			delete chooser;
		});
	}

	ControlPanel *controlPanel_;
	std::vector<std::unique_ptr<juce::Component>> components_;
	MidiController *midiController_;
	PresetController *presetController_;
	Knob::Label label_;
};

ControlPanel::ControlPanel(MidiController *midiController, PresetController *presetController)
: impl_(std::make_unique<Impl>(this, midiController, presetController)) {}

ControlPanel::~ControlPanel() noexcept = default;

#ifdef PKGDATADIR
std::string ControlPanel::skinsDirectory {PKGDATADIR "/skins"};
#else
std::string ControlPanel::skinsDirectory;
#endif
