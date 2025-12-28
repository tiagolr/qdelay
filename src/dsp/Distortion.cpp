#include "Distortion.h"

Distortion::Distortion(QDelayAudioProcessor& p) : audioProcessor(p) {}
Distortion::~Distortion() {}

void Distortion::prepare(float _srate)
{
	srate = _srate;
}

void Distortion::clear()
{
}

void Distortion::processBlock(float* left, float* right, int nsamps)
{
	(void)left;
	(void)right;
	(void)nsamps;
}
