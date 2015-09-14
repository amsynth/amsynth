/*
 *  GUI.h
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

#ifndef _GUI_H
#define _GUI_H

#include <string>
#include <gtkmm.h>

#include "../MidiController.h"
#include "Request.h"

class PresetController;
class PresetControllerView;
class Parameter;
class Preset;
class Synthesizer;
class GenericOutput;
class Config;

namespace Gtk {
	class Adjustment;
};

struct UndoArgs {
	PresetController *presetController;
	Parameter *parameter;

	UndoArgs(PresetController *nPresetController, Parameter *nParameter)
	:presetController(nPresetController),
	parameter(nParameter) {}
};


/**
 * @brief The top-level Graphical User Interface
 *
 * Remember to call setPresetController() before calling init().
 * The gui must be init()ialised before entering the main gtk execution loop.
 */
class GUI:public Gtk::Window, public UpdateListener {
public:
	GUI				( Configuration & config,
					  MidiController & mc, 
					  Synthesizer *synth,
					  GenericOutput *audio );
	~GUI				( );
	/**
	 * Sets up all the Interface controls etc..
	 * Must be called after setPresetController().
	 */
	void	init			();
	void	run			();
	/**
	 * The gui needs to be told about the PresetController for the overall
	 * system, to allow communication between it and the system.
	 */
	void	setPresetController	(PresetController & p_c);

	int	delete_event_impl	(GdkEventAny *);
	int	delete_events		(GdkEventAny *, Gtk::Window *dialog)
					{ dialog->hide_all(); return 0; };
	void	update();
        void    onUpdate();
	
	virtual void	UpdateParameter(Param, float);

protected:
	virtual void	on_hide () { Gtk::Main::quit(); }
	
private:
	Gtk::MenuBar*	create_menus		( );

	void		event_handler	(const int);
	
	void		preset_new		( );
	void		preset_copy		( );
	void		preset_paste		( );
	void		preset_paste_as_new	( );
	
	void		bank_open		( );
	void		bank_save_as		( );

	void		scale_open		( );
	void		key_map_open		( );
	void		tuning_reset		( );

	int		command_exists		(const char *command);
	void		command_run		(const char *command);
	
	void		on_midi_channel_change	(int value);
	void		on_ployphony_change		(int value, Gtk::RadioMenuItem *item);
	void		on_pitch_bend_range_menu_show();
	void		on_pitch_bend_range_change(int value, Gtk::RadioMenuItem *item);
	
	void		post_init();
	void		update_title();
	void		UpdateParameterOnMainThread(Param, float);
	
	static void preset_paste_callback(GtkClipboard *clipboard, const gchar *text, gpointer data);
	static void preset_paste_as_new_callback(GtkClipboard *clipboard, const gchar *text, gpointer data);

	gint setActiveParam( GdkEventButton *event, Parameter * param );
	std::string status;
	Gtk::VBox vbox;

	Gtk::Style 		*style;
    	
	// rename preset dialog
	Gtk::Dialog preset_rename;
	Gtk::Label preset_rename_label;
	Gtk::Entry preset_rename_entry;
	Gtk::Button preset_rename_cancel, preset_rename_ok;
	
	// new preset dialog
	Gtk::Dialog		d_preset_new;
	Gtk::Label		d_preset_new_label;
	Gtk::Entry		d_preset_new_entry;
	Gtk::Button		d_preset_new_cancel, d_preset_new_ok;
	
	// recording dialog
	Gtk::Window		record_dialog;
	Gtk::VBox		record_vbox;
	Gtk::Label		record_label;
	Gtk::Frame		record_file_frame;
	Gtk::Entry		record_entry;
	Gtk::Button		record_pause, record_record, record_choose;
	Gtk::HBox		record_buttons_hbox, record_file_hbox;
	Gtk::Statusbar		record_statusbar;
	gboolean		record_recording;
		
	PresetController *preset_controller;
	PresetControllerView *presetCV;
    
	Configuration *config;
	MidiController *midi_controller;
	Synthesizer *m_synth;
	GenericOutput *audio_out;
	
	Preset			*clipboard_preset;
	
	GtkAdjustment 	*m_adjustments[kAmsynthParameterCount];
	UndoArgs		*m_undoArgs[kAmsynthParameterCount];

	bool			m_presetIsNotSaved;
	bool			m_auditionKeyDown;

	Gtk::Menu		*m_pitchBendRangeMenu;
};

#endif
