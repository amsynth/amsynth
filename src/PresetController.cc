/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string>
#include <fstream>

#include "PresetController.h"

using namespace std;


PresetController::PresetController	()
:	updateListener (0)
,	nullpreset ("null preset")
,	currentPresetNo (0)
{
	presets = new Preset [kNumPresets];
#ifndef _MSC_VER
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
	string buffer;
  
	if (file.bad()) {
#ifdef _DEBUG
		cout << "<PresetController::loadPresets()> file.bad()" << endl;
#endif
		return -1;
	}
  
	file >> buffer;
  
	if (buffer != "amSynth") {
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
		if (buffer == "<preset>") {
			preset++;
			file >> buffer;
			
			//get the preset's name
			file >> buffer;
			string presetName(buffer);
			file >> buffer;
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

	currentPreset = presets[currentPresetNo];
	notify ();
#ifdef _DEBUG
	cout << "<PresetController::loadPresets()>: success" << endl;
#endif
	return 0;
}
