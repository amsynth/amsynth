/*
 *  ADSR.h
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#ifndef _ADSR_H
#define _ADSR_H

#include "Synth--.h"

#include <climits>

class ADSR
{
public:
	enum class State {
		kAttack,
		kDecay,
		kSustain,
		kRelease,
		kOff
	};

	void	SetSampleRate	(int value) { m_sample_rate = value; }

	void	SetAttack	(float value) { m_attack = value; }
	void	SetDecay	(float value) { m_decay = value; }
	void	SetSustain	(float value) { m_sustain = value; if (m_state == State::kSustain) m_value = value; }
	void	SetRelease	(float value) { m_release = value; }
	
	void	process		(float *buffer, unsigned frames);
	
	void	triggerOn	();
	void	triggerOff	();

	int		getState	() { return (m_state == State::kOff) ? 0 : 1; };

	/**
	 * puts the envelope directly into the off (ADSR_OFF) state, without
	 * going through the usual stages (ADSR_R).
	 */
	void reset();

private:
	float			m_attack = 0;
	float			m_decay = 0;
	float			m_sustain = 1;
	ParamSmoother	m_sustain_smoother{1.f};
	float			m_release = 0;

	float			m_sample_rate = 44100;
	State			m_state = State::kOff;

	float			m_value = 0.0F;
	float			m_inc = 0.0F;
	unsigned		m_frames_left_in_state = UINT_MAX;
};

#endif				//_ADSR_H
