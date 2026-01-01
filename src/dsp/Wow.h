/**
 * Wow and Flutter taken from ChowDSP Analog Tape Model
 * https://github.com/jatinchowdhury18/AnalogTapeModel/
 */

#pragma once
#include <JuceHeader.h>
#include <utility>
#include "Utils.h"
#include "SVF.h"

/**
 * Class to simulate the Ornstein-Uhlenbeck process.
 * Mostly lifted from https://github.com/mhampton/ZetaCarinaeModules
 * under the GPLv3 license.
 */
class OHProcess
{
public:
    OHProcess() = default;

    void prepare(float sampleRate)
    {
        lpf.lp(sampleRate, 10.0f, 0.707f);

        sqrtdelta = 1.0f / std::sqrt(sampleRate);
        T = 1.0f / sampleRate;
    }

    void prepareBlock(float amount)
    {
        amount = std::pow(amount, 1.25f);
        amt = amount;
        damping = amt * 20.0f + 1.0f;
        mean = amt;
    }

    inline float process() noexcept
    {
        y += sqrtdelta * noiseGen.gaussian() * noiseGain * amt;
        y += damping * (mean - y) * T;
        return lpf.process(y);
    }

private:
    float y = 0.f;
    float sqrtdelta = 1.0f / std::sqrt(44100.0f);
    float T = 1.0f / 44100.0f;

    float amt = 0.0f;
    float mean = 0.0f;
    float damping = 0.0f;
    const float noiseGain = 1.0f / 2.33f;

    NoiseGen noiseGen{12345};

    SVF lpf;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OHProcess)
};


class QDelayAudioProcessor;

class Wow
{
public:
    float curDepth = 0.f;

	Wow(QDelayAudioProcessor& proc);

	void prepare (float sampleRate);
	void prepareBlock ();

	inline float getLFO () noexcept
    {
        phase += angleDelta;
        curDepth = depthSmooth.process(depth) * amp;
        return curDepth * (std::cos(phase) + ohProc.process());
    }

	inline void boundPhase () noexcept
    {
        while (phase >= MathConstants<float>::twoPi)
            phase -= MathConstants<float>::twoPi;
    }

private:
	QDelayAudioProcessor& audioProcessor;
    float depth = 0.f;
    float rate = 0.f;
	float srate = 44100.0f;
	RCFilter depthSmooth;
    OHProcess ohProc;
	float phase = 0.f;
	float amp = 0.0f;
    float angleDelta = 0.f;
    float drift = 0.f;
    float var = 0.f;
    Random driftRand;
};