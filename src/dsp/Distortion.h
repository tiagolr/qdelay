// Copyright (C) 2025 tilr
// Tape distortion based of JSFXClones/JClones_TapeHead
#pragma once
#include <JuceHeader.h>

class QDelayAudioProcessor;

class Distortion
{
public:
	enum Mode
	{
		Tape,
		Tanh
	};

	Distortion(QDelayAudioProcessor& p);
	~Distortion();

	void prepare(float _srate);

	void processBlock(float* left, float* right, int nsamps);
	void clear();

private:
	float srate = 88200.0;
	QDelayAudioProcessor& audioProcessor;
};