/*
 *  PresetController.cc
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

#include "PresetController.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;


PresetController::PresetController	()
:	updateListener (0)
,	nullpreset ("null preset")
,	currentPresetNo (-1)
{
	presets = new Preset [kNumPresets];
}

PresetController::~PresetController	()
{
	delete[] presets;
	clearChangeBuffers();
}

int
PresetController::selectPreset		(const int preset)
{
    if (preset > (kNumPresets - 1) || preset < 0) { return -1; }
	if (preset != currentPresetNo)
	{
		currentPreset = getPreset (preset);
		currentPresetNo = preset;
		notify ();
		clearChangeBuffers ();
	}
    return 0;
}

int 
PresetController::selectPreset		(const string name)
{
	for (int i=0; i<kNumPresets; i++) if (getPreset(i).getName() == name) return selectPreset (i);
	return -1;
}

bool
PresetController::containsPresetWithName(const string name)
{
	for (int i=0; i<kNumPresets; i++) 
		if (getPreset(i).getName() == name) 
			return true;
	return false;
}

Preset&
PresetController::getPreset			(const string name)
{
	for (int i=0; i<kNumPresets; i++) if (getPreset(i).getName() == name) return getPreset (i);
	return nullpreset;
}

int
PresetController::newPreset			()
{
	for (int i=0; i<kNumPresets; i++) if (getPreset(i).getName() == "New Preset") return selectPreset (i);
	return -1;
}

void
PresetController::deletePreset		()
{
	currentPreset = blankPreset;
	notify ();
	clearChangeBuffers();
}

void
PresetController::pushParamChange	(const Param param, const float value)
{
	undoBuffer.push(new ParamChange(param, value));
	clearRedoBuffer();
}

void
PresetController::undoChange	()
{
	if(!undoBuffer.empty())
	{
		undoBuffer.top()->initiateUndo(this);
		delete undoBuffer.top();
		undoBuffer.pop();
	}
}

void
PresetController::redoChange	()
{
	if(!redoBuffer.empty())
	{
		redoBuffer.top()->initiateRedo(this);
		delete redoBuffer.top();
		redoBuffer.pop();
	}
}

void
PresetController::undoChange	( ParamChange *change )
{
	redoBuffer.push(new ParamChange(change->param, currentPreset.getParameter(change->param).getValue()));
	currentPreset.getParameter(change->param).setValue(change->value);
}

void
PresetController::redoChange	( ParamChange *change )
{
	undoBuffer.push(new ParamChange(change->param, currentPreset.getParameter(change->param).getValue()));
	currentPreset.getParameter(change->param).setValue(change->value);
}

void
PresetController::undoChange	( RandomiseChange *change )
{
	redoBuffer.push(new RandomiseChange(currentPreset));
	currentPreset = change->preset;
}

void
PresetController::redoChange	( RandomiseChange *change )
{
	undoBuffer.push(new RandomiseChange(currentPreset));
	currentPreset = change->preset;
}

void
PresetController::randomiseCurrentPreset	()
{
	undoBuffer.push(new RandomiseChange(currentPreset));
	clearRedoBuffer();
	currentPreset.randomise();
}

int
PresetController::exportPreset		(const string filename)
{
	ofstream file( filename.c_str(), ios::out );
	file << currentPreset.toString();
	file.close();
	return 0;
}

int
PresetController::importPreset		(const string filename)
{	
	ifstream ifs( filename.c_str(), ios::in );
	std::string str( (std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>() );
	if (!currentPreset.fromString( str )) return -1;
	currentPreset.setName("Imported: " + currentPreset.getName());
	notify ();
	clearChangeBuffers ();
	return 1;
}

static unsigned long mtime(const char *filename)
{
	struct stat st;
	if (stat(filename, &st) != 0) {
		return 0;
	}
	return st.st_mtime;
}

int 
PresetController::savePresets		(const char *filename)
{
	if (filename == NULL)
		filename = bank_file.c_str();

	ofstream file( filename, ios::out );
  
	file << "amSynth" << endl;
	for (int i = 0; i < kNumPresets; i++) {
		if (presets[i].getName()!="unused"){
#ifdef _DEBUG
			cout << "<PresetController::savePresets():- preset: name= "
			<< presets[i].getName() << endl;
#endif
			file << "<preset> " << "<name> " << presets[i].getName() << endl;
			for (unsigned n = 0; n < presets[i].ParameterCount(); n++)
			{
#ifdef _DEBUG
				cout << "PresetController::savePresets() :- parameter name="
				<< presets[i].getParameter(n).getName() << " value= "
				<< presets[i].getParameter(n).getValue() << endl;
#endif
				file << "<parameter> " 
				<< presets[i].getParameter(n).getName()
				<< " " << presets[i].getParameter(n).getValue() << endl;
			}
		}
	}
	file << "EOF" << endl;
	file.close();
#ifdef _DEBUG
	cout << "<PresetController::savePresets() success" << endl;
#endif

	lastPresetsFileModifiedTime = mtime(filename);
	bank_file = std::string(filename);

	return 0;
}

int 
PresetController::loadPresets		(const char *filename)
{
	if (filename == NULL)
		filename = bank_file.c_str();

#ifdef _DEBUG
	cout << "<PresetController::loadPresets()>" << endl;
#endif

	if (strcmp(filename, bank_file.c_str()) == 0 && lastPresetsFileModifiedTime == mtime(filename)) {
		return 0; // file not modified since last load
	}

	ifstream file;
	string buffer;

	try {
		file.open(filename, ios::in);
		file >> buffer;
	}
	catch(...) {
		return -1;
	}

	if (buffer != "amSynth") {
#ifdef _DEBUG
		cout <<
		"<PresetController::loadPresets()> not an amSynth file, bailing out"
		<< endl;
#endif
	return -1;
	}

	delete[] presets;
	presets = new Preset [kNumPresets];

	int preset = -1;
	file >> buffer;
	while (file.good()) {
		if (buffer == "<preset>") {
			preset++;
			file >> buffer;

			string presetName = "";
			
			//get the preset's name
			file >> buffer;

			if (buffer != "<parameter>") {
				presetName = buffer;
				file >> buffer;
			}

			while (buffer != "<parameter>") {
				presetName += " ";
				presetName += buffer;
				file >> buffer;
			}
#ifdef _DEBUG
			cout << "<PresetController::loadPresets()>: Preset name: "
			<< presetName << endl;
#endif
			presets[preset].setName(presetName);
			//get the parameters
			while (buffer == "<parameter>") {
				string name;
				file >> buffer;
				name = buffer;
				file >> buffer;
#ifdef _DEBUG
				cout << "<PresetController::loadPresets()>: Parameter:- name="
				<< name << " value=" << buffer << endl;
#endif

				Parameter &param = presets[preset].getParameter(name);
				if (param.getName() == name) // make sure parameter name is supported
				{
					float fval = Parameter::valueFromString(buffer);
					param.setValue(fval);
					if (param.getValue() != fval)
					{
						cerr << "warning: parameter '" << name  << 
							"' could not be set to value: " << fval << 
							" (min = " << param.getMin() << ", max = " << 
							param.getMax() << ")" << endl;
					}
				}
				file >> buffer;
			}
		} else {
			file.close();
		}
	}

	bank_file = std::string(filename);
	lastPresetsFileModifiedTime = mtime(filename);

	notify ();
	
#ifdef _DEBUG
	cout << "<PresetController::loadPresets()>: success" << endl;
#endif

	return 0;
}

///////////////////////////////////

static std::vector<BankInfo> s_banks;

static void scan_preset_bank(const std::string dir_path, const std::string file_name, bool read_only)
{
	std::string file_path = dir_path + std::string("/") + std::string(file_name);

	std::string bank_name = std::string(file_name);
	if (bank_name == std::string(".amSynth.presets")) {
		bank_name = "User bank";
	} else {
		std::string::size_type pos = bank_name.find_first_of(".");
		if (pos != std::string::npos)
			bank_name.erase(pos, string::npos);
	}

	std::replace(bank_name.begin(), bank_name.end(), '_', ' ');

	PresetController preset_controller;
	if (preset_controller.loadPresets(file_path.c_str()) != 0)
		return;

	BankInfo bank_info;
	bank_info.name = bank_name;
	bank_info.file_path = file_path;
	bank_info.read_only = read_only;
	s_banks.push_back(bank_info);
}

static void scan_preset_banks(const std::string dir_path, bool read_only)
{
	DIR *dir = opendir(dir_path.c_str());
	if (!dir)
		return;

	std::vector<std::string> filenames;

	int return_code = 0;
	struct dirent entry = {0};
	struct dirent *result = NULL;

	for (return_code = readdir_r(dir, &entry, &result); result != NULL && return_code == 0; return_code = readdir_r(dir, &entry, &result))
		filenames.push_back(std::string(entry.d_name));

	closedir(dir);

	std::sort(filenames.begin(), filenames.end());

	for (std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
		scan_preset_bank(dir_path, *it, read_only);
}

static void scan_preset_banks()
{
	s_banks.clear();
	scan_preset_bank(std::string(getenv("HOME")), ".amSynth.presets", false);
	scan_preset_banks(PresetController::getUserBanksDirectory(), false);
	scan_preset_banks(PresetController::getFactoryBanksDirectory(), true);
}

const std::vector<BankInfo> &
PresetController::getPresetBanks()
{
	if (!s_banks.size())
		scan_preset_banks();
	return s_banks;
}

void PresetController::rescanPresetBanks()
{
	scan_preset_banks();
}

#ifdef PKGDATADIR
std::string PresetController::getFactoryBanksDirectory()
{
	return std::string(PKGDATADIR "/banks");
}
#endif

std::string PresetController::getUserBanksDirectory()
{
	return std::string(getenv("HOME")) + std::string("/.amsynth/banks");
}
