/*
 *  Thread.h
 *
 *  Copyright (c) 2001-2015 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef _THREAD_H
#define _THREAD_H

#ifdef _MSC_VER

class Thread {
};

#else

#include <pthread.h>
#include <signal.h>

class Thread
{
public:
	Thread() : mThread(0), mShouldStop(false) {}
	virtual ~Thread () {}
	
	int		Run		() { return pthread_create (&mThread, NULL, Thread::start_routine, this); }
	void	Stop	() { mShouldStop = true; }
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

#endif /* _MSC_VER */

#endif /* _THREAD_H */
