/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PARAMETERVIEW_H
#define _PARAMETERVIEW_H

#include <gtk--/widget.h>
#include <stdlib.h>
#include "../Parameter.h"
#include "../UpdateListener.h"


class ParameterView : public UpdateListener {
  public:
    virtual void setParameter(Parameter & param) = 0;
    virtual Parameter *getParameter() = 0;
    virtual void setName(string name) = 0;
    virtual void update() = 0;
  private:
    Parameter *parameter;
};


#endif
