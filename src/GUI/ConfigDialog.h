/*
 *  ConfigDialog.h
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

#ifndef _ConfigDialog_h
#define _ConfigDialog_h

#include <gtkmm.h>
#include "../Configuration.h"

class ConfigDialog : public Gtk::Dialog
{
public:
	ConfigDialog (Gtk::Window& parent, Configuration &);
	
protected:
	void ReadValues ();
	void WriteValues ();
	virtual void on_response (int);

private:
	Configuration &		mConfig;
	Gtk::ComboBoxText	mMidiDriver;
	Gtk::ComboBoxText	mAudioDriver;
	Gtk::ComboBoxText	mSampleRate;
	Gtk::Entry		mOSSMidiDevice;
	Gtk::Entry		mOSSAudioDevice;
	Gtk::Entry		mALSAAudioDevice;
};

#endif
