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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "MainComponent.h"

#include "ControlPanel.h"
#include "core/synth/PresetController.h"
#include "core/synth/Synthesizer.h"

// TODO: Disable save button when appropriate

enum CommandIDs {
	randomisePreset = 0x10000,
};

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
	Impl(MainComponent *component, MidiController *midiController, PresetController *presetController, juce::ApplicationCommandManager *commandManager)
	: component_(component)
	, presetController_(presetController)
	, controlPanel_(midiController, presetController)
	, menuButton_(GETTEXT("Menu"))
	, saveButton_(GETTEXT("Save"))
	, commandManager_(commandManager) {
		controlPanel_.setBounds(controlPanel_.getBounds().withY(toolbarHeight));
		menuButton_.onMouseDown = [this] { showMainMenu(&menuButton_); };
		saveButton_.onClick = [this] { savePreset(); };
		populateBankCombo();
		populatePresetCombo();
	}

	~Impl() {
		delete alertWindow_;
	}

	void propertyChanged(const std::string &name, const std::string &value) {
		if (name == PROP_NAME(preset_bank_name)) {
			int bankNumber = 0;
			for (const auto &bank : PresetController::getPresetBanks()) {
				if (bank.name == value) {
					bankCombo_.setSelectedItemIndex(bankNumber, juce::NotificationType::dontSendNotification);
					presetController_->loadPresets(bank.file_path.c_str());
					populatePresetCombo();
					break;
				}
				bankNumber++;
			}
		}
		if (name == PROP_NAME(preset_name)) {
			presetController_->getCurrentPreset().setName(value);
			setPresetComboLabelText(value);
		}
		if (name == PROP_NAME(preset_number) && !value.empty()) {
			int presetNumber = std::stoi(value);
			presetController_->setCurrPresetNumber(presetNumber);
			// Don't call selectPreset() because that would change the parameter values
			presetCombo_.setSelectedItemIndex(presetNumber, juce::NotificationType::dontSendNotification);
		}
	}

	void showMainMenu(juce::Component *targetComponent) {
		auto menu = juce::PopupMenu();

		menu.addSectionHeader(GETTEXT("Tuning"));
		menu.addItem(GETTEXT("Open Alternate Tuning File..."), [this] {
			openFile(GETTEXT("Open Scala (.scl) alternate tuning file"), "*.scl", [this] (const char *filename) {
				setProperty(PROP_NAME(tuning_scl_file), filename);
			});
		});
		menu.addItem(GETTEXT("Open Alternate Keyboard Map..."), [this] {
			openFile(GETTEXT("Open alternate keyboard map (Scala .kbm format)"), "*.kbm", [this] (const char *filename) {
				setProperty(PROP_NAME(tuning_kbm_file), filename);
			});
		});
		menu.addItem(GETTEXT("Reset All Tuning Settings to Default"), [this] {
			setProperty(PROP_NAME(tuning_scl_file), nullptr);
			setProperty(PROP_NAME(tuning_kbm_file), nullptr);
		});

		menu.addSectionHeader(GETTEXT("Edit"));
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::copy);
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::paste);
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::undo);
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::redo);

		menu.addSectionHeader(GETTEXT("Preset"));
		menu.addItem(GETTEXT("Rename..."), [this] {
			renamePreset();
		});
		menu.addItem(GETTEXT("Clear"), [this] {
			presetController_->clearPreset();
		});
		menu.addCommandItem(commandManager_, CommandIDs::randomisePreset);

		menu.addSectionHeader(GETTEXT("Config"));
        auto getIntProperty = [&] (const char *key, int fallback) {
            auto it = component_->properties.find(key);
            if (it != component_->properties.end())
                return std::stoi(it->second);
            return fallback;
        };
        auto setIntProperty = [&] (const char *key, int value) {
            setProperty(key, std::to_string(value).c_str());
        };
		menu.addSubMenu(GETTEXT("Pitch Bend Range"), [&] {
			juce::PopupMenu submenu;
            auto key = PROP_NAME(pitch_bend_range);
			int currentValue = getIntProperty(key, 2);
			for (int i = 1; i <= 24; i++) {
				submenu.addItem(juce::String(std::to_string(i)) + GETTEXT(" Semitones"), true, i == currentValue, [=] {
					setIntProperty(key, i);
				});
			}
			return submenu;
		}());
		menu.addSubMenu(GETTEXT("Max. Polyphony"), [&] {
			juce::PopupMenu submenu;
            auto key = PROP_NAME(max_polyphony);
            int currentValue = getIntProperty(key, 10);
			for (int i = 0; i <= 16; i++) {
				submenu.addItem(i ? std::to_string(i) : GETTEXT("Unlimited"), true, i == currentValue, [=] {
					setIntProperty(key, i);
				});
			}
			return submenu;
		}());
		if (!component_->isPlugin) {
			menu.addSubMenu(GETTEXT("MIDI Channel"), [&] {
				juce::PopupMenu submenu;
                auto key = PROP_NAME(midi_channel);
                int currentValue = getIntProperty(key, 0);
				for (int i = 0; i <= 16; i++) {
					submenu.addItem(i ? std::to_string(i) : GETTEXT("Unlimited"), true, i == currentValue, [=] {
						setIntProperty(key, i);
					});
				}
				return submenu;
			}());
		}

		menu.addSectionHeader(GETTEXT("Help"));
		menu.addItem(GETTEXT("About"), [this] {
			showAbout();
		});
		menu.addItem(GETTEXT("Report a Bug"), [] {
			juce::URL("https://github.com/amsynth/amsynth/issues").launchInDefaultBrowser();
		});
		menu.addItem(GETTEXT("Online Documentation"), [] {
			juce::URL("https://github.com/amsynth/amsynth/wiki").launchInDefaultBrowser();
		});
		menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(targetComponent));
	}

	void savePreset() {
		presetController_->saveCurrentPreset();
		PresetController::rescanPresetBanks();
		populateBankCombo();
		populatePresetCombo();
	}

	juce::Label *setPresetComboLabelText(juce::String text) {
		auto label = dynamic_cast<juce::Label *>(presetCombo_.getChildComponent(0));
		label->setText(text, juce::NotificationType::dontSendNotification);
		return label;
	}

	void renamePreset() {
		alertWindow_ = new juce::AlertWindow(GETTEXT("Rename Preset"), "", juce::MessageBoxIconType::NoIcon, component_);
		alertWindow_->addButton(GETTEXT("Rename"), 100, juce::KeyPress(juce::KeyPress::returnKey));
		alertWindow_->addButton(GETTEXT("Cancel"), 0);
		alertWindow_->addTextEditor("name", presetController_->getCurrentPreset().getName());
		auto callback = juce::ModalCallbackFunction::create([this] (int result) {
			if (result == 100) {
				auto text = alertWindow_->getTextEditorContents("name").toStdString();
				if (presetController_->getCurrentPreset().getName() != text) {
					presetController_->getCurrentPreset().setName(text);
					setProperty(PROP_NAME(preset_name), text.c_str());
					auto label = dynamic_cast<juce::Label *>(presetCombo_.getChildComponent(0));
					label->setText(std::to_string(presetController_->getCurrPresetNumber() + 1) + ": " + text,
								   juce::NotificationType::dontSendNotification);
				}
			}
			alertWindow_ = nullptr;
		});
		alertWindow_->enterModalState(false, callback, true);
		juce::Timer::callAfterDelay(100, [this] {
			// On X11 this needs to be delayed to be effective
			alertWindow_->getTextEditor("name")->grabKeyboardFocus();
		});
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
				bankCombo_.addSectionHeading(GETTEXT("User banks"));
				foundUser = true;
			}
			if (bank.read_only && !foundFactory) {
				bankCombo_.addSectionHeading(GETTEXT("Factory banks"));
				foundFactory = true;
			}
			bankCombo_.addItem(bank.name, bankCombo_.getNumItems() + 1);
			if (bank.file_path == presetController_->getFilePath()) {
				bankCombo_.setSelectedId(bankCombo_.getNumItems(), juce::NotificationType::dontSendNotification);
				saveButton_.setEnabled((currentBankIsWritable_ = juce::File(bank.file_path).hasWriteAccess()));
			}
		}
		bankCombo_.onChange = [this] {
			auto &bank = PresetController::getPresetBanks().at(bankCombo_.getSelectedItemIndex());
			int presetNumber = std::max(0, presetController_->getCurrPresetNumber());
			presetController_->loadPresets(bank.file_path.c_str());
			selectPreset(presetNumber);
			setProperty(PROP_NAME(preset_bank_name), bank.name.c_str());
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
			auto presetNumber = presetCombo_.getSelectedId() - 1;
			if (presetNumber == -1)
				return;
			if (presetNumber == presetController_->getCurrPresetNumber())
				return; // Ignore spam from juce::ComboBox::handleAsyncUpdate)
			selectPreset(presetNumber);
		};
	}

	void selectPreset(int presetNumber) {
		presetController_->selectPreset(presetNumber);
		setProperty(PROP_NAME(preset_name), presetController_->getCurrentPreset().getName().c_str());
		setProperty(PROP_NAME(preset_number), std::to_string(presetNumber).c_str());
	}

	static void openFile(const juce::String &title, const char *filters, const std::function<void(const char *)> &handler) {
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

	std::function<void(const char *name, const char *value)> setProperty;

	juce::ApplicationCommandManager *commandManager_;
	MainComponent *component_;
	PresetController *presetController_;
	ControlPanel controlPanel_;
	MouseDownButton menuButton_;
	juce::ComboBox bankCombo_;
	juce::ComboBox presetCombo_;
	juce::TextButton saveButton_;
	juce::AlertWindow *alertWindow_{nullptr};
	LookAndFeel lookAndFeel_;
	bool currentBankIsWritable_ {false};
};

