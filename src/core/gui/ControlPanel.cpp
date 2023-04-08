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
#include "core/synth/Parameter.h"

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

class MouseDownControl : public juce::Component {
public:
	using Handler = std::function<void(const juce::MouseEvent&)>;
	explicit MouseDownControl(Handler handler) : handler_(std::move(handler)) {}
	void mouseDown(const juce::MouseEvent &event) override { handler_(event); }
	Handler handler_;
};

struct ControlPanel::Impl final
{
	Impl(ControlPanel *controlPanel, PresetController *presetController)
	: controlPanel_(controlPanel)
	, presetController_(presetController)
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
				controlPanel->addAndMakeVisible(component.get());
				components_.push_back(std::move(component));
			}
		}
	}
	
	void showPopupMenu() {
		auto presetMenu = juce::PopupMenu();
		for (auto &bank : PresetController::getPresetBanks()) {
			char text[64];
			auto bankMenu = juce::PopupMenu();
			for (int i = 0; i < PresetController::kNumPresets; i++) {
				snprintf(text, sizeof text, "%d: %s", i, bank.presets[i].getName().c_str());
				bankMenu.addItem(text, [this, &bank, i] {
					presetController_->loadPresets(bank.file_path.c_str());
					presetController_->selectPreset(i);
				});
			}
			snprintf(text, sizeof text, "[%s] %s", bank.read_only ? gettext("F") : gettext("U"), bank.name.c_str());
			presetMenu.addSubMenu(text, bankMenu);
		}
		
		auto fileMenu = juce::PopupMenu();
		if (controlPanel_->loadTuningScl) {
			fileMenu.addItem(gettext("Open Alternate Tuning File..."), [this] {
				openFile(gettext("Open Scala (.scl) alternate tuning file"),
						 "*.scl", controlPanel_->loadTuningScl);
			});
		}
		if (controlPanel_->loadTuningKbm) {
			fileMenu.addItem(gettext("Open Alternate Keyboard Map..."), [this] {
				openFile(gettext("Open alternate keyboard map (Scala .kbm format)"),
						 "*.kbm", controlPanel_->loadTuningKbm);
			});
		}
		if (controlPanel_->loadTuningKbm || controlPanel_->loadTuningScl) {
			fileMenu.addItem(gettext("Reset All Tuning Settings to Default"), [this] {
				if (controlPanel_->loadTuningKbm)
					controlPanel_->loadTuningKbm(nullptr);
				if (controlPanel_->loadTuningScl)
					controlPanel_->loadTuningScl(nullptr);
			});
		}

		auto menu = juce::PopupMenu();
		menu.addSubMenu(gettext("File"), fileMenu);
		menu.addSubMenu(gettext("Preset"), presetMenu);
#ifdef PACKAGE_VERSION
		menu.addSeparator();
		menu.addItem(1, "v" PACKAGE_VERSION, false);
#endif
		menu.showMenuAsync(juce::PopupMenu::Options());
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
	PresetController *presetController_;
	Knob::Label label_;
};

ControlPanel::ControlPanel(PresetController *presetController)
: impl_(std::make_unique<Impl>(this, presetController)) {}

ControlPanel::~ControlPanel() noexcept = default;

#ifdef PKGDATADIR
std::string ControlPanel::skinsDirectory {PKGDATADIR "/skins"};
#else
std::string ControlPanel::skinsDirectory;
#endif
