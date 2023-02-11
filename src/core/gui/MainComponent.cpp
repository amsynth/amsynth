/*
 *  MainComponent.cpp
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

#include "MainComponent.h"

#include "ControlPanel.h"
#include "core/gettext.h"

class ToolbarItem : public juce::ToolbarItemComponent {
public:
	ToolbarItem(int itemId, juce::String text, std::function<void(juce::Component *)> onClick)
	: juce::ToolbarItemComponent(itemId, "", true)
	, text_(text) {
		this->onClick = std::bind(onClick, this);
	}

	bool getToolbarItemSizes (int toolbarDepth, bool /*isToolbarVertical*/, int& preferredSize, int& minSize, int& maxSize) override {
		int textWidth = getLookAndFeel().getTooltipBounds(text_, juce::Point<int>(), juce::Rectangle<int>(0, 0, 100, 100)).getWidth();
		preferredSize = minSize = maxSize = std::max(toolbarDepth, textWidth);
		return true;
	}

	void paintButtonArea (juce::Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown) override {
		g.setColour(juce::Colour::fromRGB(255, 255, 255));
		g.drawText(text_, 0, 0, width, height, juce::Justification::centred);
	}

	void contentAreaChanged (const juce::Rectangle<int>&) override {}

private:
	juce::String text_;
};

class PresetMenuToolbarItem : public juce::ToolbarItemComponent, public PresetController::Observer {
public:
	PresetMenuToolbarItem(int itemId, PresetController *presetController)
	: juce::ToolbarItemComponent(itemId, "", true)
	, presetController_(presetController) {
		this->onClick = [this] {
			auto menu = juce::PopupMenu();
			for (auto &bank : PresetController::getPresetBanks()) {
				auto bankMenu = juce::PopupMenu();
				for (int i = 0; i < PresetController::kNumPresets; i++) {
					bankMenu.addItem(juce::String::formatted("%d: %s", i, bank.presets[i].getName().c_str()), [this, &bank, i] {
						presetController_->loadPresets(bank.file_path.c_str());
						presetController_->selectPreset(i);
					});
				}
				menu.addSubMenu(juce::String::formatted("[%s] %s", bank.read_only ? gettext("F") : gettext("U"), bank.name.c_str()), bankMenu);
			}
			menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
		};
		presetController->addObserver(this);
	}

	~PresetMenuToolbarItem() {
		presetController_->removeObserver(this);
	}

	bool getToolbarItemSizes (int toolbarDepth, bool /*isToolbarVertical*/, int& preferredSize, int& minSize, int& maxSize) override {
		preferredSize = minSize = toolbarDepth;
		maxSize = INT_MAX;
		return true;
	}

	void paintButtonArea (juce::Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown) override {
		g.setColour(juce::Colour::fromRGB(255, 255, 255));
		g.drawText(juce::String::formatted("%03d: %s",
										   presetController_->getCurrPresetNumber(),
										   presetController_->getCurrentPreset().getName().c_str()),
				   5, 0, width, height, juce::Justification::left | juce::Justification::verticallyCentred);
	}

	void contentAreaChanged (const juce::Rectangle<int>&) override {}

	void currentPresetDidChange() override { repaint(); }

private:
	PresetController *presetController_;
};

struct MainComponent::Impl : public juce::ToolbarItemFactory {
	Impl(MainComponent *component, PresetController *presetController)
	: presetController_(presetController)
	, controlPanel_(std::make_unique<ControlPanel>(presetController, true))
	, toolbar_(std::make_unique<juce::Toolbar>()) {
		int toolbarHeight = 25;
		controlPanel_->setBounds(controlPanel_->getBounds().withY(toolbarHeight));
		toolbar_->setBounds(0, 0, controlPanel_->getWidth(), toolbarHeight);
		toolbar_->addDefaultItems(*this);
	}

	enum Item {
		BurgerMenu = 1,
		PresetPrevious,
		PresetNext,
		PresetPopup,
		SavePreset,
	};

	void getAllToolbarItemIds(juce::Array<int> &ids) override {
		ids = {
			BurgerMenu,
			PresetPrevious,
			PresetNext,
			PresetPopup,
			SavePreset,
		};
	}

	void getDefaultItemSet (juce::Array <int>& ids) override {
		getAllToolbarItemIds(ids);
	}

	juce::ToolbarItemComponent* createItem (int itemId) override {
		switch (itemId) {
			case BurgerMenu:
				return new ToolbarItem(itemId, "=", [this] (auto item) { showBurgerMenu(item); });
			case PresetPrevious:
				return new ToolbarItem(itemId, "<", [this] (auto item) { navigatePresets(-1); });
			case PresetNext:
				return new ToolbarItem(itemId, ">", [this] (auto item) { navigatePresets(1); });
			case PresetPopup:
				return new PresetMenuToolbarItem(itemId, presetController_);
			case SavePreset:
				return new ToolbarItem(itemId, "Save", [this] (auto item) { savePreset(); });
		}
		return nullptr;
	}

	void showBurgerMenu(juce::Component *targetComponent) {
		auto menu = juce::PopupMenu();
		menu.addItem(gettext("Open Alternate Tuning File..."), [this] {
			openFile(gettext("Open Scala (.scl) alternate tuning file"), "*.scl", component_->loadTuningScl);
		});
		menu.addItem(gettext("Open Alternate Keyboard Map..."), [this] {
			openFile(gettext("Open alternate keyboard map (Scala .kbm format)"), "*.kbm", component_->loadTuningKbm);
		});
		menu.addItem(gettext("Reset All Tuning Settings to Default"), [this] {
			component_->loadTuningScl(nullptr);
			component_->loadTuningKbm(nullptr);
		});
		menu.addSeparator();
		menu.addItem(1, "Version 1.2.3", false);
		menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(targetComponent));
	}

	void navigatePresets(int delta) {
		presetController_->selectPreset(presetController_->getCurrPresetNumber() + delta);
	}

	void savePreset() {
		// TODO: Create a preset save dialog / menu
		// options:
		// - choose a user bank & slot
		// - create a new bank
		// - enter preset name
	}

	void showPresetMenu(juce::Component *targetComponent) {
		auto menu = juce::PopupMenu();
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
			menu.addSubMenu(text, bankMenu);
		}
		menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(targetComponent));
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

	MainComponent *component_;
	PresetController *presetController_;
	std::unique_ptr<ControlPanel> controlPanel_;
	std::unique_ptr<juce::Toolbar> toolbar_;
};

MainComponent::MainComponent(PresetController *presetController)
: impl_(std::make_unique<Impl>(this, presetController)) {
	addAndMakeVisible(impl_->toolbar_.get());
	addAndMakeVisible(impl_->controlPanel_.get());
	setBounds(0, 0, impl_->controlPanel_->getWidth(), impl_->controlPanel_->getBottom());
}

MainComponent::~MainComponent() = default;

void MainComponent::paint(juce::Graphics &g) {
	g.setColour(juce::Colour::fromRGB(30, 30, 30));
	g.fillAll();
}
