/**
 * Wow and Flutter taken from ChowDSP Analog Tape Model
 * https://github.com/jatinchowdhury18/AnalogTapeModel/
 */

#pragma once
#include <JuceHeader.h>
#include <utility>
#include "Utils.h"

class QDelayAudioProcessor;

class Flutter
{
public:
    float dcOffset = 0.f;

	Flutter(QDelayAudioProcessor& proc);

	void prepare (float sampleRate);
	void prepareBlock ();

	inline void updatePhase () noexcept
    {
        phase1 += angleDelta1;
        phase2 += angleDelta2;
        phase3 += angleDelta3;
    }

	inline float getLFO () noexcept
    {
        updatePhase ();
        return depthSmooth.process(depth) * (amp1 * std::cos (phase1 + phaseOff1)
                                + amp2 * std::cos (phase2 + phaseOff2)
                                + amp3 * std::cos (phase3 + phaseOff3));
    }

	inline void boundPhase () noexcept
    {
        while (phase1 >= MathConstants<float>::twoPi)
            phase1 -= MathConstants<float>::twoPi;
        while (phase2 >= MathConstants<float>::twoPi)
            phase2 -= MathConstants<float>::twoPi;
        while (phase3 >= MathConstants<float>::twoPi)
            phase3 -= MathConstants<float>::twoPi;
    }

private:
	QDelayAudioProcessor& audioProcessor;
    float depth = 0.f;
    float rate = 0.f;
	float srate = 44100.0f;
	RCFilter depthSmooth;
	float phase1 = 0.f;
    float phase2 = 0.f;
    float phase3 = 0.f;
	float amp1 = 0.0f;
    float amp2 = 0.0f;
    float amp3 = 0.0f;
	float angleDelta1 = 0.0f;
    float angleDelta2 = 0.0f;
    float angleDelta3 = 0.0f;
	static constexpr float phaseOff1 = 0.0f;
    static constexpr float phaseOff2 = 13.0f * MathConstants<float>::pi / 4.0f;
    static constexpr float phaseOff3 = -MathConstants<float>::pi / 10.0f;
};