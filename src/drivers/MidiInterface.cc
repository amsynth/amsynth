/* amSynth
 * (c) 2001 Nick Dowell
 **/

#include "MidiInterface.h"

int
 MidiInterface::read(unsigned char *buffer)
{
    return midi->read(buffer);
}

void MidiInterface::close()
{
  midi->close();
#ifdef _DEBUG
  cout << "<MidiInterface::close()> closed Midi Device" << endl;
#endif 
  //  delete midi; // this is (was) causing a Segfault....
}

int MidiInterface::open(string device)
{

    //try ALSA
    midi = new ALSAMidiDriver;
#ifdef _DEBUG
    cout << "<MidiInterface> Trying to open ALSA midi device..." << endl;
#endif
    if ((midi->open(device)) == 0) {
#ifdef _DEBUG
	cout << "<MidiInterface> opened ALSA midi device! :-)" << endl;
#endif
	return 0;
    }
#ifdef _DEBUG
    cout << "<MidiInterface> failed to open ALSA midi device, " <<
	"falling back to OSS.." << endl;
#endif
    delete midi;

    //try OSS
    midi = new OSSMidiDriver;
    if ((midi->open(device)) == 0) {
#ifdef _DEBUG
	cout << "<MidiInterface> opened OSS midi device! :-)" << endl;
#endif
	return 0;
    }

    cout << "<MidiInterface> failed to open OSS midi device." << endl;
    cout << "<MidiInterface> couldn't open any MIDI drivers :-(" << endl;
    return -1;
}

MidiInterface::MidiInterface()
{
}

MidiInterface::~MidiInterface()
{
    close();
}
