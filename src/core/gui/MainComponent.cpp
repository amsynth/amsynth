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

#include <alloca.h>

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

class MenuButton : public juce::ShapeButton {
public:
	explicit MenuButton(const juce::String &buttonName)
	: juce::ShapeButton(buttonName, juce::Colours::lightgrey, juce::Colours::lightgrey, juce::Colours::white) {
		// From JUCE/examples/GUI/MenusDemo.h
		static constexpr uint8_t burgerMenuPathData[] = {
			110,109,0,0,128,64,0,0,32,65,108,0,0,224,65,0,0,32,65,98,254,212,232,65,0,0,32,65,0,0,240,65,252,
			169,17,65,0,0,240,65,0,0,0,65,98,0,0,240,65,8,172,220,64,254,212,232,65,0,0,192,64,0,0,224,65,0,0,
			192,64,108,0,0,128,64,0,0,192,64,98,16,88,57,64,0,0,192,64,0,0,0,64,8,172,220,64,0,0,0,64,0,0,0,65,
			98,0,0,0,64,252,169,17,65,16,88,57,64,0,0,32,65,0,0,128,64,0,0,32,65,99,109,0,0,224,65,0,0,96,65,108,
			0,0,128,64,0,0,96,65,98,16,88,57,64,0,0,96,65,0,0,0,64,4,86,110,65,0,0,0,64,0,0,128,65,98,0,0,0,64,
			254,212,136,65,16,88,57,64,0,0,144,65,0,0,128,64,0,0,144,65,108,0,0,224,65,0,0,144,65,98,254,212,232,
			65,0,0,144,65,0,0,240,65,254,212,136,65,0,0,240,65,0,0,128,65,98,0,0,240,65,4,86,110,65,254,212,232,
			65,0,0,96,65,0,0,224,65,0,0,96,65,99,109,0,0,224,65,0,0,176,65,108,0,0,128,64,0,0,176,65,98,16,88,57,
			64,0,0,176,65,0,0,0,64,2,43,183,65,0,0,0,64,0,0,192,65,98,0,0,0,64,254,212,200,65,16,88,57,64,0,0,208,
			65,0,0,128,64,0,0,208,65,108,0,0,224,65,0,0,208,65,98,254,212,232,65,0,0,208,65,0,0,240,65,254,212,
			200,65,0,0,240,65,0,0,192,65,98,0,0,240,65,2,43,183,65,254,212,232,65,0,0,176,65,0,0,224,65,0,0,176,
			65,99,101,0,0
		};
		juce::Path path;
		path.loadPathFromData(burgerMenuPathData, sizeof(burgerMenuPathData));
		setShape(path, false, true, false);
		setBorderSize(juce::BorderSize<int>(4));
	}

	void mouseDown(const juce::MouseEvent &event) override {
		if (event.eventComponent == this) {
			onMouseDown();
		}
	}

	std::function<void()> onMouseDown;
};

struct MainComponent::Impl : private juce::Timer {
	Impl(MainComponent *component, MidiController *midiController, PresetController *presetController, juce::ApplicationCommandManager *commandManager)
	: commandManager_(commandManager)
	, component_(component)
	, presetController_(presetController)
	, controlPanel_(midiController, presetController)
	, menuButton_(GETTEXT("Menu"))
	, saveButton_(GETTEXT("Save"))
	, prevButton_("<")
	, nextButton_(">") {
		controlPanel_.setBounds(controlPanel_.getBounds().withY(toolbarHeight));
		menuButton_.onMouseDown = [this] { showMainMenu(&menuButton_); };
		saveButton_.onClick = [this] { savePreset(); };
		prevButton_.onClick = [this] { selectNextPreset(-1); };
		nextButton_.onClick = [this] { selectNextPreset(1); };
		populateBankCombo();
		populatePresetCombo();
		startTimer(100);
	}

	~Impl() {
		delete alertWindow_;
	}

	void timerCallback() final {
		updateSaveButton();
	}

