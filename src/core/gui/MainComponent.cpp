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

// TODO: Disable save button when appropriate

static constexpr int toolbarHeight = 25;

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
	juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox& box, juce::Label& label) override {
		return juce::LookAndFeel_V4::getOptionsForComboBoxPopupMenu(box, label)
			.withMaximumNumColumns(4)
			.withMinimumNumColumns(box.getNumItems() == PresetController::kNumPresets ? 4 : 1);
	}

	// TODO: hide borders around toolbar buttons and combos
};

class MouseDownButton : public juce::TextButton {
public:
	explicit MouseDownButton(const juce::String& buttonName) : juce::TextButton(buttonName) {}

	void mouseDown(const juce::MouseEvent &event) override {
		if (event.eventComponent == this) {
			onMouseDown();
		}
	}

	std::function<void()> onMouseDown;
};

struct MainComponent::Impl {
	Impl(MainComponent *component, PresetController *presetController)
	: component_(component)
	, presetController_(presetController)
	, controlPanel_(presetController, true)
	, menuButton_("Menu")
	, saveButton_("Save") {
		controlPanel_.setBounds(controlPanel_.getBounds().withY(toolbarHeight));
		menuButton_.onMouseDown = [this] { showMainMenu(&menuButton_); };
		saveButton_.onClick = [this] { savePreset(); };
		populateBankCombo();
		populatePresetCombo();
	}

	void showMainMenu(juce::Component *targetComponent) {
		auto menu = juce::PopupMenu();

		menu.addSectionHeader(gettext("File"));
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

		menu.addSectionHeader(gettext("Edit"));
		menu.addItem(gettext("Undo"), [this] {
			presetController_->undoChange();
		});
		menu.addItem(gettext("Redo"), [this] {
			presetController_->redoChange();
		});

		menu.addSectionHeader(gettext("Preset"));
		menu.addItem(gettext("Rename..."), [this] {
			renamePreset();
		});
		menu.addItem(gettext("Clear"), [this] {
			presetController_->clearPreset();
		});
		menu.addItem(gettext("Randomise"), [this] {
			presetController_->randomiseCurrentPreset();
		});

		menu.addSectionHeader(gettext("Help"));
		menu.addItem(gettext("About"), [this] {
			showAbout();
		});
		menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(targetComponent));
	}

	void savePreset() {
		presetController_->saveCurrentPreset();
		PresetController::rescanPresetBanks();
		populateBankCombo();
		populatePresetCombo();
	}

	void renamePreset() {
		if (!presetCombo_.getSelectedId())
			return;

		auto label = dynamic_cast<juce::Label *>(presetCombo_.getChildComponent(0));
		label->setText(presetController_->getCurrentPreset().getName(),
					   juce::NotificationType::dontSendNotification);

		presetCombo_.setEditableText(true);
		presetCombo_.showEditor();

		label->onEditorHide = [this, label] {
			presetCombo_.setEditableText(false);
			auto text = label->getText().toStdString();
			if (presetController_->getCurrentPreset().getName() != text) {
				presetController_->getCurrentPreset().setName(text);
			}
			label->setText(std::to_string(presetController_->getCurrPresetNumber() + 1) + ": " + text,
						   juce::NotificationType::dontSendNotification);
			label->onEditorHide = nullptr;
		};
	}

	void showAbout() {
		auto editor = new juce::TextEditor();
		editor->setSize(component_->getWidth(), component_->getHeight());
		editor->setJustification(juce::Justification::centred);
		editor->setMultiLine(true);
		editor->setReadOnly(true);
		editor->setText("amsynth\n"
						"version " PACKAGE_VERSION "\n"
						"\n"
						"Copyright (c) 2002 Nick Dowell\n"
						"\n"
						"With contributions from:\n"
						"Adam Sampson\n"
						"Adrian Knoth\n"
						"Andy Ryan\n"
						"Bob Ham\n"
						"Brian\n"
						"Chris Cannam\n"
						"Darrick Servis\n"
						"Johan Martinsson\n"
						"Karsten Wiese\n"
						"Martin Tarenskeen\n"
						"Paul Winkler\n"
						"Samuli Suominen\n"
						"Sebastien Cevey\n"
						"Taybin Rutkin\n"
						"\n"
						"Includes Freeverb by Jezar Wakefield");
		class MouseListener : public juce::MouseListener {
			void mouseDown(const juce::MouseEvent &event) override {
				event.eventComponent->getParentComponent()->removeChildComponent(event.eventComponent);
				delete event.eventComponent;
			}
		};
		editor->addMouseListener(new MouseListener(), false);
		component_->addAndMakeVisible(editor);
	}

	void populateBankCombo() {
		bankCombo_.clear();
		bool foundUser {false};
		bool foundFactory {false};
		for (const auto &bank : PresetController::getPresetBanks()) {
			if (!bank.read_only && !foundUser) {
				bankCombo_.addSectionHeading("User banks");
				foundUser = true;
			}
			if (bank.read_only && !foundFactory) {
				bankCombo_.addSectionHeading("Factory banks");
				foundFactory = true;
			}
			bankCombo_.addItem(bank.name, bankCombo_.getNumItems() + 1);
			if (bank.file_path == presetController_->getFilePath()) {
				bankCombo_.setSelectedId(bankCombo_.getNumItems());
				saveButton_.setEnabled((currentBankIsWritable_ = juce::File(bank.file_path).hasWriteAccess()));
			}
		}
		bankCombo_.onChange = [this] {
			auto &bank = PresetController::getPresetBanks().at(bankCombo_.getSelectedItemIndex());
			presetController_->loadPresets(bank.file_path.c_str());
			presetController_->selectPreset(std::max(0, presetController_->getCurrPresetNumber()));
			saveButton_.setEnabled((currentBankIsWritable_ = juce::File(bank.file_path).hasWriteAccess()));
			populatePresetCombo();
		};
	}

	void populatePresetCombo() {
		presetCombo_.clear();
		for (const auto &bank : PresetController::getPresetBanks())
			if (bank.file_path == presetController_->getFilePath())
				for (int i = 0; i < PresetController::kNumPresets; i++)
					presetCombo_.addItem(std::to_string(i + 1) + ": " + bank.presets[i].getName(), i + 1);
		presetCombo_.setSelectedItemIndex(presetController_->getCurrPresetNumber(),
										  juce::NotificationType::dontSendNotification);
		presetCombo_.onChange = [this] {
			auto presetIndex = presetCombo_.getSelectedId() - 1;
			if (presetIndex == -1)
				return;
			presetController_->selectPreset(presetIndex);
		};
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
	ControlPanel controlPanel_;
	MouseDownButton menuButton_;
	juce::ComboBox bankCombo_;
	juce::ComboBox presetCombo_;
	juce::TextButton saveButton_;
	LookAndFeel lookAndFeel_;
	bool currentBankIsWritable_ {false};
};

