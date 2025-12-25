// Copyright (C) 2025 tilr
#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"
#include "Utils.h"
#include "Diffusor.h"

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

	// exposed variables for UI draw
	float feedbackL = 1.f;
	float feedbackR = 1.f;

	Delay(QDelayAudioProcessor& p);
	~Delay();

	void prepare(float _srate);

	std::array<int, 2> getTimeSamples(bool forceSync = false);
	void processBlock(float* left, float* right, int nsamps);
	void clear();

	void parameterChanged(const String& paramId, float value) override;

private:
	QDelayAudioProcessor& audioProcessor;
	RCFilter timeL{};
	RCFilter timeR{};
	DelayLine predelayL{};
	DelayLine predelayR{};
	DelayLine delayL{};
	DelayLine delayR{};
	Diffusor diffusor{};
	float srate = 44100.f;
};