/*
 * ConfigDialog.h
 * amSynth
 * (C) 2006 Nick Dowell
 */

#ifndef _ConfigDialog_h
#define _ConfigDialog_h

#include <gtkmm.h>
#include "../Config.h"

class ConfigDialog : public Gtk::Dialog
{
public:
	ConfigDialog (Gtk::Window& parent, Config &);
	
protected:
	void ReadValues ();
	void WriteValues ();
	virtual void on_response (int);

private:
	Config &		mConfig;
	Gtk::ComboBoxText	mMidiDriver;
	Gtk::ComboBoxText	mAudioDriver;
	Gtk::ComboBoxText	mSampleRate;
	Gtk::Entry		mOSSMidiDevice;
	Gtk::Entry		mOSSAudioDevice;
	Gtk::Entry		mALSAAudioDevice;
};

#endif