	void updateSaveButton() {
		saveButton_.setEnabled(currentBankIsWritable_ && presetController_->isCurrentPresetModified());
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
			updatePresetComboLabelText();
		}
		if (name == PROP_NAME(preset_number) && !value.empty()) {
			auto presetName = presetController_->getCurrentPreset().getName();
			int presetNumber = std::stoi(value);
			presetController_->setCurrPresetNumber(presetNumber);
			presetController_->getCurrentPreset().setName(presetName);
			// Don't call selectPreset() because that would change the parameter values
			presetCombo_.setSelectedItemIndex(presetNumber, juce::NotificationType::dontSendNotification);
			updatePresetComboLabelText();
		}
	}

	void showMainMenu(juce::Component *targetComponent) {
		auto menu = juce::PopupMenu();

		menu.addSectionHeader(GETTEXT("Edit"));
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::copy);
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::paste);
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::undo);
		menu.addCommandItem(commandManager_, juce::StandardApplicationCommandIDs::redo);

		menu.addSectionHeader(GETTEXT("Preset"));
		menu.addItem(GETTEXT("Create New User Bank..."), [this] {
			addNewUserBank();
		});
		menu.addItem(GETTEXT("Rename..."), [this] {
			renamePreset();
		});
		menu.addItem(GETTEXT("Clear"), [this] {
			presetController_->clearPreset();
		});
		menu.addCommandItem(commandManager_, CommandIDs::randomisePreset);

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
#ifdef WITH_MTS_ESP
		do {
			auto it = component_->properties.find(PROP_NAME(tuning_mts_esp_disabled));
			bool currentValue = it == component_->properties.end() ? false : std::stoi(it->second);
			menu.addItem(GETTEXT("Use MTS-ESP if available"), true, !currentValue, [this, currentValue] {
				setProperty(PROP_NAME(tuning_mts_esp_disabled), currentValue ? "0" : "1");
			});
		} while (0);
