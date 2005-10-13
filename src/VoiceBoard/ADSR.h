/* amSynth
 * (c) 2001-2004 Nick Dowell
 **/
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
	
	float * const getNFData (int nFrames);
	
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
