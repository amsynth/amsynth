/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PRESET_H
#define _PRESET_H

#include <string>
#include "base.h"
#include "Parameter.h"

/**
 * @class Preset
 * the Preset holds all the parameters for a paticular timbre (preset).
 */

class Preset {

  public:
    Preset();
    ~Preset();
    string getName();
    void setName(string name);
    Parameter & getParameter(string name);
    Parameter & getParameter(int no) {
		return parameters[no];
    };
    void clone(Preset & preset);
    void randomise();
	void	AddListenerToAll	(UpdateListener*);
  private:
    string _name;
    Parameter parameters[128];
    Parameter nullparam;
    int no_p;
};

#endif
