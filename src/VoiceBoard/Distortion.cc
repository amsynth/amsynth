/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "Distortion.h"

Distortion::Distortion()
{
	driveParam = 0;
	crunchParam = 0;
    drive = 1;
    crunch = 1 / 4;
	done = 0;
}

void
Distortion::setInput( FSource & input )
{
    this->input = &input;
}

void 
Distortion::setDrive(Parameter & param)
{
    driveParam = &param;
    driveParam->addUpdateListener(*this);
    update();
}

void 
Distortion::setCrunch(Parameter & param)
{
    crunchParam = &param;
    crunchParam->addUpdateListener(*this);
    update();
}

void 
Distortion::update()
{
    if(driveParam){
		drive = driveParam->getControlValue();
//		cout << "drive: " << drive << endl;
	}
    // crunch values of 16 = nice and clean, 2 = diiirty!
    if(crunchParam)
	{
//		crunch = 1/(2*(crunchParam->getControlValue()+1));
//		crunch*=crunch;
//		crunch*=crunch;
//		crunch*=8;
		/* LIMITDISTORT
		crunch=crunchParam->getControlValue();
		*/
/*		// LOGDISTORT
		crunch=crunchParam->getControlValue()*2;
		crunch*=crunch;crunch*=crunch;*/
		// EXPDISTORT
		crunch=1-crunchParam->getControlValue();
	}
}

float *
Distortion::getNFData()
{
    buffer = input->getFData();
    register float x, y, s;
	if(crunch==0)crunch=0.01;
/*	// LOGDISTORT (3lines)
	register float k;
	if (crunch<0.001) return buffer;
	k=log(crunch+1);*/
	
	for (int i = 0; i < BUF_SIZE; i++) {
//		y = drive * buffer[i];
		/* LIMITDISTORT.
		x = buffer[i]*drive;
		if (x<0) s=-1; else s=1;
		x *= s;
		if (x>1) x=1+((x-1)*crunch);
		buffer[i]=s*x;
		*/
		
/*		// LOGDISTORT (5lines)
		x=buffer[i]*drive;
		if (x<0) s=-1; else s=1;
		x*=s;
		x=log(crunch*x +1)/k;
		buffer[i]=x*s;*/

		// EXPDISTORT (3lines)
		x=buffer[i]*drive;if(x<0) s=-1; else s=1; x*=s;
		x=pow(x,crunch);
		buffer[i]=x*s;
		
//		x=x-(x*x*crunch);
//		y=fabs(x);		
//		buffer[i] = 0;
//		buffer[i] = x - (x*x*x)*crunch;
		/*
		if (y < 0) x = -y;
		else x = y;
			
		x = x - ((x * x)*4);
		
		if (y < 0) y = -x;
		else y = x;
	
		buffer[i] = y;
		*/
//		buffer[i] = x*((y*y)*4);
	}
	return buffer;
}
