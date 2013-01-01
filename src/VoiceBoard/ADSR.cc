/*
 *  ADSR.cc
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

#include "ADSR.h"

#include "Synth--.h"

#include <climits>

static const float kMinimumTime = 0.0005;

ADSR::ADSR(float * buffer)
:	m_attack(0)
,	m_decay(0)
,	m_sustain(1)
,	m_release(0)
,	m_buffer(buffer)
,	m_sample_rate(44100)
,	m_state(off)
,	m_value(0)
,	m_inc(0)
,	m_frames_left_in_state(UINT_MAX)
{
}

void
ADSR::triggerOn()
{
	m_state = attack;
	m_frames_left_in_state = (m_attack * m_sample_rate);
	const float target = m_decay <= kMinimumTime ? m_sustain : 1.0;
	m_inc = (target - m_value) / (double)m_frames_left_in_state;
}

void 
ADSR::triggerOff()
{
	m_state = release;
	m_frames_left_in_state = (m_release * m_sample_rate);
	m_inc = (0.0 - m_value) / (double)m_frames_left_in_state;
}

void
ADSR::reset()
{
	m_state = off;
	m_value = 0;
	m_inc = 0;
	m_frames_left_in_state = UINT_MAX;
}

float *
ADSR::getNFData(unsigned int frames)
{
	float *buffer = m_buffer;

	while (frames) {

		const unsigned int count = MIN(frames, m_frames_left_in_state);

		for (int i=0; i<count; i++) {
			*buffer = m_value;
			m_value += m_inc;
			buffer++;
		}

		m_frames_left_in_state -= count;

		if (m_frames_left_in_state == 0) {
			switch (m_state) {
				case attack:
					m_state = decay;
					m_frames_left_in_state = (m_decay * m_sample_rate);
					m_inc = (m_sustain - m_value) / (double)m_frames_left_in_state;
					break;
				case decay:
					m_state = sustain;
					m_value = m_sustain;
					m_frames_left_in_state = UINT_MAX;
					m_inc = 0;
					break;
				case sustain:
					m_frames_left_in_state = UINT_MAX;
					break;
				default:
					m_state = off;
					m_value = 0;
					m_frames_left_in_state = UINT_MAX;
					m_inc = 0;
					break;
			}
		}

		frames -= count;
	}

	return m_buffer;
}
