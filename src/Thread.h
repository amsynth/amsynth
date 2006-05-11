/***************************************************************************
 *            amSynth - Thread.h
 *
 *  Wed Jun  1 23:36:17 2005
 *  Copyright  2005  Nick Dowell
 *  nixx2097@users.sourceforge.net
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>
#include <signal.h>

class Thread
{
public:
	virtual ~Thread () {}
	
	int		Run		() { return pthread_create (&mThread, NULL, Thread::start_routine, this); }
	void	Stop	() { mShouldStop = true; }
	int		Kill	(int sig = 2) { return mThread ? pthread_kill(mThread, sig) : 0; }
	int		Join	() { return mThread ? pthread_join(mThread, NULL) : 0; }

protected:
	// override me!
	// and make sure to call ShouldStop() periodically and return if so.
	virtual void 	ThreadAction () = 0;
	bool			ShouldStop () { return mShouldStop; }

private:
	static void* start_routine (void *arg)
	{
		Thread *self = (Thread *) arg;
		self->mShouldStop = false;
		self->ThreadAction ();
		pthread_exit (0);
	}
	pthread_t	mThread;
	bool		mShouldStop;
};

#endif /* _THREAD_H */
