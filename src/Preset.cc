/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "Preset.h"

#define setupTimeParam { parameters[no_p].setType( PARAM_POWER, 3, 0.0005 ); parameters[no_p].setMin( 0.0 );	parameters[no_p].setMax( 2.5 ); parameters[no_p].setLabel("secs"); }

Preset::Preset()
{
    _name = "New Preset";

    nullparam.setName("null");

    no_p = 0;

    parameters[no_p].setName("amp_attack");
	setupTimeParam
    no_p++;

    parameters[no_p].setName("amp_decay");
    setupTimeParam
    no_p++;

    parameters[no_p].setName("amp_sustain");
    parameters[no_p].setValue(1);
    no_p++;

    parameters[no_p].setName("amp_release");
    setupTimeParam
    no_p++;

    parameters[no_p].setName("osc1_waveform");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(4.0);
    parameters[no_p].setStep(1.0);
    no_p++;

    parameters[no_p].setName("filter_attack");
    setupTimeParam
    no_p++;

    parameters[no_p].setName("filter_decay");
    setupTimeParam
    no_p++;

    parameters[no_p].setName("filter_sustain");
    parameters[no_p].setValue(1);
    no_p++;

    parameters[no_p].setName("filter_release");
    setupTimeParam
    no_p++;

    parameters[no_p].setName("filter_resonance");
	parameters[no_p].setMax(1.0);
    parameters[no_p].setValue(0);
    no_p++;
	
	parameters[no_p].setName("filter_env_amount");
	parameters[no_p].setMax(16.0);
    parameters[no_p].setValue(0);
    no_p++;
    
	parameters[no_p].setName("filter_cutoff");
    parameters[no_p].setType(PARAM_EXP, 16, 0);
    parameters[no_p].setMin(-0.5);
    parameters[no_p].setMax(1);
    parameters[no_p].setValue(1);
    no_p++;

    parameters[no_p].setName("osc2_detune");
    parameters[no_p].setType(PARAM_EXP, 1.25, 0);
    parameters[no_p].setMin(-1);
    parameters[no_p].setMax(1);
    parameters[no_p].setValue(0);
    no_p++;

    parameters[no_p].setName("osc2_waveform");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(4.0);
    parameters[no_p].setStep(1.0);
    no_p++;

    parameters[no_p].setName("master_vol");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(2.0);
    parameters[no_p].setValue(1.0);
    no_p++;

    parameters[no_p].setName( "lfo_freq" );
	parameters[no_p].setType( PARAM_POWER, 2, 0 );
    parameters[no_p].setMax( 7.5 );
	parameters[no_p].setLabel( "Hz" );
    no_p++;

    parameters[no_p].setName( "lfo_depth" );
    parameters[no_p].setValue( 0 );
    no_p++;

    parameters[no_p].setName("lfo_waveform");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(4.0);
    parameters[no_p].setStep(1.0);
    no_p++;

    parameters[no_p].setName("osc2_range");
    parameters[no_p].setMin(-1);
    parameters[no_p].setMax(2);
    parameters[no_p].setStep(1.0);
    parameters[no_p].setType(PARAM_EXP, 2, 0);
    parameters[no_p].setValue(0);
    no_p++;
	
	parameters[no_p].setName("osc_mix");
    parameters[no_p].setMin(-1.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("freq_mod_amount");
	parameters[no_p].setType( PARAM_POWER, 3, -1 );
	parameters[no_p].setMin( 0.0 );
	parameters[no_p].setMax( 1.25992105 );
    no_p++;
	
	parameters[no_p].setName("filter_mod_amount");
    parameters[no_p].setMin(-1.0);
    parameters[no_p].setMax(1.0);
	parameters[no_p].setValue(-1);
    no_p++;

	parameters[no_p].setName("amp_mod_amount");
    parameters[no_p].setMin(-1.0);
    parameters[no_p].setMax(1.0);
	parameters[no_p].setValue(-1);
    no_p++;
	
	parameters[no_p].setName("osc1_pwm_amount");
    parameters[no_p].setMin(-1.0);
    parameters[no_p].setMax(1.0);
	parameters[no_p].setValue(-1);
    no_p++;
	
	parameters[no_p].setName("osc_mix_mode");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
	parameters[no_p].setStep(1.0);
    no_p++;
	
	parameters[no_p].setName("osc1_pulsewidth");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("osc2_pulsewidth");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("reverb_roomsize");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("reverb_damp");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("reverb_wet");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("reverb_dry");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
	parameters[no_p].setValue(0.5);
    no_p++;
	
	parameters[no_p].setName("reverb_width");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("reverb_mode");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(0.0);
//	parameters[no_p].setStep(1.0);
	parameters[no_p].setDiscrete(true);
    no_p++;
	
	parameters[no_p].setName("distortion_drive");
    parameters[no_p].setMin(1.0);
    parameters[no_p].setMax(16.0);
	parameters[no_p].setValue(1.0);
    no_p++;
	
	parameters[no_p].setName("distortion_crunch");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
    no_p++;
	
	parameters[no_p].setName("osc2_sync");
    parameters[no_p].setMin(0.0);
    parameters[no_p].setMax(1.0);
	parameters[no_p].setStep(1.0);
    no_p++;
}

Preset::~Preset()
{
}

void
Preset::clone(Preset & preset)
{
    for (int i = 0; i < 128; i++) {
		parameters[i].setValue(preset.getParameter(i).getValue());
		parameters[i].setName(preset.getParameter(i).getName());
    }
    setName(preset.getName());
}

string 
Preset::getName()
{
    return _name;
}

void 
Preset::setName(string name)
{
    _name = name;
}

Parameter & 
Preset::getParameter(string name)
{
    for (int i = 0; i < no_p; i++) {
		if (parameters[i].getName() == name) {
#ifdef _DEBUG
			cout << "<Preset::getParameter()> Parameter '" << name
			<< "' found" << endl;
#endif
			return parameters[i];
		}
    }
#ifdef _DEBUG
    cout << "<Preset::getParameter()> Parameter '" << name
	<< "' not found" << endl;
#endif
    return nullparam;
}

void
Preset::randomise()
{
	float master_vol = getParameter( "master_vol" ).getValue();
	for (int i=0; i<no_p; i++) 
		parameters[i].random_val();
	getParameter( "master_vol" ).setValue( master_vol );
}
