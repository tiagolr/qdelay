#include "Crusher.h"
#include "../PluginProcessor.h"

Crusher::Crusher(QDelayAudioProcessor& p) : audioProcessor(p) {}
Crusher::~Crusher() {}

static float linearInterpolation(float y1, float y2, float mu)
{
	return y1 * (1.0f - mu) + y2 * mu;
};

void Crusher::prepare(float _srate)
{
	srate = _srate;
	dc_r = std::exp(-MathConstants<float>::twoPi * 1.f / srate);
	clear();
}

void Crusher::clear()
{
	dnFilterL.clear();
	dnFilterR.clear();
	upFilterL.clear();
	upFilterR.clear();
	dc_inL = dc_outL = 0.f;
	dc_inR = dc_outR = 0.f;
	counterDS = 0;
	counterUS = 0;
	stateDS_L = stateUS_L = 0.f;
	stateDS_R = stateUS_R = 0.f;
	lastStateDS_L = lastStateUS_L = 0.f;
	lastStateDS_R = lastStateUS_R = 0.f;
}

void Crusher::onSlider()
{
	upsample = (bool)audioProcessor.params.getRawParameterValue("crush_upsample")->load();

	float dnsample = 1.f - audioProcessor.params.getRawParameterValue("crush_srate")->load();
	int newratio = MIN_CRATIO + (int)std::round(dnsample * (MAX_CRATIO - MIN_CRATIO));
	if (ratio != newratio)
	{
		counterUS = 0;
		counterDS = 0;
		stateDS_L = stateUS_L = 0.f;
		stateDS_R = stateUS_R = 0.f;
		lastStateDS_L = lastStateUS_L = 0.f;
		lastStateDS_R = lastStateUS_R = 0.f;
		ratio = newratio;
	}

	float crusherBits = 1.f - audioProcessor.params.getRawParameterValue("crush_bits")->load();
	bits = MAX_CBITS - (crusherBits * (MAX_CBITS - MIN_CBITS));
	bitsLevel = 1.f / std::pow(2.f, bits);

	float outSR = srate / ratio;
	dnFilterL.LP(outSR * 0.5f, 4, srate);
	dnFilterR.LP(outSR * 0.5f, 4, srate);
	upFilterL.LP(outSR * 0.5f, 4, srate);
	upFilterR.LP(outSR * 0.5f, 4, srate);
}

void Crusher::downSample (float& left, float& right)
{
	counterDS += 1;

    if ((counterDS > ratio) || (counterDS == 1))
    {
      	lastStateDS_L = stateDS_L;
      	stateDS_L = left;

	  	lastStateDS_R = stateDS_R;
      	stateDS_R = right;

      	counterDS = 1;
	}
	else
	{
      	if (dsmode == Repeat)
      	{
      	  	left = stateDS_L;
			right = stateDS_R;
	  	}
      	else if (dsmode == Zero)
      	{
			left = 0.f;
			right = 0.f;
		}
		else if (dsmode == Interpolate)
		{
			float iratio = counterDS / (float)ratio;
			left = linearInterpolation(lastStateDS_L, stateDS_L, iratio);
			right = linearInterpolation(lastStateDS_R, stateDS_R, iratio);
		}
	}
}

void Crusher::upSample(float& left, float& right)
{
    counterUS += 1;

    if ((counterUS > ratio) || (counterUS == 1))
    {
      	lastStateUS_L = stateUS_L;
	  	lastStateUS_R = stateUS_R;

      	stateUS_L = left;
		stateUS_R = right;

      	counterUS = 1;
	}
	else
    {
		float iratio = counterUS / (float)ratio;
      	left = linearInterpolation(lastStateUS_L, stateUS_L, iratio);
      	right = linearInterpolation(lastStateUS_R, stateUS_R, iratio);
	}
}

void Crusher::dcBlock(float& left, float& right)
{
	dc_outL *= dc_r;
	dc_outR *= dc_r;
	dc_outL += left - dc_inL;
	dc_outR += right - dc_inR;
	dc_inL = left;
	dc_inR = right;
	left = dc_outL;
	right = dc_outR;
}

float Crusher::bitReduce(float x) const
{
	return std::fabs(x) < bitsLevel ? 0.f : x;
}

void Crusher::processBlock(float* left, float* right, int nsamps)
{
	if (ratio <= 1.f && bits == MAX_CBITS)
		return;

	for (int i = 0; i < nsamps; ++i)
	{
		process(left[i], right[i]);
	}
}

void Crusher::process(float& left, float& right)
{
	if (ratio > 1.f)
	{
		left = dnFilterL.process(left);
		right = dnFilterR.process(right);
		downSample(left, right);

		if (upsample)
		{
			upSample(left, right);
			left = upFilterL.process(left);
			right = upFilterR.process(right);
		}
	}

	if (bits < MAX_CBITS)
	{
		left = bitReduce(left);
		right = bitReduce(right);
	}

	dcBlock(left, right);
}