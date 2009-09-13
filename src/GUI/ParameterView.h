/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _PARAMETERVIEW_H
#define _PARAMETERVIEW_H

#include <gtkmm.h>
#include <stdlib.h>
#include <unistd.h>
#include "../Parameter.h"
#include "../UpdateListener.h"
#include "Request.h"

class ParameterView : public UpdateListener
{
public:
						ParameterView	(int pipe_d) : parameter (0), piped (pipe_d) { request.slot = sigc::mem_fun (*this, &ParameterView::_update_); }
	virtual				~ParameterView	() { if (parameter) parameter->removeUpdateListener (*this); parameter = 0; }

	virtual void		setParameter	(Parameter& param) { parameter = &param; param.addUpdateListener (*this); update (); }
	virtual Parameter*	getParameter	() { return parameter; }
	virtual void 		setName			(string name) = 0;
	/**
	 * This should only ever called by the GUI thread itself!
	 * non-compliance will break the GUI!
	 **/
	virtual void 		_update_	( ) = 0;
	/**
	 * Update the GUI to reflect changes in associated Parameter. 
	 * This function is safe to be called by any thread.
	 **/
	void 				update		( ) { ssize_t res = write(piped, &request, sizeof(request)); res = 0; }
	
protected:
	Parameter	*parameter;
private:
	Request		request;
	int 		piped; 
};


#endif
