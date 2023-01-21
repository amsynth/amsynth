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

#include "LayoutDescription.h"
#include "core/Configuration.h"
#include "core/controls.h"
#include "core/gettext.h"
#include "core/synth/Parameter.h"

#include <utility>
#include <vector>

void modal_midi_learn(Param);

class SkinnedParameterControl : public juce::Component, protected UpdateListener
{
public:
	SkinnedParameterControl(Parameter &p, juce::Image image, const LayoutDescription::Resource &r)
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

	~SkinnedParameterControl() override
	{
		parameter.removeUpdateListener(this);
	}

	Parameter &parameter;

	bool isPlugin {false};

	void showPopupMenu() const
	{
		if (isPlugin) {
			return;
		}
		auto menu = juce::PopupMenu();
		auto p = parameter.getId();
		menu.addItem(gettext("Assign MIDI Controller..."), true, false, [p](){
			modal_midi_learn(p);
		});
		bool ignored = Preset::shouldIgnoreParameter(p);
		menu.addItem(gettext("Ignore Preset Value"), true, ignored, [ignored, p](){
			Preset::setShouldIgnoreParameter(p, !ignored);
			Configuration &config = Configuration::get();
			config.ignored_parameters = Preset::getIgnoredParameterNames();
			config.save();
		});
		menu.showMenuAsync(juce::PopupMenu::Options());
	}

protected:
	void mouseDown(const juce::MouseEvent &event) override
	{
		if (event.mods.isPopupMenu()) {
			showPopupMenu();
		}
	}

	void mouseDoubleClick(const juce::MouseEvent &event) override
	{
		// TODO: Reset to "default" value
	}

	void paint(juce::Graphics &g) override
	{
		int x = 0, y = 0;
		if (image_.getHeight() == height_) {
			x = frame_ * width_;
		} else {
			y = frame_ * height_;
		}
		g.drawImage(image_, 0, 0, width_, height_, x, y, width_, height_, false);
	}

	void UpdateParameter(Param param, float controlValue) override
	{
		int frame = int(float(frames_ - 1) * parameter.getNormalisedValue());
		if (frame_ == frame) {
			return;
		}
		frame_ = frame;
		repaintFromAnyThread();
	}

	void repaintFromAnyThread()
	{
		auto mm = juce::MessageManager::getInstanceWithoutCreating();
		if (mm && mm->isThisTheMessageThread()) {
			repaint();
		} else {
			juce::MessageManager::callAsync([this]() {
				repaint();
			});
		}
	}

private:
	std::atomic_int frame_;
	juce::Image image_;
	int width_, height_;
	int frames_;
};

class Knob : public SkinnedParameterControl
{
public:
	Knob(Parameter &parameter, juce::Image image, const LayoutDescription::Resource &r)
	: SkinnedParameterControl(parameter, std::move(image), r)
	{}

protected:
	void mouseDown(const juce::MouseEvent &event) override
	{
		if (event.mods.isPopupMenu()) {
			showPopupMenu();
			return;
		}
		referenceVal_ = parameter.getNormalisedValue();
		referenceY_ = event.y;
		parameter.willChange();
	}

	void mouseDrag(const juce::MouseEvent &event) override
	{
		SkinnedParameterControl::mouseDrag(event);
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
			referenceVal_ = newVal;
			referenceY_ = event.y;
		}
	}

	void mouseWheelMove(const juce::MouseEvent &event,
						const juce::MouseWheelDetails &wheel) override
	{
		if (event.mouseWasDraggedSinceMouseDown()) {
			return;
		}
		auto delta = wheel.deltaY / 2.f
				/ (event.mods.isCtrlDown() ? 4.f : 1.f)
				/ (event.mods.isShiftDown() ? 4.f : 1.f);
		parameter.willChange();
		parameter.setNormalisedValue(parameter.getNormalisedValue() + delta);
	}

private:
	float referenceVal_{0.f};
	int referenceY_{0};
};

class Button : public SkinnedParameterControl
{
public:
	Button(Parameter &parameter, juce::Image image, const LayoutDescription::Resource &r)
	: SkinnedParameterControl(parameter, std::move(image), r) {}

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

class Popup : public SkinnedParameterControl
{
public:
	Popup(Parameter &parameter, juce::Image image, const LayoutDescription::Resource &r)
	: SkinnedParameterControl(parameter, std::move(image), r) {}

protected:

