/* amSynth
 * (c) 2003 Nick Dowell
 */

#ifndef _EDITOR_PANEL_H
#define _EDITOR_PANEL_H

#include <gtk--/box.h>
#include <gtk--/frame.h>
#include <gtk--/fixed.h>
#include <gtk--/style.h>

// forward definition of classes: speeds up compilation

class Preset;
class ParameterKnob;
class ParameterSwitch;
class RadioButtonParameterView;



class EditorPanel : public Gtk::Fixed
{
public:
			EditorPanel	( Preset* preset, int piped );
			~EditorPanel	( );
	
	void		set_x_font	( const char *x_font_desc );
	void		arrange		( );

private:
	virtual void	realize_impl	( );
	



	ParameterKnob*			parameterView[32];
	RadioButtonParameterView*	rb_pv[10];
	ParameterSwitch*		param_switch;

	Gtk::Style			*style;
		
	// oscillator controls
	Gtk::Frame 			osc1_frame,
					osc2_frame,
					osc_mix_frame;
	Gtk::Fixed 			osc1_fixed,
					osc2_fixed;
	
	// random stuff
	Gtk::Frame			reverb_frame,
					distortion_frame,
					filter_frame,
					amp_frame;
	Gtk::VBox 			osc1_vbox,
					osc2_vbox,
					osc_mix_vbox,
					filter_vbox,
					amp_vbox;
	Gtk::HBox			filter_hbox1,
					filter_hbox2,
					amp_hbox1,
					amp_hbox2,
					reverb_hbox,
					distortion_hbox;
    
	// modulation controls
	Gtk::Frame			mod_frame;
	Gtk::HBox			mod_hbox;

	
};

#endif
