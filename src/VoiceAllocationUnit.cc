/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "VoiceAllocationUnit.h"
#include <iostream>
#include <math.h>

VoiceAllocationUnit::VoiceAllocationUnit( Config & config ) :
	limiter(config.sample_rate)
{
	this->config = &config;
	max_voices = config.polyphony;
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> new VAU created" << endl;
#endif
	for (int i = 0; i < 128; i++) {
		keyPressed[i] = 0;
		_voices[i] = new VoiceBoard(config.sample_rate);
		// voices are initialised in setPreset() below...
	}
  
	sustain = 0;
	pw_val.setValue(1);
	
	// wire up the components
	zero.setValue( 0.0 );

	mixer.addInput( zero );
	
	amp.addInput( mixer );
	amp.addInput( master_vol );

	distortion.setInput( amp );

	reverb.setInput( distortion );
	
	limiter.isStereo();
	limiter.setInput( reverb );
}

void
VoiceAllocationUnit::setPreset( Preset & preset )
{
	_preset = &preset;
	master_vol.setParameter( _preset->getParameter("master_vol") );
	distortion.setDrive( _preset->getParameter("distortion_drive") );
	distortion.setCrunch( _preset->getParameter("distortion_crunch") );
	reverb.setDamp( _preset->getParameter("reverb_damp") );
	reverb.setDry( _preset->getParameter("reverb_dry") );
	reverb.setMode( _preset->getParameter("reverb_mode") );
	reverb.setRoomSize( _preset->getParameter("reverb_roomsize") );
	reverb.setWet( _preset->getParameter("reverb_wet") );
	reverb.setWidth( _preset->getParameter("reverb_width") );
	
	// now we can initialise the voices
	for( int i=0; i<128; i++ ){
		_voices[i]->setPreset( *_preset );
		_voices[i]->setPitchWheel( pw_val );
		_voices[i]->setFrequency( (440.0/32.0) * pow(2,((i-9.0)/12.0)) );
		_voices[i]->init();
	}
};

void
VoiceAllocationUnit::pwChange( float value )
{
	pw_val.setValue( pow(2,value) );
}

void
VoiceAllocationUnit::sustainOff()
{
	sustain = 0;
	for(int i=0; i<128; i++)
		if( _voices[i] && (!keyPressed[i]) ) 
			_voices[i]->triggerOff();
}

void
VoiceAllocationUnit::noteOn(int note, float velocity)
{
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> noteOn(note:" << note 
	<< " vel:" << velocity << ")" << endl;
#endif
  
	keyPressed[note] = 1;
	
	if (!connected[note])
		if( !max_voices || config->active_voices < max_voices ) {
		_voices[note]->reset();
		mixer.addInput(*_voices[note]);
		config->active_voices++;
		connected[note] = 1;
		}

	_voices[note]->setVelocity(velocity);
	_voices[note]->triggerOn();
  
	purgeVoices();
}

void 
VoiceAllocationUnit::noteOff(int note)
{
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> noteOff(note:" << note << ")" << endl;
#endif
	keyPressed[note] = 0;
	if (!sustain){
		if (_voices[note]) 
			_voices[note]->triggerOff();
		purgeVoices();
	}
}

void 
VoiceAllocationUnit::purgeVoices()
{
#ifdef _DEBUG
	int purged = 0;
	int active = 0;
#endif
  
	for (int note = 0; note < 128; note++) {
		if (connected[note]) {
			if ( _voices[note]->getState()==0 ) {
#ifdef _DEBUG
				purged++;
#endif
				mixer.removeInput(*_voices[note]);
				config->active_voices--;
				connected[note] = 0;
			}
#ifdef _DEBUG
			else active++;
#endif
		}
	}
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> removed " << purged << " voice(s) from mixer, "
	<< active << " voice(s) still connected" << endl;
#endif
}

void
VoiceAllocationUnit::killAllVoices()
{
	int i;
	for( i=0; i<128; i++ ){
		if( connected[i] ){
			mixer.removeInput( *_voices[i] );
			connected[i] = 0;
			keyPressed[i] = 0;
		}
	}
	reverb.mute();
	config->active_voices = 0;
}