	void mouseDown(const juce::MouseEvent &event) override
	{
		if (event.mods.isPopupMenu()) {
			showPopupMenu();
			return;
		}
		auto strings = parameter_get_value_strings(parameter.getId());
		auto menu = juce::PopupMenu();
		for (int i = 0; i <= parameter.getSteps(); i++) {
			auto value = parameter.getMin() + parameter.getStep() * float(i);
			auto isTicked = parameter.getValue() == value;
			menu.addItem(strings[i], true, isTicked, [&parameter = parameter, value](){
				parameter.willChange();
				parameter.setValue(value);
			});
		}
		menu.showMenuAsync(juce::PopupMenu::Options());
	}
};

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
	Impl(ControlPanel *controlPanel, PresetController *presetController, bool isPlugin)
	: controlPanel_(controlPanel), presetController_(presetController)
	{
		auto skin = Skin(ControlPanel::skinsDirectory + "/default");
//		auto skin = Skin(ControlPanel::skinsDirectory + "/Etna");

		auto background = juce::Drawable::createFromImageFile(skin.getBackground());
		background->setOpaque(true);
		controlPanel->setOpaque(true);
		controlPanel->addAndMakeVisible(background.get());
		controlPanel->setSize(background->getWidth(), background->getHeight());
		components_.push_back(std::move(background));

		if (isPlugin) {
			auto clickArea = std::make_unique<MouseDownControl>([this](auto &event) {
				if (event.mods.isPopupMenu()) {
					showPopupMenu();
				}
			});
			clickArea->setSize(controlPanel->getWidth(), controlPanel->getHeight());
			controlPanel->addAndMakeVisible(clickArea.get());
			components_.push_back(std::move(clickArea));
		}

		for (int i = 0; i < kAmsynthParameterCount; i++) {
			auto &parameter = presetController_->getCurrentPreset().getParameter(i);

			const auto &control = skin.layout.controls.at(parameter.getName());
			const auto &resource = control.resource;
			auto image = skin.getImage(resource);

			std::unique_ptr<SkinnedParameterControl> component;
			if (control.type == "button") {
				component = std::make_unique<Button>(parameter, image, resource);
			}
			if (control.type == "knob") {
				component = std::make_unique<Knob>(parameter, image, resource);
			}
			if (control.type == "popup") {
				component = std::make_unique<Popup>(parameter, image, resource);
			}
			if (component) {
				component->isPlugin = isPlugin;
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
				bankMenu.addItem(text, [this, &bank, i]() {
					presetController_->loadPresets(bank.file_path.c_str());
					presetController_->selectPreset(i);
				});
			}
			snprintf(text, sizeof text, "[%s] %s", bank.read_only ? gettext("F") : gettext("U"), bank.name.c_str());
			presetMenu.addSubMenu(text, bankMenu);
		}
		
		auto fileMenu = juce::PopupMenu();
		if (controlPanel_->loadTuningScl) {
			fileMenu.addItem(gettext("Open Alternate Tuning File..."), [this]() {
				openFile(gettext("Open Scala (.scl) alternate tuning file"),
						 "*.scl", controlPanel_->loadTuningScl);
			});
		}
		if (controlPanel_->loadTuningKbm) {
			fileMenu.addItem(gettext("Open Alternate Keyboard Map..."), [this]() {
				openFile(gettext("Open alternate keyboard map (Scala .kbm format)"),
						 "*.kbm", controlPanel_->loadTuningKbm);
			});
		}
		if (controlPanel_->loadTuningKbm || controlPanel_->loadTuningScl) {
			fileMenu.addItem(gettext("Reset All Tuning Settings to Default"), [this]() {
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
		chooser->launchAsync(juce::FileBrowserComponent::openMode, [chooser, handler](const auto &ignored) {
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
};

ControlPanel::ControlPanel(PresetController *presetController, bool isPlugin)
: impl_(std::make_unique<Impl>(this, presetController, isPlugin)) {}

ControlPanel::~ControlPanel() noexcept = default;

#ifdef PKGDATADIR
std::string ControlPanel::skinsDirectory {PKGDATADIR "/skins"};
#else
std::string ControlPanel::skinsDirectory;
#endif
