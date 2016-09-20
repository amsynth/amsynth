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

#include "gettext.h"
#define _(string) gettext (string)

using namespace std;


PresetController::PresetController	()
:	bank_file ("")
,	updateListener (0)
,	nullpreset ("null preset")
,	currentPresetNo (-1)
,	lastPresetsFileModifiedTime (0)
{
	presets = new Preset [kNumPresets];
}

PresetController::~PresetController	()
{
	delete[] presets;
	clearChangeBuffers();
}

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

static const char amsynth_file_header[] = { 'a', 'm', 'S', 'y', 'n', 't', 'h', '\n' };

static bool is_amsynth_file(const char *filename)
{
	struct stat st = {0};
	if (stat(filename, &st) < 0)
		return false;

	if (!S_ISREG(st.st_mode))
		return false;

	FILE *file = fopen(filename, "r");
	if (!file)
		return false;

	char buffer[sizeof(amsynth_file_header)] = {0};
	fread(buffer, sizeof(buffer), 1, file);
	fclose(file), file = NULL;

	if (memcmp(buffer, amsynth_file_header, sizeof(amsynth_file_header)) != 0)
		return false;

	return true;
}

static off_t file_read_contents(const char *filename, void **result)
{
	*result = NULL;
	FILE *file = fopen(filename, "r");
	if (!file)
		return 0;
	fseek(file, 0, SEEK_END);
	off_t length = ftello(file);
	void *buffer = calloc(length + 1, 1);
	fseek(file, 0, SEEK_SET);
	fread(buffer, length, 1, file);
	fclose(file), file = NULL;
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

int
PresetController::loadPresets		(const char *filename)
{
	if (filename == NULL)
		filename = bank_file.c_str();

	if (!is_amsynth_file(filename))
		return -1;

	if (strcmp(filename, bank_file.c_str()) == 0 && lastPresetsFileModifiedTime == mtime(filename))
		return 0; // file not modified since last load

	void *buffer = NULL;
	off_t buffer_length = file_read_contents(filename, &buffer);
	if (!buffer)
		return -1;

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
	for (preset_index++; preset_index < kNumPresets; preset_index++)
		presets[preset_index] = Preset();
	free(buffer);

	bank_file = std::string(filename);

	return 0;
}

///////////////////////////////////

static std::vector<BankInfo> s_banks;

static void scan_preset_bank(const std::string dir_path, const std::string file_name, bool read_only)
{
	std::string file_path = dir_path + std::string("/") + std::string(file_name);

	std::string bank_name = std::string(file_name);
	if (bank_name == std::string(".amSynth.presets")) {
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

static std::string sFactoryBanksDirectory;

static void scan_preset_banks()
{
	s_banks.clear();
	scan_preset_bank(std::string(getenv("HOME")), ".amSynth.presets", false);
	scan_preset_banks(PresetController::getUserBanksDirectory(), false);
#ifdef PKGDATADIR
	if (sFactoryBanksDirectory.empty())
		sFactoryBanksDirectory = std::string(PKGDATADIR "/banks");
#endif
	if (!sFactoryBanksDirectory.empty())
		scan_preset_banks(sFactoryBanksDirectory, true);
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

void PresetController::setFactoryBanksDirectory(std::string path)
{
	sFactoryBanksDirectory = path;
	if (!s_banks.empty())
		scan_preset_banks();
}

std::string PresetController::getUserBanksDirectory()
{
	return std::string(getenv("HOME")) + std::string("/.amsynth/banks");
}