MainComponent::MainComponent(PresetController *presetController, MidiController *midiController)
: impl_(std::make_unique<Impl>(this, midiController, presetController, &commandManager)) {
	impl_->setProperty = [this] (const char *name, const char *value) {
		sendProperty(name, value);
		properties[name] = value;
	};
	setLookAndFeel(&impl_->lookAndFeel_);
	addAndMakeVisible(impl_->menuButton_);
	addAndMakeVisible(impl_->bankCombo_);
	addAndMakeVisible(impl_->presetCombo_);
	addAndMakeVisible(impl_->controlPanel_);
	addAndMakeVisible(impl_->saveButton_);
	setBounds(0, 0, impl_->controlPanel_.getWidth(), impl_->controlPanel_.getBottom());
	commandManager.registerAllCommandsForTarget(this);
	addKeyListener(commandManager.getKeyMappings());
}

MainComponent::~MainComponent() {
	setLookAndFeel(nullptr);
}

void MainComponent::getAllCommands(juce::Array<juce::CommandID> &commands) {
	commands.add(juce::StandardApplicationCommandIDs::copy);
	commands.add(juce::StandardApplicationCommandIDs::paste);
	commands.add(juce::StandardApplicationCommandIDs::undo);
	commands.add(juce::StandardApplicationCommandIDs::redo);
	commands.add(CommandIDs::randomisePreset);
}

