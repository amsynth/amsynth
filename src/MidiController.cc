/* amSynth 
 * (c) 2001-2006 Nick Dowell 
 */

#include "MidiController.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <assert.h>

using namespace std;

MidiController::MidiController( Config & config )
:	last_active_controller ("last_active_cc", (Param) -1, 0, 0, MAX_CC, 1)
,	_handler(NULL)
{
	this->config = &config;
	presetController = 0;
	channel = config.midi_channel;
	for( int i=0; i<MAX_CC; i++ ) midi_controllers[i] = 0;
}

MidiController::~MidiController()
{
}

void
MidiController::setPresetController(PresetController & pc)
{
	presetController = &pc;
	
	for(int i=0; i<MAX_CC; i++)
		midi_controllers[i] = &(presetController->getCurrentPreset().
				getParameter("null"));

	midi_controllers[1] = &(presetController->getCurrentPreset().
			getParameter("freq_mod_amount"));
	midi_controllers[7] = &(presetController->getCurrentPreset().
			getParameter("master_vol"));
	
	// load controller mapping config. from file	
	string fname(getenv("HOME"));
	fname += "/.amSynthControllersrc";
	ifstream file(fname.c_str(), ios::out);
	int i=0;
	char buffer[100];
	if (file.bad())	return;
  
	file >> buffer;
	while( file.good() )
	{
		midi_controllers[i++] = &(presetController->getCurrentPreset().
				getParameter( string(buffer) ));
		file >> buffer;
	}
	file.close();
}

void
MidiController::HandleMidiData(unsigned char* bytes, unsigned numBytes)
{
    for (unsigned i=0; i<numBytes; i++)
	{
		const unsigned char byte = bytes[i];
		
		if (byte & 0x80) {	// then byte is a status byte
			if (byte < 0xf0) {	// dont deal with system messages
			status = byte;
			channel = (byte & 0x0f);
			data = 0xff;
			}
			continue;
		}
		// now we have at least one data byte

		if (config->midi_channel && ((int) channel != config->midi_channel-1)) break;

		switch (status & 0xf0)
		{
		case 0x80:
			// note off : data byte 1 = note, data byte 2 = velocity
			// N.B. many devices send a 'note on' event with 0 velocity
			// rather than a distinct 'note off' event.
			if (data == 0xff) { // wait until we receive the second data byte
				data = byte;
				break;
			}
			dispatch_note( channel, data, 0 );
			data = 0xff;
			break;
	
		case 0x90:
			// note on.
			// data byte 1 = note, data byte 2 = velocity
			if (data == 0xff) {
				data = byte;
				break;
			}
			dispatch_note(channel, data, byte);
			data = 0xff;
			break;
	
		case 0xa0:
			// key pressure. 
			// data byte 1 = note, data byte 2 = pressure (after-touch)
			if (data == 0xff) { // wait until we receive the second data byte
				data = byte;
				break;
			}
			data = 0xff;
			break;

		case 0xb0:
			// parameter change
			// data byte 1 = controller, data byte 2 = value
			if (data == 0xFF) {
				data = byte;
				break;
			}
			controller_change(data, byte);
			data = 0xFF;
			break;

		case 0xc0:
			// program change
//			if (_handler) {
//				_handler->HandleMidiAllSoundOff();
//				_handler->HandleMidiProgramChange(byte);
//			}
			if( presetController->getCurrPresetNumber() != byte )
			{
				if (_handler) _handler->HandleMidiAllSoundOff();
				presetController->selectPreset((int) byte);
			}
			data = 0xff;
			break;
	
		case 0xd0:
			// channel pressure
			// data byte = pressure (after-touch)
			data = 0xff;
			break;
	
		case 0xe0:
			// pitch wheel
			// 2 data bytes give a 14 bit value, least significant 7 bits
			// first
			if (data == 0xFF) {
				data = byte;
				break;
			}
			int bend; bend = (int) ((data & 0x7F) | ((byte & 0x7F) << 7));
			float fbend; fbend = (float) (bend - 0x2000) / (float) (0x2000);
			pitch_wheel_change(fbend);
			data = 0xFF;
			break;
	
		default:
#ifdef _DEBUG
			cout << "<MidiController> unknown status :" << hex <<
			(int) status << "for data byte: " << hex << (int) byte <<
			endl;
#endif
			break;
		}
    }
}

