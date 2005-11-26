/* amSynth
 * (c) 2003 Nick Dowell
 */

#ifndef _EDITOR_PANEL_H
#define _EDITOR_PANEL_H

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/fixed.h>
#include <gtkmm/style.h>

// forward definition of classes: speeds up compilation

class Preset;
class ParameterKnob;
class ParameterSwitch;
class RadioButtonParameterView;



class EditorPanel : public Gtk::VBox
{
 public:
  EditorPanel	( Preset* preset, int piped );
  ~EditorPanel	( );
	
  void		set_x_font	( const char *x_font_desc );

 private:
  virtual void	on_realize	( );
	



  ParameterKnob*			parameterView[32];
  RadioButtonParameterView*	rb_pv[10];
  ParameterSwitch*		param_switch;

  Gtk::Style			*style;
		
  // oscillator controls
  Gtk::Frame 			osc1_frame,
    osc2_frame,
    osc_mix_frame;
	
  // random stuff
  Gtk::Frame			reverb_frame,
    distortion_frame,
    filter_frame,
    amp_frame;
  Gtk::VBox osc_mix_vbox,
    filter_vbox,
    amp_vbox;
  
  Gtk::HBox filter_hbox1,
    filter_hbox2,
    amp_hbox1,
    amp_hbox2,
    reverb_hbox,
    distortion_hbox,
    top_box,
    bottom_box;
    
  // modulation controls
  Gtk::Frame			mod_frame;
  Gtk::HBox			mod_hbox;
};

#endif