#endif

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

	void updatePresetComboLabelText() {
		auto text = (std::to_string(presetController_->getCurrPresetNumber() + 1)
					 + ": " + presetController_->getCurrentPreset().getName());
		auto label = dynamic_cast<juce::Label *>(presetCombo_.getChildComponent(0));
		label->setText(text, juce::NotificationType::dontSendNotification);
	}

	void addNewUserBank() {
		showTextAlert(GETTEXT("Add New User Bank"), GETTEXT("Create"), "Bank 1", [this](std::string text) {
			if (PresetController::createUserBank(text)) {
				PresetController::rescanPresetBanks();
				populateBankCombo();
			} else {
				showError(GETTEXT("Failed to create user bank with name \"%s\""), text.c_str());
			}
		});
	}

	void renamePreset() {
		auto &name = presetController_->getCurrentPreset().getName();
		showTextAlert(GETTEXT("Rename Preset"), GETTEXT("Rename"), name, [this](std::string text) {
			if (presetController_->getCurrentPreset().getName() != text) {
				presetController_->getCurrentPreset().setName(text);
				setProperty(PROP_NAME(preset_name), text.c_str());
				auto label = dynamic_cast<juce::Label *>(presetCombo_.getChildComponent(0));
				label->setText(std::to_string(presetController_->getCurrPresetNumber() + 1) + ": " + text,
							   juce::NotificationType::dontSendNotification);
			}
		});
	}
	
	void showTextAlert(const juce::String &title, const juce::String &okButton, const std::string &text,
					   std::function<void(std::string)> &&okAction) {
		alertWindow_ = new juce::AlertWindow(title, "", juce::MessageBoxIconType::NoIcon, component_);
		alertWindow_->addButton(okButton, 100, juce::KeyPress(juce::KeyPress::returnKey));
		alertWindow_->addButton(GETTEXT("Cancel"), 0);
		alertWindow_->addTextEditor("text", text);
		auto *textEditor = alertWindow_->getTextEditor("text");
		auto callback = juce::ModalCallbackFunction::create([=] (int result) {
			if (result == 100) {
				okAction(textEditor->getText().toStdString());
			}
			alertWindow_ = nullptr;
		});
		alertWindow_->enterModalState(false, callback, true);
#if JUCE_LINUX
		// On X11 this needs to be delayed to be effective
		juce::Timer::callAfterDelay(100, [=] { textEditor->grabKeyboardFocus(); });
#else
		textEditor->grabKeyboardFocus();
#endif
	}

	template <typename... Args>
	void showError(const juce::String &fmt, Args... args) {
		if (alertWindow_)
			alertWindow_->exitModalState(0);
		// Not using juce::String::formatted because it doesn't properly handle UTF-8
		auto size = snprintf(nullptr, 0, fmt.toStdString().c_str(), args...) + 1;
		char *message = (char *)alloca(size);
		snprintf(message, size, fmt.toStdString().c_str(), args...);
		juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, GETTEXT("Error"),
											   juce::String(juce::CharPointer_UTF8(message)));
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
				currentBankIsWritable_ = juce::File(bank.file_path).hasWriteAccess();
				updateSaveButton();
			}
		}
		bankCombo_.onChange = [this] {
			auto &bank = PresetController::getPresetBanks().at(bankCombo_.getSelectedItemIndex());
			int presetNumber = std::max(0, presetController_->getCurrPresetNumber());
			presetController_->loadPresets(bank.file_path.c_str());
			selectPreset(presetNumber);
			setProperty(PROP_NAME(preset_bank_name), bank.name.c_str());
			currentBankIsWritable_ = juce::File(bank.file_path).hasWriteAccess();
			updateSaveButton();
			populatePresetCombo();
		};
	}

	void populatePresetCombo() {
		presetCombo_.clear(juce::NotificationType::dontSendNotification);
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
			selectPreset(presetNumber);
		};
	}

	void selectPreset(int presetNumber) {
		presetController_->selectPreset(presetNumber);
		setProperty(PROP_NAME(preset_name), presetController_->getCurrentPreset().getName().c_str());
		setProperty(PROP_NAME(preset_number), std::to_string(presetNumber).c_str());
	}

	void selectNextPreset(int direction) {
		int numBanks = bankCombo_.getNumItems(), numPresets = presetCombo_.getNumItems();
		if (!numBanks) return;
		int presetIndex = presetCombo_.getSelectedItemIndex() + direction;
		int bankIndex = bankCombo_.getSelectedItemIndex();
		if (presetIndex >= numPresets)
		{
			presetIndex = 0;
			bankIndex = (bankIndex + 1) % numBanks;
		}
		else if (presetIndex < 0)
		{
			presetIndex = numPresets - 1;
			bankIndex = (bankIndex ? bankIndex : numBanks) - 1;
		}
		bankCombo_.setSelectedItemIndex(bankIndex, juce::NotificationType::sendNotificationSync);
		presetCombo_.setSelectedItemIndex(presetIndex, juce::NotificationType::sendNotificationSync);
		updateSaveButton();
	}

	static void openFile(const juce::String &title, const char *filters, const std::function<void(const char *)> &handler) {
		auto cwd = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
		auto chooser = new juce::FileChooser(title, cwd, filters);
		chooser->launchAsync(juce::FileBrowserComponent::openMode, [chooser, handler] (const auto &) {
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
	MenuButton menuButton_;
	juce::ComboBox bankCombo_;
	juce::ComboBox presetCombo_;
	juce::TextButton saveButton_;
	juce::TextButton prevButton_;
	juce::TextButton nextButton_;
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
	addAndMakeVisible(impl_->prevButton_);
	addAndMakeVisible(impl_->nextButton_);
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
				impl_->updatePresetComboLabelText();
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
	g.setColour(impl_->lookAndFeel_.findColour(juce::ComboBox::ColourIds::backgroundColourId));
	g.fillAll();
}

void MainComponent::resized() {
	impl_->menuButton_.setBounds(0, 0, toolbarHeight, toolbarHeight);
	impl_->saveButton_.setTopLeftPosition(impl_->menuButton_.getRight(), 0);
	impl_->saveButton_.changeWidthToFitText(toolbarHeight);
	impl_->prevButton_.setBounds(getWidth() - toolbarHeight * 2, 0, toolbarHeight, toolbarHeight);
	impl_->nextButton_.setBounds(getWidth() - toolbarHeight, 0, toolbarHeight, toolbarHeight);
	// TODO: shrink bank combo if possible
	int space = impl_->prevButton_.getX() - impl_->saveButton_.getRight();
	impl_->bankCombo_.setBounds(impl_->saveButton_.getRight(), 0, space / 2, toolbarHeight);
	impl_->presetCombo_.setBounds(impl_->bankCombo_.getRight(), 0, space / 2, toolbarHeight);
}

void MainComponent::propertyChanged(const char *name, const char *value) {
	properties[name] = value;
	impl_->propertyChanged(name, value);
}
