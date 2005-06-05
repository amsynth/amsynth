/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

#include "PresetController.h"

PresetController::PresetController	()
:	updateListener (0)
,	nullpreset ("null preset")
,	currentPresetNo (0)
{
	presets = new Preset [kNumPresets];
#ifndef _WINDOWS
	bank_file = string (getenv ("HOME")) + "/.amSynth.presets";
#endif
}

PresetController::~PresetController	()
{
	delete[] presets;
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
	}
    return 0;
}

int 
PresetController::selectPreset		(const string name)
{
	for (int i=0; i<kNumPresets; i++) if (getPreset(i).getName() == name) return selectPreset (i);
	return -1;
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
}

int
PresetController::exportPreset		(const string filename)
{
	ofstream file( filename.c_str(), ios::out );
	file << "amSynth1.0preset" << endl;
	
	file << "<preset> " << "<name> " << currentPreset.getName() << endl;
	for (unsigned n = 0; n < currentPreset.ParameterCount(); n++)
	{
		file << "<parameter> " << currentPreset.getParameter(n).getName() 
		<< " " << currentPreset.getParameter(n).getValue() << endl;
	}
	file.close();
	
	return 0;
}

int
PresetController::importPreset		(const string filename)
{	
	ifstream file( filename.c_str(), ios::in );
	char buffer[100];
  
	if (file.bad())	return -1;
  
	file >> buffer;
  
	if (string(buffer) != "amSynth1.0preset") return -1;
  
	file >> buffer;
	if (string(buffer) == "<preset>") {
		file >> buffer;
		
		//get the preset's name
		file >> buffer;
		string presetName = "Imported: ";
		presetName += string(buffer);
		file >> buffer;
		while (string(buffer) != "<parameter>") {
			presetName += " ";
			presetName += string(buffer);
			file >> buffer;
		}
		currentPreset.setName(presetName); 
		//get the parameters
		while (string(buffer) == "<parameter>") {
			string name;
			file >> buffer;
			name = string(buffer);
			file >> buffer;
			if(name!="unused")
				currentPreset.getParameter(name).setValue( atof(buffer) );
			file >> buffer;
		}
		currentPreset.setName(presetName);
		notify ();
	} else file.close();
	return 1;
}


int 
PresetController::savePresets		(const char *filename)
{
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
	return 0;
}

int 
PresetController::loadPresets		(const char *filename)
{
#ifdef _DEBUG
	cout << "<PresetController::loadPresets()>" << endl;
#endif
	ifstream file( filename, ios::in );
	char buffer[100];
  
	if (file.bad()) {
#ifdef _DEBUG
		cout << "<PresetController::loadPresets()> file.bad()" << endl;
#endif
		return -1;
	}
  
	file >> buffer;
  
	if (string(buffer) != "amSynth") {
#ifdef _DEBUG
		cout <<
		"<PresetController::loadPresets()> not an amSynth file, bailing out"
		<< endl;
#endif
	return -1;
	}
  
	int preset = -1;
	file >> buffer;
	while (file.good()) {
		if (string(buffer) == "<preset>") {
			preset++;
			file >> buffer;
			
			//get the preset's name
			file >> buffer;
			string presetName = string(buffer);
			file >> buffer;
			while (string(buffer) != "<parameter>") {
				presetName += " ";
				presetName += string(buffer);
				file >> buffer;
			}
#ifdef _DEBUG
			cout << "<PresetController::loadPresets()>: Preset name: "
			<< presetName << endl;
#endif
			presets[preset].setName(presetName);
			//get the parameters
			while (string(buffer) == "<parameter>") {
				string name;
				file >> buffer;
				name = string(buffer);
				file >> buffer;
#ifdef _DEBUG
				cout << "<PresetController::loadPresets()>: Parameter:- name="
				<< name << " value=" << buffer << endl;
#endif
				if(name!="unused")
					presets[preset].getParameter(name).setValue( atof(buffer) );
				file >> buffer;
			}
		} else {
			file.close();
		}
	}

	currentPreset = presets[currentPresetNo];
	notify ();
#ifdef _DEBUG
	cout << "<PresetController::loadPresets()>: success" << endl;
#endif
	return 0;
}