void MainComponent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo &result) {
	juce::String category {GETTEXT("Preset")};

	switch (commandID) {
		case juce::StandardApplicationCommandIDs::copy:
			result.setInfo(GETTEXT("Copy"), "", category, 0);
			result.addDefaultKeypress('c', juce::ModifierKeys::commandModifier);
			break;
		case juce::StandardApplicationCommandIDs::paste:
			result.setInfo(GETTEXT("Paste"), "", category, 0);
			result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier);
			break;
		case juce::StandardApplicationCommandIDs::undo:
			result.setInfo(GETTEXT("Undo"), "", category, 0);
			result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
			break;
		case juce::StandardApplicationCommandIDs::redo:
			result.setInfo(GETTEXT("Redo"), "", category, 0);
			result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
			break;
		case randomisePreset:
			result.setInfo(GETTEXT("Randomise"), GETTEXT("Sets all parameters to a random value"), category, 0);
			result.addDefaultKeypress('r', juce::ModifierKeys::commandModifier);
			break;
		default:
			break;
	}
}

bool MainComponent::perform(const InvocationInfo &info) {
	auto presetController = impl_->presetController_;

	switch (info.commandID) {
		case juce::StandardApplicationCommandIDs::copy:
			juce::SystemClipboard::copyTextToClipboard(presetController->getCurrentPreset().toString());
			break;
		case juce::StandardApplicationCommandIDs::paste:
			if (presetController->getCurrentPreset().fromString(juce::SystemClipboard::getTextFromClipboard().toStdString()))
				impl_->setPresetComboLabelText(std::to_string(presetController->getCurrPresetNumber() + 1)
											   + ": " + presetController->getCurrentPreset().getName());
			break;
		case juce::StandardApplicationCommandIDs::undo:
			presetController->undoChange();
			break;
		case juce::StandardApplicationCommandIDs::redo:
			presetController->redoChange();
			break;
		case randomisePreset:
			presetController->randomiseCurrentPreset();
			break;
		default:
			return false;
	}
	return true;
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

void MainComponent::propertyChanged(const char *name, const char *value) {
	properties[name] = value;
	impl_->propertyChanged(name, value);
}
