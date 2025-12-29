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

	Mode mode = Mode::Tape;
	float color = -1.f;
	float drive = -1.f;
	float driveGain = 0.f;
	float trimGain = 0.f;
	float trim = -100.f;
	float drift = 0.f;
	float bias = 0.f;
	float dc_alpha = 0.995f;
	float drift_alpha = 0.999f;

	Distortion(QDelayAudioProcessor& p);
	~Distortion();

	void prepare(float _srate);
	void onSlider();
	float saturate(float x, float& dc) const;
	void process(float& left, float& right, float drygain, float wetgain);
	void processBlock(float* left, float* right, int nsamps, float drygain, float wetgain);
	void clear();

private:
	float srate = 88200.0;
	QDelayAudioProcessor& audioProcessor;

	
	float k1 = 0.f;
	float k2 = 0.f;
	float k3 = 0.f;
	float g3 = 0.f;
	float y1l = 0.f;
	float y2l = 0.f;
	float y3l = 0.f;
	float y1r = 0.f;
	float y2r = 0.f;
	float y3r = 0.f;

	// dc blocker memory
	float dc_y1l = 0.f;
	float dc_y2l = 0.f;
	float dc_y1r = 0.f;
	float dc_y2r = 0.f;
};