/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _GUI_H
#define _GUI_H

#include <string>
#include <gtkmm.h>

#include "PresetControllerView.h"
#include "ControllerMapDialog.h"
#include "Request.h"

class PresetController;
class Parameter;
class Preset;
class VoiceAllocationUnit;
class GenericOutput;
class Config;
class EditorPanel;

namespace Gtk {
	class Adjustment;
};


/**
 * @brief The top-level Graphical User Interface
 *
 * Remember to call setPresetController() before calling init().
 * The gui must be init()ialised before entering the main gtk execution loop.
 */
class GUI:public Gtk::Window, public UpdateListener {
public:
	GUI				( Config & config,
					  MidiController & mc, 
					  VoiceAllocationUnit & vau,
					  GenericOutput *audio,
					  const char *title );
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

	int			lnav;
	void		event_handler	(const int);
	void config_controllers();
	
	void		preset_new		( );
	void		preset_copy		( );
	void		preset_paste		( );
	void		preset_paste_as_new	( );
	
	void		bank_open		( );
	void		bank_save_as		( );

	int		command_exists		(const char *command);
	void		command_run		(const char *command);
	
	void		changed_midi_channel	( );
	void		changed_voices		( );
	
	void		post_init();
	void		update_title();
	void		UpdateParameterOnMainThread(Param, float);

	gint idle_callback();
	gint setActiveParam( GdkEventButton *event, Parameter * param );
	string status;
	Gtk::VBox vbox;

	Gtk::Style 		*style;
    	
	// top level window & main panel
	Gtk::Statusbar statusBar;
	
	Gtk::AboutDialog	aboutDlg;
	
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
		
	Gtk::Adjustment		*adj_midi,
				*adj_voices;

	Parameter *active_param;
	PresetController *preset_controller;
	PresetControllerView *presetCV;
    
	Config *config;
	MidiController *midi_controller;
	VoiceAllocationUnit *vau;
	ControllerMapDialog *controller_map_dialog;
	GenericOutput *audio_out;
	
	EditorPanel		*editor_panel;
	
	Preset			*clipboard_preset;
        
        std::string             m_baseName;
	bool			m_presetIsNotSaved;
};

#endif

