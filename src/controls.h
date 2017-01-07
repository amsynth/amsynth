/*
 *  controls.h
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

#ifndef _controls_h
#define _controls_h

#include <stdlib.h>

typedef enum {
	kAmsynthParameter_AmpEnvAttack             = 0,
	kAmsynthParameter_AmpEnvDecay              = 1,
	kAmsynthParameter_AmpEnvSustain            = 2,
	kAmsynthParameter_AmpEnvRelease            = 3,
	
	kAmsynthParameter_Oscillator1Waveform      = 4,
	
	kAmsynthParameter_FilterEnvAttack          = 5,
	kAmsynthParameter_FilterEnvDecay           = 6,
	kAmsynthParameter_FilterEnvSustain         = 7,
	kAmsynthParameter_FilterEnvRelease         = 8,
	kAmsynthParameter_FilterResonance          = 9,
	kAmsynthParameter_FilterEnvAmount          = 10,
	kAmsynthParameter_FilterCutoff             = 11,
	
	kAmsynthParameter_Oscillator2Detune        = 12,
	kAmsynthParameter_Oscillator2Waveform      = 13,
	
	kAmsynthParameter_MasterVolume             = 14,
	
	kAmsynthParameter_LFOFreq                  = 15,
	kAmsynthParameter_LFOWaveform              = 16,
	
	kAmsynthParameter_Oscillator2Octave        = 17,
	kAmsynthParameter_OscillatorMix            = 18,
	
	kAmsynthParameter_LFOToOscillators         = 19,
	kAmsynthParameter_LFOToFilterCutoff        = 20,
	kAmsynthParameter_LFOToAmp                 = 21,
	
	kAmsynthParameter_OscillatorMixRingMod     = 22,
	
	kAmsynthParameter_Oscillator1Pulsewidth    = 23,
	kAmsynthParameter_Oscillator2Pulsewidth    = 24,
	
	kAmsynthParameter_ReverbRoomsize           = 25,
	kAmsynthParameter_ReverbDamp               = 26,
	kAmsynthParameter_ReverbWet                = 27,
	kAmsynthParameter_ReverbWidth              = 28,
	
	kAmsynthParameter_AmpDistortion            = 29,
	
	kAmsynthParameter_Oscillator2Sync          = 30,

	kAmsynthParameter_PortamentoTime           = 31,
	
	kAmsynthParameter_KeyboardMode             = 32,

	kAmsynthParameter_Oscillator2Pitch         = 33,
	kAmsynthParameter_FilterType               = 34,
	kAmsynthParameter_FilterSlope              = 35,

	kAmsynthParameter_LFOOscillatorSelect      = 36,

	kAmsynthParameter_FilterKeyTrackAmount     = 37,
	kAmsynthParameter_FilterKeyVelocityAmount  = 38,
	
	kAmsynthParameter_AmpVelocityAmount        = 39,
	
	kAmsynthParameter_PortamentoMode           = 40,

	kAmsynthParameterCount
} Param;

typedef enum {
	KeyboardModePoly,
	KeyboardModeMono,
	KeyboardModeLegato,
} KeyboardMode;

typedef enum {
	PortamentoModeAlways,
	PortamentoModeLegato
} PortamentoMode;

#ifdef __cplusplus
extern "C" {
#endif

const char *parameter_name_from_index (int param_index);
int parameter_index_from_name (const char *param_name);

int parameter_get_display (int parameter_index, float parameter_value, char *buffer, size_t maxlen);
const char **parameter_get_value_strings (int parameter_index);

#ifdef __cplusplus
}
#endif

#endif
