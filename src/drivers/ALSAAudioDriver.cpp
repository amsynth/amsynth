/*
 *  ALSAAudioDriver.cpp
 *
 *  Copyright (c) 2001-2019 Nick Dowell
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstddef>

#include "ALSAAudioDriver.h"

#ifdef WITH_ALSA

#include "../Configuration.h"
#include "AudioDriver.h"

#include <iostream>
#include <alsa/asoundlib.h>


class ALSAAudioDriver : public AudioDriver
{
public:

    int open() override;
    void close() override;
    int write(float *buffer, int nsamples) override;

private:

    snd_pcm_t *_handle = nullptr;
    short *_buffer = nullptr;
    unsigned _channels = 0;
};


int
ALSAAudioDriver::write(float *buffer, int nsamples)
{
	if (!_handle) {
		return -1;
	}

	assert(nsamples <= kMaxWriteFrames);
	for (int i = 0; i < nsamples; i++) {
		short s16 = buffer[i] * 32767;
		((unsigned char *)_buffer)[i * 2 + 0] = ( s16       & 0xff);
		((unsigned char *)_buffer)[i * 2 + 1] = ((s16 >> 8) & 0xff);
	}

	snd_pcm_sframes_t err = snd_pcm_writei(_handle, _buffer, nsamples / _channels);
	if (err < 0) {
		err = snd_pcm_recover(_handle, err, 1);
	}
	if (err < 0) {
		return -1;
	}
	return 0;
}

int 
ALSAAudioDriver::open()
{
	Configuration & config = Configuration::get();
	
	if (_handle != nullptr) {
		return 0;
	}

#define ALSA_CALL(expr) \
	if ((err = (expr)) < 0) { \
		std::cerr << #expr << " failed with error: " << snd_strerror(err) << std::endl; \
		if (pcm) { snd_pcm_close(pcm); } \
		return -1; \
	}

	int err = 0;

	snd_pcm_t *pcm = nullptr;
	ALSA_CALL(snd_pcm_open(&pcm, config.alsa_audio_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0));

	unsigned int latency = 10 * 1000;
	ALSA_CALL(snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, config.channels, config.sample_rate, 0, latency));

#if DEBUG
	snd_pcm_uframes_t period_size = 0;
	snd_pcm_uframes_t buffer_size = 0;
	ALSA_CALL(snd_pcm_get_params(pcm, &buffer_size, &period_size));
	std::cout << "Opened ALSA device \"" << config.alsa_audio_device<< "\" @ " << config.sample_rate << "Hz, period_size = " << period_size << " buffer_size = " << buffer_size << std::endl;
#endif

	_handle = pcm;
	_channels = config.channels;
	_buffer = (short *)malloc(kMaxWriteFrames * _channels * sizeof(short));

	config.current_audio_driver = "ALSA";
#ifdef ENABLE_REALTIME
	config.current_audio_driver_wants_realtime = 1;
#endif

	return 0;
}

void ALSAAudioDriver::close()
{
	if (_handle != nullptr) {
		snd_pcm_close((snd_pcm_t *)_handle);
		_handle = nullptr;
	}

	free(_buffer);
	_buffer = nullptr;
}

#endif


AudioDriver * CreateALSAAudioDriver()
{
#ifdef WITH_ALSA
    return new ALSAAudioDriver();
#else
    return NULL;
#endif
}
