// Copyright 2025 tilr
// Based off Vital synth one_pole
#pragma once
#include <JuceHeader.h>
#include <cmath>
#include "Utils.h"

class OnePole {
public:
    inline static LookupTable coeffLUT = LookupTable(
		[] (float ratio) {
			constexpr float kMaxRads = 0.499f * juce::MathConstants<float>::pi;
			float scaled = ratio * juce::MathConstants<float>::pi;
			return std::tan(juce::jmin(kMaxRads, (scaled / (scaled + 1.f))));
		},
		0.0f, 0.5f, 2048
	);

    float coeff = 0.0f;
    float state = 0.0f;
    float curr = 0.0f;

    OnePole() {}

    void init(float freq, float srate) {
        freq = std::clamp(freq, 20.f, srate * 0.48f);
		float ratio = std::clamp(freq / srate, 0.0f, 0.5f);
		coeff = coeffLUT.cubic(ratio);
    }

    float eval(float sample) {
        float delta = coeff * (sample - state);
        state += delta;
        curr = state;
        state += delta;
        return curr;
    }

    void reset(float sample) {
        state = sample;
        curr = sample;
    }
};