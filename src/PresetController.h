/*
 *  PresetController.h
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#ifndef _PRESETCONTROLLER_H
#define _PRESETCONTROLLER_H

#include <string>
#include <vector>
#include <stack>

#include "Preset.h"
#include "UpdateListener.h"

struct BankInfo {
	std::string name;
	std::string file_path;
	bool read_only;
};

class PresetController {
public:
	enum { kNumPresets = 128 };

			PresetController	();
			~PresetController	();
	
	/* Selects a Preset and makes it current, updating averything as neccessary.
	 * If the requested preset does not exist, then the request is ignored, and
	 * an error value is returned. */
	int		selectPreset		(const int preset);
	int		selectPreset		(const std::string preset);

	// returns the preset currently being edited
	Preset&	getCurrentPreset	() { return currentPreset; }
	
	// access presets in the memory bank
	Preset&	getPreset			(int preset) { return presets[preset]; }
	const Preset & getPreset	(int preset) const { return presets[preset]; }
	Preset&	getPreset			(const std::string name);

	bool	containsPresetWithName(const std::string name);
	bool	isCurrentPresetModified() { return !currentPreset.isEqual(presets[currentPresetNo]); }
	
	// Commit the current preset to memory
	void	commitPreset		() { presets[currentPresetNo] = currentPreset; notify(); }

	// Selects a new, unused preset ready for editing.
	int		newPreset			();
	void	deletePreset		();

	// Manages undo/redo for changes to current preset.
	void	pushParamChange		( const Param param, const float value );
	void	undoChange			();
	void	redoChange			();

	// Randomises the current preset.
	void	randomiseCurrentPreset	();
	
	// Saves the active preset to the filename filename
	int		exportPreset		(const std::string filename);
	int		importPreset		(const std::string filename);
	
	// Loading & Saving of bank files
	int		loadPresets			(const char *filename = NULL);
	int		savePresets			(const char *filename = NULL);

    void	setUpdateListener	(UpdateListener & ul) { updateListener = &ul; }

    int		getCurrPresetNumber	() { return currentPresetNo; }

	const std::string & getFilePath() { return bank_file; }

	static const std::vector<BankInfo> & getPresetBanks();
	static void rescanPresetBanks();

	static void setFactoryBanksDirectory(std::string path);
	static std::string getUserBanksDirectory();

protected:
	void	notify				() { if (updateListener) updateListener->update(); }

private:
	std::string		bank_file;
	UpdateListener*	updateListener;
	Preset*			presets;
	Preset 			currentPreset;
	Preset			blankPreset;
	Preset 			nullpreset;
	int 			currentPresetNo;
	unsigned long 	lastPresetsFileModifiedTime;

	class ChangeData {
		public:
            virtual ~ChangeData() {};
			virtual void initiateUndo( PresetController * ) = 0;
			virtual void initiateRedo( PresetController * ) = 0;
	};

	class ParamChange: public ChangeData {
		public:
			Param param;
			float value;

			ParamChange(Param nParam, float nValue)
					:param(nParam),
					value(nValue) {}

			void initiateUndo( PresetController *presetController ) {
				presetController->undoChange(this);
			}

			void initiateRedo( PresetController *presetController ) {
				presetController->redoChange(this);
			}
	};

	class RandomiseChange: public ChangeData {
		public:
			Preset preset;

			RandomiseChange(Preset &nPreset) {
				preset = nPreset; // Uses operator override.
			}

			void initiateUndo( PresetController *presetController ) {
				presetController->undoChange(this);
			}

			void initiateRedo( PresetController *presetController ) {
				presetController->redoChange(this);
			}
	};

	void undoChange	( ParamChange * );
	void undoChange	( RandomiseChange * );
	void redoChange	( ParamChange * );
	void redoChange	( RandomiseChange * );

	std::stack<ChangeData *>	undoBuffer;
	std::stack<ChangeData *>	redoBuffer;
	void clearUndoBuffer	() { while( !undoBuffer.empty() ) { delete undoBuffer.top(); undoBuffer.pop(); } }
	void clearRedoBuffer	() { while( !redoBuffer.empty() ) { delete redoBuffer.top(); redoBuffer.pop(); } }
	void clearChangeBuffers	() { clearUndoBuffer(); clearRedoBuffer(); }

};

#endif
