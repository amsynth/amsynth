/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PARAMETERVIEW_H
#define _PARAMETERVIEW_H

#include <gtk--/widget.h>
#include <stdlib.h>
#include <unistd.h>
#include "../Parameter.h"
#include "../UpdateListener.h"
#include "Request.h"

class ParameterView : public UpdateListener {
  public:
    virtual void setParameter(Parameter & param) = 0;
    virtual Parameter *getParameter() = 0;
    virtual void setName(string name) = 0;
	/**
	 * This should only ever called by the GUI thread itself!
	 * non-compliance will break the GUI!
	 **/
	virtual void _update_() = 0;
	/**
	 * Update the GUI to reflect changes in associated Parameter. 
	 * This function is safe to be called by any thread.
	 **/
    virtual void update() = 0;
	int piped;
	Request request;
  private:
    Parameter *parameter;
};


#endif
