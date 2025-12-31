// Copyright (C) 2025 tilr
// Tape distortion based of JSFXClones/JClones_TapeHead
#pragma once
#include <JuceHeader.h>

class QDelayAudioProcessor;

class Distortion
{
public:
	struct Dynamics
	{
		float env_ca = 0.f;
		float env_cr = 0.f;
		float s = 0.f;
		float ms1 = 0.f;
		float ms2 = 0.f;
		float denv = 0.f;
		float env = 0.f;
		int k = 0;

		void prepare(float srate)
		{
			env_ca = 2.f / (srate * 0.001f);
			env_cr = 2.f / (srate * 0.050f);
		}

		void process(float x)
		{
			s = x * x;
			ms1 += (s > ms1 ? env_ca : env_cr) * (s - ms1);
			ms2 += (ms1 > ms2 ? env_ca : env_cr) * (ms1 - ms2);
			if (k <= 0) {
				denv = (std::sqrt(ms2) - env) * 0.125f;
				k = 8;
			}
			k -= 1;
			env += denv;
		}
	};

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
	float dynamics = 0.f;
	Dynamics dyn_l;
	Dynamics dyn_r;

	Distortion(QDelayAudioProcessor& p);
	~Distortion();

	void prepare(float _srate);
	void onSlider();
	float saturate(float x, float& dc) const;
	void processBlock(float* left, float* right, int nsamps, float drygain, float wetgain);
	void clear();

private:
	float srate = 88200.0;
	QDelayAudioProcessor& audioProcessor;
	
	// coeffs
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