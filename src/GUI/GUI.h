/* amSynth
 * (c) 2001-2003 Nick Dowell
 */

#ifndef _GUI_H
#define _GUI_H

#include <string>
#include <gtk--/box.h>
#include <gtk--/main.h>
#include <gtk--/frame.h>
#include <gtk--/fixed.h>
#include <gtk--/style.h>
#include <gtk--/dialog.h>
#include <gtk--/menu.h>
#include <gtk--/menubar.h>
#include <gtk--/menuitem.h>
#include <gtk--/tearoffmenuitem.h>
#include <gtk--/label.h>
#include <gtk--/button.h>
#include <gtk--/statusbar.h>
#include <gtk--/pixmap.h>
#include <gtk--/fileselection.h> 
#include <gtk--/fontselection.h>

#include "../PresetController.h"
#include "PresetControllerView.h"
#include "ControllerMapDialog.h"
#include "../UpdateListener.h"
#include "../Parameter.h"
#include "../Config.h"
#include "../VoiceAllocationUnit.h"
#include "../AudioOutput.h"


class EditorPanel;

/**
 * @brief The top-level Graphical User Interface
 *
 * Remember to call setPresetController() before calling init().
 * The gui must be init()ialised before entering the main gtk execution loop.
 */
class GUI:public Gtk::Window, public UpdateListener {
public:
	GUI				( Config & config, MidiController & mc, 
					VoiceAllocationUnit & vau, int pipe[2],
					GenericOutput *audio, const char *title );
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
	void	set_x_font		( const char *x_font_name );
	string	get_x_font		( )	{ return xfontname; };
	int	delete_event_impl	(GdkEventAny *);
	int	delete_events		(GdkEventAny *, Gtk::Window *dialog)
					{ dialog->hide_all(); return 0; };
	void	update();
	void	serve_request();
private:
	void		realize_impl		( );
	Gtk::MenuBar*	create_menus		( );

	int *pipe, lnav;
	void event_handler(string text);
	void arrange();
	void config_controllers();
	
	void		preset_new		( );
	void		preset_copy		( );
	void		preset_paste		( );
	void		preset_paste_as_new	( );
	
	void		bank_open		( );
	void		bank_open_ok		( );
//	void		bank_save		( );
	void		bank_save_as		( );
	void		bank_save_as_ok		( );

	gint idle_callback();
	gint setActiveParam( GdkEventButton *event, Parameter * param );
	string status;
	Gtk::VBox vbox;

	Gtk::Style 		*style;
    	
	// top level window & main panel
	Gtk::Statusbar statusBar;
	
	
	// about dialog
	Gtk::Dialog about_window;
	Gtk::Label about_label;
	Gtk::Pixmap *about_pixmap;
	Gtk::Button about_close_button;
	
	// realtime warning dialog
	Gtk::Dialog realtime_warning;
	Gtk::Label realtime_text_label;
	Gtk::Button realtime_close_button;
	
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

	// delete preset dialog
	Gtk::Dialog preset_delete;
	Gtk::Label preset_delete_label;
	Gtk::Button preset_delete_ok, preset_delete_cancel;
	
	// export/import preset dialog
	Gtk::FileSelection preset_export_dialog;
	Gtk::FileSelection preset_import_dialog;
	
	// export/import preset dialog
	Gtk::FileSelection	d_bank_open;
	Gtk::FileSelection	d_bank_save_as;
	
	// recording dialog
	Gtk::Window		record_dialog;
	Gtk::VBox		record_vbox;
	Gtk::Label		record_label;
	Gtk::Frame		record_file_frame;
	Gtk::Entry		record_entry;
	Gtk::Button		record_pause, record_record, record_choose;
	Gtk::HBox		record_buttons_hbox, record_file_hbox;
	Gtk::FileSelection	record_fileselect;
	Gtk::Statusbar		record_statusbar;
	gboolean		record_recording;
	
	// quit confirmation dialog
	Gtk::Dialog		quit_confirm;
	Gtk::Label		quit_confirm_label;
	Gtk::Button		quit_confirm_ok, quit_confirm_cancel;
	
	// font selection
	Gtk::FontSelectionDialog	font_sel;
	string				xfontname;

	Parameter *active_param;
	PresetController *preset_controller;
	PresetControllerView *presetCV;
    
	Config *config;
	MidiController *midi_controller;
	VoiceAllocationUnit *vau;
	ControllerMapDialog *controller_map_dialog;
	GenericOutput *audio_out;
	
	EditorPanel		*editor_panel;
	
	Preset			clipboard_preset;
};
#endif
