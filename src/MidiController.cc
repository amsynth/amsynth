/* amSynth 
 * (c) 2001,2002 Nick Dowell 
 */

#include "MidiController.h"

MidiController::MidiController()
{
    running = 0;
    buffer = new unsigned char[MIDI_BUF_SIZE];
    presetController = 0;
    _va = 0;
	for( int i=0; i<32; i++ ) midi_controllers[i] = 0;
}

MidiController::~MidiController()
{
    delete[]buffer;
}

void
MidiController::setPresetController(PresetController & pc)
{
    presetController = &pc;
	midi_controllers[1] = &(presetController->getCurrentPreset().getParameter("freq_mod_amount"));
	midi_controllers[7] = &(presetController->getCurrentPreset().getParameter("master_vol"));
}

void
MidiController::setVAU(VoiceAllocationUnit & vau)
{
    _va = &vau;
}

void
MidiController::run()
{
    running = 1;
#ifdef _DEBUG
    cout << "<MidiController> opening midi interface.." << endl;
#endif
    if (midi.open( config->midi_device ) == -1) {
	cout << "<MidiController> failed to open midi." << endl;
	exit(-1);
    }
#ifdef _DEBUG
    else
	cout << "<MidiController> opened midi interface." << endl;

    cout << "<MidiController> entering doMidi() loop.." << endl;
#endif

    while (running)
	doMidi();

    midi.close();
}

void
MidiController::stop()
{
#ifdef _DEBUG
    cout << "<MidiController::stop()>" << endl;
#endif
    running = 0;
}

void
MidiController::doMidi()
{
    if ((bytes_read = midi.read(buffer)) == -1) {
	cout << "error reading from midi device" << endl;
	exit(-1);
    }

    int receiveChannel = config->midi_channel;

    for (int i = 0; i < bytes_read; i++) {
	byte = buffer[i];
#ifdef _DEBUG
	if (byte < 0xf0)
	    cout << "raw_midi: " << hex << (int) byte << endl;
#endif
	if (byte & 0x80) {	// then byte is a status byte
	    if (byte < 0xf0) {	// dont deal with system messages
		status = byte;
		channel = (byte & 0x0f);
		data = 0xff;
	    }
	    continue;
	}
	// so we have a data byte

	// this filters out messages on other channels:
	if (receiveChannel && ((int) (status & 0x0f) != receiveChannel-1)) 
		break;

	switch (status & 0xf0) {

	case 0x80:
	    // note off.
	    // data byte 1 = note, data byte 2 = velocity

	    // NOTE - many (most?) devices send a 'note on' event with
	    // velocity
	    // 0 rather than a distinct 'note off' event...
	    if (data == 0xff) {
		data = byte;
		break;
	    }
	    dispatch_note(channel, data, byte);
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
		if (presetController && channel == receiveChannel){
		// this is broken, due to threading issues..
			presetController->selectPreset((int) byte);
#ifdef _DEBUG
			cout << "<MidiController> program change: " << (int) byte <<
			endl;
#endif
			data = 0xff;
		}
	    break;

	case 0xd0:
	    // channel pressure
	    // data byte = pressure (after-touch)
	    break;

	case 0xe0:
	    // pitch wheel
	    // 2 data bytes give a 14 bit value, least significant 7 bits
	    // first
	    if (data == 0xFF)
			data = byte;
	    else {
			int bend = (int) ((data & 0x7F) | ((byte & 0x7F) << 7));
			float fbend = (float) (bend - 0x2000) / (float) (0x2000);
			pitch_wheel_change(fbend);
			data = 0xFF;
	    }
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
    if (_va)
		_va->pwChange(val);
}

void
MidiController::dispatch_note(unsigned char ch, unsigned char note,
			      unsigned char vel)
{
#ifdef _DEBUG
    cout << "<MidiController> note: " << (int) note
	<< " velocity: " << (int) vel << endl;
#endif
    ch = 0;
    float           v = (float) vel / (float) 127;
    if (_va) {
	if ((int) vel == 0)
	    _va->noteOff((int) note);
	else
	    _va->noteOn((int) note, v);
    }
}

void
MidiController::controller_change(unsigned char controller,
				  unsigned char value)
{
    // controllers:
    // 0 - 31 continuous controllers 0 - 31, most significant byte
    // 32 - 63 continuous controllers 0 - 31, least significant byte
    // 64 - 95 on / off switches
    // 96 - 121 unspecified, reserved for future.
    // 122 - 127 channel mode messages
    // Controller number 1 IS standardized to be the modulation wheel.

    // from experimentation (with yamaha keyboards)
    // controller 0x40 (64) is the sustain pedal. (0 if off, 127 is on)

    switch (controller) {
		
		case 64:
		// sustain pedal
			if (!value) _va->sustainOff();
			else _va->sustainOn();
			break;

		default:
			float fval = value/(float)127;
//			cout << "<MidiController> controller: " << (float) controller
//				<< " value: " << (float) value << "fval " << fval << endl;
			if( controller<32)
				if(midi_controllers[controller]){
					cout << midi_controllers[controller]->getName() << endl;
					midi_controllers[controller]->setValue( 
						fval*(midi_controllers[controller]->getMax()-midi_controllers[controller]->getMin())
						+ midi_controllers[controller]->getMin() );
				}
			break;
    }
	return;
}

void
MidiController::setController( int controller_no, Parameter & param )
{
	if(controller_no<32)
		midi_controllers[controller_no] = &param;
}

Parameter &
MidiController::getController( int controller_no )
{
	if(controller_no>32) return presetController->getCurrentPreset().getParameter("null");
	else return *midi_controllers[controller_no];
}