/*
 *  ADSR.cpp
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

#include "ADSR.h"

#include <algorithm>

static const float kMinimumTime = 0.0005f;

void
ADSR::triggerOn()
{
	m_state = State::kAttack;
	m_frames_left_in_state = (int) (m_attack * m_sample_rate);
	const float target = m_decay <= kMinimumTime ? m_sustain : 1.0;
	m_inc = (target - m_value) / (float) m_frames_left_in_state;
}

void 
ADSR::triggerOff()
{
	m_state = State::kRelease;
	m_frames_left_in_state = (int) (m_release * m_sample_rate);
	m_inc = (0.f - m_value) / (float) m_frames_left_in_state;
}

void
ADSR::reset()
{
	m_state = State::kOff;
	m_value = 0;
	m_inc = 0;
	m_frames_left_in_state = UINT_MAX;
}

void
ADSR::process(float *buffer, unsigned frames)
{
	while (frames) {

		const unsigned int count = std::min(frames, m_frames_left_in_state);

		if (m_state == State::kSustain) {
			for (unsigned i = 0; i < count; i++) {
				*buffer = m_value;
				m_value = m_sustain_smoother.processSample(m_sustain);
				buffer++;
			}
		} else {
			for (unsigned i = 0; i < count; i++) {
				*buffer = m_value;
				m_value += m_inc;
				buffer++;
			}
		}

		m_frames_left_in_state -= count;

		if (m_frames_left_in_state == 0) {
			switch (m_state) {
				case State::kAttack:
					m_state = State::kDecay;
					m_frames_left_in_state = (int) (m_decay * m_sample_rate);
					m_inc = (m_sustain - m_value) / (float) m_frames_left_in_state;
					break;
				case State::kDecay:
					m_sustain_smoother.set(m_value);
					m_state = State::kSustain;
					m_frames_left_in_state = UINT_MAX;
					m_inc = 0;
					break;
				case State::kSustain:
					m_frames_left_in_state = UINT_MAX;
					break;
				default:
					m_state = State::kOff;
					m_value = 0;
					m_frames_left_in_state = UINT_MAX;
					m_inc = 0;
					break;
			}
		}

		frames -= count;
	}
}
