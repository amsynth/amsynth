/* amSynth
 * (c) 2001-2004 Nick Dowell
 */
#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

#include "Synth--.h"

/**
 * @brief An Audio Oscillator unit.
 * 
 * Provides several different output waveforms (sine, saw, square, noise, 
 * random).
 */
class Oscillator
{
public:
	enum Waveform { 
		Waveform_Sine,
		Waveform_Pulse,
		Waveform_Saw,
		Waveform_Noise,
		Waveform_Random
	};

	Oscillator	(float *buf);

	void	SetSampleRate	(int rateIn) { rate = rateIn; twopi_rate = (float) TWO_PI / rate; }
	
	void	ProcessSamples		(float*, int, float freq_hz, float pw);
	void	SetWaveform		(Waveform);

	void reset();
	/*
	 * reset the oscillator, initially at sample indicated by offset, and then 
	 * every period samples. used for oscillator sync. 
	 * NB. period >= delta
	 */
    void reset( int offset, int period );

	void	SetSyncOsc	(Oscillator &);
	void	SetSync		(int);

	void	update		();

  private:
    float *inBuffer, *outBuffer;
    float rads, twopi_rate, random, freq;
	double a0, a1, b1, d; // for the low-pass filter
    int waveform, rate, random_count, period;

	float	mPulseWidth;
	
	// oscillator sync stuff
	int reset_offset, reset_cd, sync_c, sync, sync_offset, sync_period, reset_period;
	Oscillator *syncOsc;
	
    inline void doSine(float*, int nFrames);
    inline void doSquare(float*, int nFrames);
    inline void doSaw(float*, int nFrames);
    inline void doNoise(float*, int nFrames);
	inline void doRandom(float*, int nFrames);
	inline float sqr(float foo);
	inline float saw(float foo);
};


#endif				/// _OSCILLATOR_H
