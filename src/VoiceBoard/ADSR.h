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

/**
 * @file   ADSR.h
 * @author Nick Dowell
 * @date   Sun Sep 16 17:13:57 2001
 * 
 * @brief  implementation of an ADSR contour generator
 */
#ifndef _ADSR_H
#define _ADSR_H

/**
 * @class ADSR
 * @brief An implementation of an ADSR contour generator.
 *
 * ADSR is an implementation of the class Attack-Decay-Sustain-Release
 * contour generators found in nearly all analogue synths..
 */
class ADSR
{
public:
	enum ADSRState { attack, decay, sustain, release, off };

	ADSR	(float * const buf);
	
	void	SetSampleRate	(int);

	void	SetAttack	(float);
	void	SetDecay	(float);
	void	SetSustain	(float);
	void	SetRelease	(float);
	
	float * getNFData (int nFrames);
	
	void	triggerOn	();
	void	triggerOff	();

	// returns 1 if envelope is still alive.
	int	getState	();

	/**
	 * puts the envelope directly into the off (ADSR_OFF) state, without
	 * going through the usual stages (ADSR_R).
	 */
	void reset();
private:
	float * const buffer;
	ADSRState	state;
	int		rate;
	float a_time, a_delta, d_time, d_delta, d_frames, r_time, r_delta, s_val, c_val;
  	float m_attack_frames, m_release_frames, m_decay_frames;
};

#endif				//_ADSR_H
