#pragma once
#include <JuceHeader.h>
#include "PhaserFilter.h"
#include "Utils.h"
#include "../Globals.h"

using namespace globals;
class QDelayAudioProcessor;

// Simple triangle LFO
class LFO
{
public:
    float phase = 0.0f;
    float rate = 0.5f;
    float srate = 44100.0f;
    int direction = 1;
	float offset = 0.f;

	void prepare(float _srate)
	{
		srate = _srate;
	}
	void setRate(float r)
	{
		rate = r;
	}
	void clear()
	{
		phase = 0.f;
	}

	float tick(float _offset = 0.f)
	{
		offset = _offset;
		phase += rate / srate;
		if (phase >= 1.f)
			phase -= 1.f;

		float p = phase + offset;
		p -= std::floor(p);

		float tri = 4.f * fabsf(p - 0.5f) - 1.f;
		return tri;
	}

	void syncToSongTime(double seconds)
	{
		double cycles = seconds * rate;
		phase = (float)(cycles - std::floor(cycles));
		phase += offset;
		phase -= std::floor(phase);
	}
};

class Phaser
{
public:
	bool isOn = false;

	Phaser(QDelayAudioProcessor& p);
	~Phaser() {}

	void prepare(float srate);
	void onSlider();
	void clear();
	void syncToSongTime(float elapsed);
	void process(float& left, float& right);

private:
	float srate = 44100.f;
	QDelayAudioProcessor& audioProcessor;
	PhaserFilter lphaser{};
	PhaserFilter rphaser{};
	Lerp lfo_center = 1000.f;
	LFO lfoL;
	LFO lfoR;
	float lfo = 0.f; // norm -1..1 lfo
	float lfo_rate = 1.f;
	float lfo_depth = 200.f;
	float lfo_stereo = 0.f; // 0..1 normal stereo separation offset
	float res = 0.f;
	int lfo_sync = 0;
	Lerp mix = 0.f;

	float getLfoRate(int sync);
};