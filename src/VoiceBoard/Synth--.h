/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#ifndef _SYNTH_H
#define _SYNTH_H

// 128 frames @ 44100 = 2.9ms,  @ 48000 = 2.67ms
// 256 frames @ 44100 = 5.8ms,  @ 48000 = 5.33ms
// 512 frames @ 44100 = 11.6ms, @ 48000 = 10.6ms
#define BUF_SIZE    256

#define TWO_PI 6.28318530717958647692
#define PI     3.14159265358979323846


//  Abstract Classes

/** \class FSource
 *
 * this class captures objects which have one (or more) outputs, in floating
 * point format, but aren't normalised.
 **/
class FSource {
public:
  virtual float *getFData()
    = 0;
private:
};


/** \class NFSource
 *  
 * these objects are all those which produce one (or more) outputs, and those
 * outputs are in normalised (between -1 and 1) floating point format.
 **/
class NFSource : public FSource {
public:
  inline float *getFData(){ return getNFData(); }
  virtual float *getNFData()
    = 0;
};


/** \class FInput
 *
 * an FInput object is one which takes input data in F form, neccesarily from
 * an FSource object.
 **/
class FInput {
public:
  virtual void setInput(FSource & source)
    = 0;
};


/** \class NFInput
 *
 * an NFInput object is one which takes input data in NF form, neccesarily from
 * an NFSource object.
 **/
class NFInput {
public:
    virtual void setInput(NFSource & source)
    = 0;
};


#endif // _SYNTH_H
