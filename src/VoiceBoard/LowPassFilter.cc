/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "LowPassFilter.h"
#include <math.h>

LowPassFilter::LowPassFilter(int rate)
{
	f = k = p = r = d1 = d2 = d3 = d4 = 0.0;
	a0 = a1 = a2 = b1 = b2 = 0.0;
		
	cutoff = 0;
	cutoff_param = 0;
	res_param = 0;
	this->rate = rate;
	nyquist = rate/(float)2;
	max = 1;
}

void
LowPassFilter::reset()
{
	d1 = d2 = d3 = d4 = 0;
}
	

void
LowPassFilter::setInput( NFSource & source )
{
	this->source = &source;
}

void
LowPassFilter::setCFreq( FSource & fval )
{
	cutoff = &fval;
}

void
LowPassFilter::setCutoff( Parameter & param )
{
	cutoff_param = &param;
}

void
LowPassFilter::setResonance( Parameter & param )
{
	res_param = &param;
	param.addUpdateListener( *this );
	update();
}

void
LowPassFilter::update()
{
	res = res_param->getControlValue();
}

float *
LowPassFilter::getNFData()
{
	float *buffer = source->getNFData();
	float fc = cutoff->getFData()[(int)(BUF_SIZE/2)];
	
	// constrain cutoff
#define SAFE 0.99 // filter is unstable _AT_ PI
	if (fc>(nyquist*SAFE))
		fc=nyquist*SAFE;
	if (fc<10)
		{fc = 10;/*d1=d2=d3=d4=0;*/}
	float w = (fc/(float)rate); // cutoff freq [ 0 <= w <= 0.5 ]
	
//	w *= 2;
	
	// find final coeff values for end of this buffer
	register double k, k2, bh, a0, a1, a2, b1, b2;
	double r = 2*(1-res);
	if(r==0.0) r = 0.001;
	k=tan(w*PI);
	k2 = k*k;
	bh = 1 + (r*k) + k2;
	a0 = a2 = double(k2/bh);
	a1 = a0 * 2;
	b1 = double(2*(k2-1)/-bh);
	b2 = double((1-(r*k)+k2)/-bh);
	
	// renormalise the gain *at* the cutoff frequency. (think high resonance)
	/*
	w *= PI;
	register float w2 = w*2;
	float _w = w;
	register float g, gd, tmp1, tmp2, cf;
	*/
	/*
	tmp1 = (a1*cos(w) + a2*cos(w2));
	tmp2 = (a1*sin(w) + a2*sin(w2));
	g = ( a0 + tmp1*tmp1 - tmp2*tmp2 );
	tmp1 = -(-1 + b1*cos(w) + b2*cos(w2));
	tmp2 = -(b1*sin(w) + b2*sin(w2));
	g /= ( 1 -(tmp1*tmp1) -(tmp2*tmp2) );
	g = sqrt(g);
	*/
	/*
	g = 0.0;
	for(w=0.0; w<PI; w+=0.01){
		w2 = 2*w;
		tmp1 = (a0 + a1*cos(-w) + a2*cos(-w2));
		tmp2 = (a1*sin(-w) + a2*sin(-w2));
		gd = ((tmp1*tmp1) + (tmp2*tmp2));
		tmp1 = (1+b1*-cos(-w) + b2*-cos(-w2));
		tmp2 = (b1*-sin(-w)+b2*-sin(-w2));
		tmp1 = (tmp1*tmp1) + (tmp2*tmp2);
		gd/=tmp1;
		gd=sqrt(gd);
		if(gd>g){
			g=gd;
			cf = w;
		}
	}
	
	if(g<1)
		g=1;
	*/
//	a0 /= max;
//	a1 /= max;
//	a2 /= max;
	
	/*
	g = sqrt( 
		( a0 + pow((a1*cos(w) + a2*cos(2*w)),2) - pow((a1*sin(w) - a2*sin(2*w)),2) ) /
		( 1  + pow((-b1*cos(w) - -b2*cos(2*w)),2) - pow((-b1*sin(w) - -b2*sin(2*w)),2) ) );
	*/
//	g = ( ( a0 + a1 + a2 ) / ( 1 -b1 - b2 ) );
	//cout << "w= " << _w << "\tg = " << g << "\tcf=" << cf;
	
	double x,y;
//	max = 0.0;
//	max -= 0.01;
//	register float foo;
	// filter (2 cascaded second order filters)
	// this is the easy bit :)
	for (int i=0; i<BUF_SIZE; i++) {
		x = buffer[i];
		
		// first 2nd-order unit
		y = ( a0*x ) + d1;
		d1 = d2 + ( (a1)*x ) + ( (b1)*y );
		d2 = ( (a2)*x ) + ( (b2)*y );
		x=y;
		// and the second
		
		y = ( a0*x ) + d3;
		d3 = d4 + ( a1*x ) + ( b1*y );
		d4 = ( a2*x ) + ( b2*y );
		
		buffer[i] = y;
		/*
		foo = fabs(y);
		if(foo>max)
			max -= (foo-max)/10;
		*/
	}
//	cout << " max output=" << max << endl;
	/*
	if(max<1)
		max = 1;
	*/
	return buffer;
}