MainComponent::MainComponent(PresetController *presetController)
: impl_(std::make_unique<Impl>(this, presetController)) {
	setLookAndFeel(&impl_->lookAndFeel_);
	addAndMakeVisible(impl_->menuButton_);
	addAndMakeVisible(impl_->bankCombo_);
	addAndMakeVisible(impl_->presetCombo_);
	addAndMakeVisible(impl_->controlPanel_);
	addAndMakeVisible(impl_->saveButton_);
	setBounds(0, 0, impl_->controlPanel_.getWidth(), impl_->controlPanel_.getBottom());
}

MainComponent::~MainComponent() {
	setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics &g) {
	g.setColour(juce::Colour::fromRGB(30, 30, 30));
	g.fillAll();
}

void MainComponent::resized() {
	impl_->menuButton_.setTopLeftPosition(0, 0);
	impl_->menuButton_.changeWidthToFitText(toolbarHeight);
	impl_->saveButton_.changeWidthToFitText(toolbarHeight);
	impl_->saveButton_.setTopRightPosition(getWidth(), 0);
	// TODO: shrink bank combo if possible
	int space = impl_->saveButton_.getX() - impl_->menuButton_.getRight();
	impl_->bankCombo_.setBounds(impl_->menuButton_.getRight(), 0, space / 2, toolbarHeight);
	impl_->presetCombo_.setBounds(impl_->bankCombo_.getRight(), 0, space / 2, toolbarHeight);
}