void
MidiController::pitch_wheel_change(float val)
{
	if (_handler) _handler->HandleMidiPitchWheel(val);
}

void
MidiController::dispatch_note(unsigned char, unsigned char note, unsigned char vel)
{
	static const float scale = 1.f/127.f;
    if (!_handler) return;
	if (vel) _handler->HandleMidiNoteOn((int) note, (float)vel * scale);
	else     _handler->HandleMidiNoteOff((int) note, (float)vel * scale);
}

void
MidiController::controller_change(unsigned char cc, unsigned char value)
{
	static const float scale = 1.f/127.f;
    // controllers:
    // 0 - 31 continuous controllers 0 - 31, most significant byte
    // 32 - 63 continuous controllers 0 - 31, least significant byte
    // 64 - 95 on / off switches
    // 96 - 121 unspecified, reserved for future.
    // 122 - 127 channel mode messages
    // Controller number 1 IS standardized to be the modulation wheel.

    // from experimentation (with yamaha keyboards)
    // controller 0x40 (64) is the sustain pedal. (0 if off, 127 is on)
    switch (cc) {
		
		case 64:
		// sustain pedal
			if (_handler) _handler->HandleMidiSustainPedal(value);
			break;
			
		case 120:	// ALL SOUND OFF
			if (value == 0) _handler->HandleMidiAllSoundOff();
			break;
			
		case 123:	// ALL NOTES OFF
			if (value == 0) _handler->HandleMidiAllNotesOff();
			break;

		default:
			if( last_active_controller.getValue() != cc ) {
				last_active_controller.setValue( cc );
			}
			getController(cc).SetNormalisedValue(value*scale);
			break;
    }
	return;
}

void
MidiController::setController( unsigned int controller_no, Parameter & param )
{
	if(controller_no<MAX_CC)
		midi_controllers[controller_no] = &param;
}

Parameter&
MidiController::getController( unsigned int idx )
{
    assert(idx < MAX_CC);
    return (idx < MAX_CC) ? *midi_controllers[idx] : presetController->getCurrentPreset().getParameter("null");
}

int
MidiController::getControllerForParam(unsigned paramIdx)
{
	for (unsigned int i=0; i<MAX_CC; i++) {
		if (midi_controllers[i] &&
			midi_controllers[i]->GetId() == i) {
			i;
		}
	}
	return -1;
}

void
MidiController::saveConfig()
{
	string fname(getenv("HOME"));
	fname += "/.amSynthControllersrc";
	ofstream file(fname.c_str(), ios::out);
	if (file.bad())	return;
  
	for(int i=0; i<MAX_CC; i++){
		file << midi_controllers[i]->getName() << endl;
	}
	file.close();
}

void
MidiController::set_midi_channel	( int ch )
{
	if (ch)	_handler->HandleMidiAllSoundOff();
	config->midi_channel = ch;
}

int
MidiController::sendMidi_values       ()
{
	if (!_midiIface)
	{
		std::cerr << "error: cannot send midi values, no midi output interface present\n";
		return -1;
	}

      for (unsigned i = 0; i < MAX_CC; i++)
      {
              if (midi_controllers[i]->getName() != "null")
              {
      
                      float fval=midi_controllers[i]->getValue();
                      int midiValue=(int)(127*(fval-midi_controllers[i]->getMin())/(midi_controllers[i]->getMax()-midi_controllers[i]->getMin()));
#ifdef _DEBUG
                      cout << "PresetController::midiSendPresets :- parameter name="
                      << midi_controllers[i]->getName() << " value= "
                      << midi_controllers[i]->getValue() 
                      << " midiValue = " << midiValue << endl;
#endif
                      if (_midiIface) _midiIface->write_cc(0,i,midiValue);
              }
      }
      return 0;
}



