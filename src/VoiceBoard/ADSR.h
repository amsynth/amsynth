/* amSynth
 * (c) 2001,2002 Nick Dowell
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

#include "Synth--.h"
#include "FValue.h"
#include "NFValue.h"
#include "EnvelopeGenerator.h"

/**
 * @class ADSR
 * @brief An implementation of an ADSR contour generator.
 *
 * ADSR is an implementation of the class Attack-Decay-Sustain-Release
 * contour generators found in nearly all analogue synths..
 */
class ADSR : public EnvelopeGenerator, public UpdateListener {
  public:
    ADSR(int rate, float *buf);
    ~ADSR();
    inline float *getNFData();
	/**
	 * Send a trigger on message to the envelope.
	 * This will cause the envelope to enter its attack stage.
	 */
    void triggerOn();
	/**
	 * Send a trigger off message to the envelope.
	 * This will cause the envelope to enter its release stage.
	 */
    void triggerOff();
	/**
	 * @param param the Parameter to use for the attack time (in secconds)
	 */
    void setAttack( Parameter & param );
	/**
	 * @param param the Parameter to use for the decay time (in secconds)
	 */
    void setDecay( Parameter & param );
	/**
	 * @param param the Parameter to use for the sustain level (0-1)
	 */
    void setSustain( Parameter & param );
	/**
	 * @param param the Parameter to use for the release time (in secconds)
	 */
    void setRelease( Parameter & param );
	/**
	 * reports which state the envelope generator is currently in.
	 * @returns ADSR_OFF if the envelope is currently off,
	 * 			ADSR_A if in the attack stage,
	 * 			ADSR_D if in the decay stage,
	 *			ADSR_S if in the sustain stage,
	 *			ADSR_R if in the release stage.
	 */
    int getState();
	/**
	 * puts the envelope directly into the off (ADSR_OFF) state, without
	 * going through the usual stages (ADSR_R).
	 */
	void reset();
	/**
	 * causes Parameters to be re-read and internal values set appropriately.
	 * this is called from the associated Parameters.
	 */
	void update();
  private:
    Parameter *attackParam, *decayParam, *sustainParam, *releaseParam; 
    float *buffer;
    int state, rate;
    float a_delta, d_delta, r_time, r_delta, s_val, c_val;
};

#endif				//_ADSR_H
