#include "Diffusor.h"

void Diffusor::clear()
{
	for (int i = 0; i < NUM_ALLPASS; i++) {
		allpassL[i].clear();
		allpassR[i].clear();
	}
}

void Diffusor::prepare(float _srate)
{
	srate = _srate;
	mps = srate / 343;
	distance = mps * 3.75f;

	std::array<float, NUM_ALLPASS> apLCoeffs = {12.11f, 10.49f, 8.51f, 7.81f, 6.21f, 5.36f, 3.17f, 2.21f};
	std::array<float, NUM_ALLPASS> apRCoeffs = {12.08f, 10.47f, 8.49f, 7.77f, 6.23f, 5.33f, 3.71f, 2.12f};

	for (int i = 0; i < NUM_ALLPASS; i++) 
	{
		allpassL[i].init(srate, apLCoeffs[i], distance);
		allpassR[i].init(srate, apRCoeffs[i], distance);
	}
}

void Diffusor::setSize(float size)
{
	for (int i = 0; i < NUM_ALLPASS; ++i) 
	{
		allpassL[i].setSizeOffsets(size);
		allpassR[i].setSizeOffsets(size);
	}
}

void Diffusor::process(float& left, float& right, float drymix, float wetmix)
{
	float spl0 = left;
	float spl1 = right;

	for (int i = 0; i < NUM_ALLPASS; ++i) {
		spl0 = allpassL[i].allPass(spl0, smear);
		spl1 = allpassR[i].allPass(spl1, smear);
	}

	left = left * drymix + spl0 * wetmix;
	right = right * drymix + spl1 * wetmix;
}