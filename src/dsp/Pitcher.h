// Port of Saikes pitch shifting library to c++ and Juce
// Tilr 2026

// WSOLA Pitch shifting library
// version: 0.01
// author: Joep Vanlier
// license: MIT
// (C) Joep Vanlier 2020

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <algorithm>
#include <cmath>

class RingBuffer
{
public:
	std::vector<float> left, right;
	int writePos = 0;
	int size = 0;
	float outL = 0.f;
	float outR = 0.f;

	void init(int _size)
	{
		size = _size;
		left.resize(size, 0.f);
		right.resize(size, 0.f);
		while (writePos >= size)
			writePos -= size;
	}

	inline void write(float l, float r)
	{
		left[writePos] = l;
		right[writePos] = r;
		writePos = (writePos + 1) % size;
	}

	inline int wrap (int idx) const
    {
        while (idx < 0) idx += size;
        while (idx >= size) idx -= size;
        return idx;
    }

	inline void read(float dtime)
	{
		// 1. Absolute fractional read position
		float readPos = (float)writePos - dtime;
		while (readPos < 0.f)         readPos += (float)size;
		while (readPos >= (float)size) readPos -= (float)size;

		// 2. Integer position and fraction
		int i1 = (int)std::floor(readPos); // sample at t = 0
		float t = readPos - (float)i1;     // fractional part [0,1)

		// 3. Neighboring samples
		int i0 = wrap(i1 - 1);
		int i2 = wrap(i1 + 1);
		int i3 = wrap(i1 + 2);

		// 4. Catmull–Rom spline
		float t2 = t * t;
		float t3 = t2 * t;

		float a0 = -0.5f * t3 + t2 - 0.5f * t;
		float a1 = 1.5f * t3 - 2.5f * t2 + 1.0f;
		float a2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
		float a3 = 0.5f * t3 - 0.5f * t2;

		// 5. Interpolate
		outL = a0 * left[i0]
			+ a1 * left[i1]
			+ a2 * left[i2]
			+ a3 * left[i3];

		outR = a0 * right[i0]
			+ a1 * right[i1]
			+ a2 * right[i2]
			+ a3 * right[i3];
	}

	// Copies 'copyLength' samples from 'delay' behind writePos into a linear target array
	// channel = true => left, false => right
	void copyFromBuffer(float* target, int delay, int copyLength, bool channel) const
	{
	    const auto& buf = channel ? left : right;

	    int start = writePos - delay;
	    while (start < 0) start += size; // wrap negative
	    start %= size;

	    int remaining = copyLength;
	    int pos = start;

	    while (remaining > 0)
	    {
	        int chunk = std::min(size - pos, remaining);
	        std::memcpy(target + (copyLength - remaining), &buf[pos], chunk * sizeof(float));
	        remaining -= chunk;
	        pos = (pos + chunk) % size; // wrap
	    }
	}
};

class Pitcher
{
public:
	struct CosineFade
	{
	    float y0, y1, y2, b1;
	    float count;
		float Nc; // store previous window length
		float w;

		void prepare(float N)
		{
			Nc = N;
			w = MathConstants<float>::pi / (N - 1);
			float ip = MathConstants<float>::halfPi;

			count = N;
			b1 = 2.0f * std::cos(w);
			y1 = std::sin(ip - w);
			y2 = std::sin(ip - 2.0f * w);
		}

	    inline void update(float N)
	    {
	        if (count <= 0) return;
    		w = juce::MathConstants<float>::pi / (N-1);
    		b1 = 2.0f * std::cos(w);
    		count *= N / Nc; // preserve phase proportion
    		Nc = N;
	    }

		inline void eval()
		{
			y0 = b1 * y1 - y2;
        	y2 = y1;
        	y1 = y0;
        	w = 0.5f * (y0 + 1.0f);
        	w = std::clamp(w, 0.0f, 1.0f);
			count -= 1.f;
		}
	};

	enum WindowMode
	{
		kSmall,   // drums
		kMedium,   // general
		kLarge,  // vocals
	};

	// fft order
	static constexpr int O_WIN_SMALL  = 7;
	static constexpr int O_WIN_MEDIUM = 9;
	static constexpr int O_WIN_LARGE  = 11;

	std::array<int, 3> windowSizes {
		1 << O_WIN_SMALL,
		1 << O_WIN_MEDIUM,
		1 << O_WIN_LARGE
	};
	std::array<int, 3> fftOrders { O_WIN_SMALL, O_WIN_MEDIUM, O_WIN_LARGE };
	std::array<bool, 3> acfFlags { false, true, true };

	struct FFTSet
	{
		juce::dsp::FFT fftSmall  { O_WIN_SMALL  };
		juce::dsp::FFT fftMedium { O_WIN_MEDIUM };
		juce::dsp::FFT fftLarge  { O_WIN_LARGE  };

		juce::dsp::FFT& get (WindowMode w)
		{
			switch (w)
			{
				case WindowMode::kSmall:  return fftSmall;
				case WindowMode::kMedium: return fftMedium;
				case WindowMode::kLarge:  return fftLarge;
				default: jassertfalse; return fftMedium;
			}
		}
	};

	float outL = 0.f;
	float outR = 0.f;

	void init(WindowMode mode);
	float computeMaxACFPosition();
	void update(float l, float r);
	void setSpeed(float newHeadSpeed);
	void setSpeedSemis(float semis);

private:
	WindowMode mode = WindowMode::kSmall;
	RingBuffer buffer; //
	CosineFade fader;
	FFTSet fftSet;
	std::vector<float> fftmem1; // Fft buffers used for phase alignment (only needed when use_acf = 1).
	std::vector<float> fftmem2;
	float readHead1 = 0.f;
	float readHead2 = 0.f;
	float lastHeadSpeed = 0.f;
	float readHeadSpeed = 0.f;
	float speed = 0.f;
	bool fade = false;
	bool acf = false;
	int crossFadeSamples = 0;
};