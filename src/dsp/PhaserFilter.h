// Copyright 2025 tilr
// Based off Vital synth phaser filter
#pragma once

#include <JuceHeader.h>
#include <cmath>
#include "OnePole.h"
#include "Utils.h"

class PhaserFilter
{
public:
	inline static LookupTable coeffLUT = LookupTable(
		[] (float ratio) {
			constexpr float kMaxRads = 0.499f * juce::MathConstants<float>::pi;
			float scaled = ratio * juce::MathConstants<float>::pi;
			return std::tan(juce::jmin(kMaxRads, scaled));
		},
		0.0f, 0.5f, 2048
	);

	static constexpr int kPeakStage = 4;
	static constexpr int kMaxStages = 3 * kPeakStage;
	static constexpr float kClearRatio = 20.0f;

	PhaserFilter(){}
	~PhaserFilter(){}

	inline static float getCoeff(float freq, float srate) {
		freq = std::clamp(freq, 20.0f, srate * 0.48f);
		float ratio = std::clamp(freq / srate, 0.0f, 0.5f);
		return coeffLUT.cubic(ratio);
	}

	void init(float srate, float freq, float q);
	void reset(float sample);
	float eval(float sample);
	void setLerp(int duration);
	void setMorph(float norm) { morph = norm; };
	void tick();

private:
	float morph = 0.0f;
	Lerp g = 0.0f;
	Lerp k = 0.0f;

	OnePole remove_lows_stage;
	OnePole remove_highs_stage;
	OnePole stages[kMaxStages];
	float allpass_output = 0.0f;
};