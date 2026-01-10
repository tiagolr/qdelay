// Copyright (C) 2026 tilr
// Bit Crusher based of Signal Crusher JSFX by chokehold
// for better comments and insights see https://github.com/chkhld/jsfx/blob/main/Lo-Fi/signal_crusher.jsfx

#pragma once
#include <JuceHeader.h>

class QDelayAudioProcessor;

struct LPFilter // high order cascaded butterworth LP filter
{
	static constexpr int MAX_ORDER = 16;

	std::array<float, MAX_ORDER> a;
	std::array<float, MAX_ORDER> d1;
	std::array<float, MAX_ORDER> d2;
	std::array<float, MAX_ORDER> w0;
	std::array<float, MAX_ORDER> w1;
	std::array<float, MAX_ORDER> w2;
	int order = -1;
	float cutoff = -1;
	float srate = 88200.f;

	void LP(float Hz, int _order, float _srate)
	{
		_order = std::clamp(_order, 1, MAX_ORDER);
		Hz = std::min(Hz, std::min(0.5f * _srate, 20000.f));
		if (order == _order && cutoff == Hz && srate == _srate)
			return;

		cutoff = Hz;
		order = _order;
		srate = _srate;

		float a1 = std::tan(MathConstants<float>::pi * (cutoff / srate)); 
		float a2 = a1 * a1; 
		float ro4 = 1.f / (4.f * order); 
		int step = 0;

		while (step < order)
		{
			float r = std::sin(MathConstants<float>::pi * (2.f * step + 1.f) * ro4); 
			float ar2 = 2.f * a1 * r;
			float s2 = a2 + ar2 + 1.0f; 
			float rs2 = 1.0f / s2; 
			a[step] = a2 * rs2;
			d1[step] = 2.0f * (1.0f - a2) * rs2; d2[step] = -(a2 - ar2 + 1.0f) * rs2;
			step += 1;
		}
	}
	
	float process(float sample)
	{
		float output = sample;
		int step = 0;
		while (step < order)
		{
			w0[step] = d1[step] * w1[step] + d2[step] * w2[step] + output;
			output = a[step] * (w0[step] + 2.f * w1[step] + w2[step]);
			w2[step] = w1[step]; w1[step] = w0[step]; step += 1;
		};
		return output;
	}

	void clear()
	{
		std::fill(w0.begin(), w0.end(), 0.f);
		std::fill(w1.begin(), w1.end(), 0.f);
		std::fill(w2.begin(), w2.end(), 0.f);
	}
};

class Crusher
{
public:
	enum DSMode
	{
		Repeat,
		Zero,
		Interpolate
	};

	DSMode dsmode = Interpolate;
	bool upsample = true;
	float bits = 24.f;
	float downsrate = 88200.f;
	int ratio = 1;

	Crusher(QDelayAudioProcessor& p);
	~Crusher();

	void prepare(float _srate);
	void onSlider();
	void processBlock(float* left, float* right, int nsamps);
	void process(float& left, float& right);
	void clear();
	void dcBlock(float& left, float& right);

private:
	LPFilter dnFilterL{};
	LPFilter dnFilterR{};
	LPFilter upFilterL{};
	LPFilter upFilterR{};
	float srate = 88200.f;
	float bitsLevel = 1.f;
	QDelayAudioProcessor& audioProcessor;

	float bitReduce(float x) const;
	void downSample(float& left, float& right);
	void upSample(float& left, float& right);

	int counterDS = 1;
	int counterUS = 1;

	float stateDS_L = 0.f;
	float stateUS_L = 0.f;
	float lastStateDS_L = 0.f;
	float lastStateUS_L = 0.f;
	float stateDS_R = 0.f;
	float stateUS_R = 0.f;
	float lastStateDS_R = 0.f;
	float lastStateUS_R = 0.f;

	float dc_r = 0.f;
	float dc_inL = 0.f;
	float dc_outL = 0.f;
	float dc_inR = 0.f;
	float dc_outR = 0.f;
};