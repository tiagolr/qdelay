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

    float tick(float offset = 0.f)
	{
        float increment = rate / srate * 2.0f;
        phase += increment * direction;
        if (phase >= 1.0f) { phase = 1.0f; direction = -1; }
        if (phase <= -1.0f){ phase = -1.0f; direction = 1; }

        // Apply phase offset
        float phasedValue = phase + offset * 2.0f;
        if (phasedValue > 1.0f) phasedValue -= 2.0f;
        if (phasedValue < -1.0f) phasedValue += 2.0f;

        return phasedValue;
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
	void resetPhase(float elapsed);
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

	Lerp mix = 0.f;
};