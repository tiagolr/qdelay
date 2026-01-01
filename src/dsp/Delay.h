// Copyright (C) 2025 tilr
#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"
#include "Utils.h"
#include "SVF.h"
#include "Pitcher.h"

class QDelayAudioProcessor;

class Delay : private juce::AudioProcessorValueTreeState::Listener
{
public:
	enum DelayMode
	{
		Normal,
		PingPong,
		Tap
	};

	enum SyncMode
	{
		RateHz,
		Straight,
		Triplet,
		Dotted
	};

	std::unique_ptr<Pitcher> pitcher;
	std::unique_ptr<Pitcher> pitcherSwing;

	// exposed variables for UI draw
	float feedbackL = 1.f;
	float feedbackR = 1.f;

	Delay(QDelayAudioProcessor& p);
	~Delay();

	void prepare(float _srate);

	std::array<int, 2> getTimeSamples(bool forceSync = false);
	int getFeelOffset(int timeL, int timeR, float swing);
	void processBlock(float* left, float* right, int nsamps);
	void clear();
	void setEqualizer(std::vector<SVF::EQBand> bands);
	void onSlider();

	void parameterChanged(const String& paramId, float value) override;

private:
	std::vector<SVF::EQBand> eqBands;
	std::vector<SVF> eqL;
	std::vector<SVF> eqSwingL;
	std::vector<SVF> eqR;
	std::vector<SVF> eqSwingR;
	QDelayAudioProcessor& audioProcessor;
	RCFilter timeL{};
	RCFilter timeR{};
	RCFilter swingSmooth{};
	RCFilter feelSmooth{};
	RCFilter modDepthSmooth{};
	DelayLine predelayL{};
	DelayLine predelayR{};
	DelayLine delayL{};
	DelayLine delayR{};
	DelayLine swingL{};
	DelayLine swingR{};
	DelayLine haasL{};
	DelayLine haasR{};
	DelayLine haasSwingL{};
	DelayLine haasSwingR{};

	float srate = 44100.f;
	float israte = 1.f / 44100.f;
	float modPhase = 0.f;
	float distWet = 0.f;
	float distDry = 1.f;
};