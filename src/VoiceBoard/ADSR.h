/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/
/**
 * @file   ADSR.h
 * @author nixx
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
 * contour generators found in nearly all synths..
 */
class ADSR : public EnvelopeGenerator, public UpdateListener {
  public:
    ADSR(int rate);
    ~ADSR();
    inline float *getNFData();
    void triggerOn();
    void triggerOff();
    void setAttack( Parameter & param );
    void setDecay( Parameter & param );
    void setSustain( Parameter & param );
    void setRelease( Parameter & param );
    int getState();
	void reset();
	void update();
  private:
    Parameter *attackParam, *decayParam, *sustainParam, *releaseParam; 
    float *buffer;
    int state, rate;
    float a_delta, d_delta, r_time, r_delta, s_val, c_val;
};

#endif				//_ADSR_H
