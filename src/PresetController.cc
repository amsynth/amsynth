/* amSynth
 * (c) 2001 Nick Dowell
 */

#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include "PresetController.h"

PresetController::PresetController()
{
    updateListener = 0;
    presets = new Preset[PRESETS];
	currentPresetNo = 0;
	nullpreset.setName("null preset");
}

PresetController::~PresetController()
{
    delete[]presets;
}

int
PresetController::selectPreset(int preset)
{
#ifdef _DEBUG
    cout << "<PresetController::selectPreset( " << preset << " )" << endl;
#endif
    if (preset > (PRESETS - 1) || preset < 0) {
#ifdef _DEBUG
		cout << "<PresetController::selectPreset( int ) out of range: " <<
	    preset << endl;
#endif
		return -1;
    }
	if (preset!=currentPresetNo){
		currentPreset.clone(presets[preset]);
		currentPresetNo = preset;
		if (updateListener) 
			updateListener->update();
	}
    return 0;
}

Preset &
PresetController::getPreset( int preset )
{
	return presets[preset];
}

Preset &
PresetController::getPreset( string name )
{
	for (int i = 0; i < PRESETS; i++)
		if (presets[i].getName() == name)
			return presets[i];
	return nullpreset;
}

void
PresetController::commitPreset()
{
#ifdef _DEBUG
	cout << "<PresetController::commitPreset()>" << endl;
#endif
	presets[currentPresetNo].clone(currentPreset);
}

int
PresetController::newPreset()
{
	for(int i=0; i<PRESETS; i++){		if(presets[i].getName() == "New Preset"){
			selectPreset(i);
			updateListener->update();
			return 0;
		}
	}
	return -1;
}

void
PresetController::deletePreset()
{
	currentPreset.clone( blankPreset );
	updateListener->update();
}

int 
PresetController::selectPreset(string name)
{
	for (int i = 0; i < PRESETS; i++) {
		if (presets[i].getName() == name) {
			selectPreset(i);
			return 0;
		} else {
#ifdef _DEBUG
			cout << "<PresetController::selectPreset(" << name << ") failed" << endl;
#endif
		}
	}
	return -1;
}

Preset & 
PresetController::getCurrentPreset()
{
    return currentPreset;
}

int 
PresetController::savePresets()
{
#ifdef _DEBUG
	cout << "<PresetController::savePresets()" << endl;
#endif
	string fname(getenv("HOME"));
	fname += "/.amSynth.presets";
	ofstream file(fname.c_str(), ios::out);
  
	file << "amSynth" << endl;
	for (int i = 0; i < PRESETS; i++) {
		if (presets[i].getName()!="unused"){
#ifdef _DEBUG
			cout << "<PresetController::savePresets():- preset: name= "
			<< presets[i].getName() << endl;
#endif
			file << "<preset> " << "<name> " << presets[i].getName() << endl;
			for (int n = 0; n < 128; n++) {
				if(presets[i].getParameter(n).getName()!="unused") {
					// discard unused Parameters
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
	}
	file << "EOF" << endl;
	file.close();
#ifdef _DEBUG
	cout << "<PresetController::savePresets() success" << endl;
#endif
	return 0;
}

int 
PresetController::loadPresets()
{
#ifdef _DEBUG
	cout << "<PresetController::loadPresets()>" << endl;
#endif
  
	string fname(getenv("HOME"));
	fname += "/.amSynth.presets";
	ifstream file(fname.c_str(), ios::in);
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

	currentPreset.clone(presets[currentPresetNo]);
	if (updateListener) updateListener->update();
#ifdef _DEBUG
	cout << "<PresetController::loadPresets()>: success" << endl;
#endif
	return 0;
}

void 
PresetController::setUpdateListener(UpdateListener & ul)
{
	updateListener = &ul;
}

