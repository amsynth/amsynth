/*
 *  PresetController.cpp
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#if !_WIN32
#include <dirent.h>
#include <unistd.h>
#endif

#include "gettext.h"
#include "filesystem.h"

#define _(string) gettext (string)

using namespace std;


int
PresetController::selectPreset		(const int presetNo)
{
	if (presetNo > (kNumPresets - 1) || presetNo < 0)
		return -1;
	currentPreset = getPreset(currentPresetNo = presetNo);
	notify();
	clearChangeBuffers ();
	return 0;
}

bool
PresetController::containsPresetWithName(const string name)
{
	for (int i=0; i<kNumPresets; i++) 
		if (getPreset(i).getName() == name) 
			return true;
	return false;
}

void
PresetController::clearPreset		()
{
	loadPresets();
	currentPreset = blankPreset;
	commitPreset();
	savePresets();
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
	try
	{
		ofstream file( filename.c_str(), ios::out );
		file << currentPreset.toString();
		file.close();
		return 0;
	}
	catch (std::exception &e)
	{
		return -1;
	}
}

int
PresetController::importPreset		(const string filename)
{
	try
	{
		ifstream ifs( filename.c_str(), ios::in );
		std::string str( (std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>() );
		if (!currentPreset.fromString( str )) return -1;
		currentPreset.setName("Imported: " + currentPreset.getName());
		notify ();
		clearChangeBuffers ();
		return 0;
	}
	catch (std::exception &e)
	{
		return -1;
	}
}

static long int mtime(const char *filename)
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
	if (filename == nullptr)
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

static const char amsynth_file_header[] = { 'a', 'm', 'S', 'y', 'n', 't', 'h', '\n' };

static bool is_amsynth_file(const char *filename)
{
	struct stat st = {0};
	if (stat(filename, &st) < 0)
		return false;

#if !_WIN32
	if (!S_ISREG(st.st_mode))
		return false;
#endif

	FILE *file = fopen(filename, "r");
	if (!file)
		return false;

	char buffer[sizeof(amsynth_file_header)] = {0};
	fread(buffer, sizeof(buffer), 1, file);
	fclose(file);

	if (memcmp(buffer, amsynth_file_header, sizeof(amsynth_file_header)) != 0)
		return false;

	return true;
}

static off_t file_read_contents(const char *filename, void **result)
{
	*result = nullptr;
	FILE *file = fopen(filename, "r");
	if (!file)
		return 0;
	fseek(file, 0, SEEK_END);
#if _WIN32
	long length = ftell(file);
#else
	off_t length = ftello(file);
#endif
	void *buffer = calloc(length + 1, 1);
	fseek(file, 0, SEEK_SET);
	fread(buffer, length, 1, file);
	fclose(file);
	*result = buffer;
	return length;
}

static float float_from_string(const char *s)
{
	if (strchr(s, 'e'))
		return Parameter::valueFromString(std::string(s));
	float rez = 0, fact = 1;
	if (*s == '-'){
		s++;
		fact = -1;
	};
	for (int point_seen = 0; *s; s++){
		if (*s == '.'){
			point_seen = 1;
			continue;
		};
		int d = *s - '0';
		if (d >= 0 && d <= 9){
			if (point_seen) fact /= 10.0f;
			rez = rez * 10.0f + (float)d;
		};
	};
	return rez * fact;
}

static bool readBankFile(const char *filename, Preset *presets)
{
	void *buffer = nullptr;
	off_t buffer_length = file_read_contents(filename, &buffer);
	if (!buffer)
		return false;

	if (memcmp(buffer, amsynth_file_header, sizeof(amsynth_file_header)) != 0) {
		free(buffer);
		return false;
	}

	char *buffer_end = ((char *)buffer) + buffer_length;

	int preset_index = -1;
	char *line_ptr = (char *)buffer + sizeof(amsynth_file_header);
	for (char *end_ptr = line_ptr; end_ptr < buffer_end && *end_ptr; end_ptr++) {
		if (*end_ptr == '\n') {
			*end_ptr = '\0';
			end_ptr++;

			static char preset_prefix[] = "<preset> <name> ";
			if (strncmp(line_ptr, preset_prefix, sizeof(preset_prefix) - 1) == 0) {
				presets[++preset_index] = Preset(std::string(line_ptr + sizeof(preset_prefix) - 1));
			}

			static char parameter_prefix[] = "<parameter> ";
			if (strncmp(line_ptr, parameter_prefix, sizeof(parameter_prefix) - 1) == 0) {
				char *ptr = line_ptr + sizeof(parameter_prefix) - 1;
				char *sep = strchr(ptr, ' ');
				if (sep) {
					Preset &preset = presets[preset_index];
					Parameter &param = preset.getParameter(std::string(ptr, sep - ptr));
					float fval = float_from_string(sep + 1);
					param.setValue(fval);
				}
			}

			line_ptr = end_ptr;
		}
	}
	for (preset_index++; preset_index < PresetController::kNumPresets; preset_index++)
		presets[preset_index] = Preset();
	free(buffer);

	return true;
}

int
PresetController::loadPresets		(const char *filename)
{
	if (filename == nullptr)
		filename = bank_file.c_str();

	long int fileModifiedTime = mtime(filename);
	if (strcmp(filename, bank_file.c_str()) == 0 && lastPresetsFileModifiedTime == fileModifiedTime)
		return 0; // file not modified since last load

	if (!readBankFile(filename, presets))
		return -1;

	currentBankNo = -1;
	const std::vector<BankInfo> &banks = getPresetBanks();
	for (int i = 0; i < (int) banks.size(); i++) {
		if (banks[i].file_path == (std::string) filename) {
			currentBankNo = i;
			break;
		}
	}

	lastPresetsFileModifiedTime = fileModifiedTime;
	bank_file = std::string(filename);

	return 0;
}

void
PresetController::selectBank(int bankNumber)
{
	const std::vector<BankInfo> &banks = getPresetBanks();

	if (bankNumber >= (int) banks.size())
		return;

	if (currentBankNo == bankNumber)
		return;

	for (int i = 0; i < kNumPresets; i++)
		presets[i] = banks[bankNumber].presets[i];

	currentBankNo = bankNumber;
	bank_file = banks[bankNumber].file_path;
	lastPresetsFileModifiedTime = mtime(banks[bankNumber].file_path.c_str());
}

///////////////////////////////////

static std::vector<BankInfo> s_banks;

static void scan_preset_bank(const std::string dir_path, const std::string file_name, bool read_only)
{
	std::string file_path = dir_path + std::string("/") + std::string(file_name);

	std::string bank_name = std::string(file_name);
	if (bank_name == "default") {
		bank_name = _("User bank");
	} else {
		std::string::size_type pos = bank_name.find_first_of(".");
		if (pos != std::string::npos)
			bank_name.erase(pos, string::npos);
	}

	std::replace(bank_name.begin(), bank_name.end(), '_', ' ');

	if (!is_amsynth_file(file_path.c_str()))
		return;

	BankInfo bank_info;
	bank_info.name = bank_name;
	bank_info.file_path = file_path;
	bank_info.read_only = read_only;
	readBankFile(file_path.c_str(), bank_info.presets);
	s_banks.push_back(bank_info);
}

static void scan_preset_banks(const std::string dir_path, bool read_only)
{
#if !_WIN32
	DIR *dir = opendir(dir_path.c_str());
	if (!dir)
		return;

	std::vector<std::string> filenames;

	struct dirent *entry;
	while ((entry = readdir(dir)))
		filenames.push_back(std::string(entry->d_name));

	closedir(dir);

	std::sort(filenames.begin(), filenames.end());

	for (auto &filename : filenames)
		scan_preset_bank(dir_path, filename, read_only);
#endif
}

static std::string sFactoryBanksDirectory;

static void scan_preset_banks()
{
	s_banks.clear();
	auto userBanksDirectory = PresetController::getUserBanksDirectory();
	scan_preset_banks(userBanksDirectory, false);
#ifdef PKGDATADIR
	if (sFactoryBanksDirectory.empty())
		sFactoryBanksDirectory = std::string(PKGDATADIR "/banks");
#endif
	// sFactoryBanksDirectory == userBanksDirectory if the build is configured with a --prefix=$HOME/.local
	if (!sFactoryBanksDirectory.empty() && sFactoryBanksDirectory != userBanksDirectory ) {
		scan_preset_banks(sFactoryBanksDirectory, true);
	}
}

const std::vector<BankInfo> &
PresetController::getPresetBanks()
{
	if (s_banks.empty())
		scan_preset_banks();
	return s_banks;
}

void PresetController::rescanPresetBanks()
{
	scan_preset_banks();
}

std::string PresetController::getUserBanksDirectory()
{
	return filesystem::get().user_banks;
}
