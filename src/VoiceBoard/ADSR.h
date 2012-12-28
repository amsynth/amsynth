/*
 *  ADSR.h
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

#ifndef _ADSR_H
#define _ADSR_H

class ADSR
{
public:
	enum ADSRState { attack, decay, sustain, release, off };

	ADSR	(float *buffer);
	
	void	SetSampleRate	(int value) { m_sample_rate = value; }

	void	SetAttack	(float value) { m_attack = value; }
	void	SetDecay	(float value) { m_decay = value; }
	void	SetSustain	(float value) { m_sustain = value; if (m_state == sustain) m_value = value; }
	void	SetRelease	(float value) { m_release = value; }
	
	float * getNFData	(unsigned int frames);
	
	void	triggerOn	();
	void	triggerOff	();

	int		getState	() { return (m_state == off) ? 0 : 1; };

	/**
	 * puts the envelope directly into the off (ADSR_OFF) state, without
	 * going through the usual stages (ADSR_R).
	 */
	void reset();

private:
	float       m_attack;
	float       m_decay;
	float       m_sustain;
	float       m_release;

	float *     m_buffer;
	float       m_sample_rate;
	ADSRState   m_state;

	float       m_value;
	float       m_inc;
	unsigned	m_frames_left_in_state;
};

#endif				//_ADSR_H
