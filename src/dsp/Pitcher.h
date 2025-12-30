// Port of Saikes pitch shifting library to c++ and Juce for FFTs
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
		writePos = (writePos + 1) & size;
	}

	inline int wrap (int idx)
    {
        while (idx < 0) idx += size;
        while (idx >= size) idx -= size;
        return idx;
    }

    inline void read(float delay)
    {
        float readPos = static_cast<float>(writePos) - delay;
        while (readPos < 0.f) readPos += static_cast<float>(size);
        while (readPos >= static_cast<float>(size)) readPos -= static_cast<float>(size);

        int idx0 = static_cast<int>(std::floor(readPos));
        float t = readPos - idx0; // fractional part

        // 5-tap Lagrange interpolation coefficients
        float tm1 = t - 1.f;
        float tm2 = t - 2.f;
        float tm3 = t - 3.f;
        float tm4 = t - 4.f;

        float a0 = tm1 * tm2 * tm3 * tm4 / 24.f;
        float a1 = -t * tm2 * tm3 * tm4 / 6.f;
        float a2 = t * tm1 * tm3 * tm4 / 4.f;
        float a3 = -t * tm1 * tm2 * tm4 / 6.f;
        float a4 = t * tm1 * tm2 * tm3 / 24.f;

        int i0 = wrap(idx0);
        int i1 = wrap(idx0 - 1);
        int i2 = wrap(idx0 - 2);
        int i3 = wrap(idx0 - 3);
        int i4 = wrap(idx0 - 4);

        outL = a0 * left[i0] + a1 * left[i1] + a2 * left[i2] + a3 * left[i3] + a4 * left[i4];
        outR = a0 * right[i0] + a1 * right[i1] + a2 * right[i2] + a3 * right[i3] + a4 * right[i4];
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
			w = juce::MathConstants<float>::pi / (N - 1);
			b1 = 2.0f * std::cos(w);
			y1 = std::sin(0.5f * juce::MathConstants<float>::pi - w);
			y2 = std::sin(0.5f * juce::MathConstants<float>::pi - 2.0f * w);

			if (Nc > 0) // if previously prepared
				count *= N/Nc;

			Nc = N;
		}

	    inline void next(float N)
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
		}
	};

	enum WindowSize
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

		juce::dsp::FFT& get (WindowSize w)
		{
			switch (w)
			{
				case WindowSize::kSmall:  return fftSmall;
				case WindowSize::kMedium: return fftMedium;
				case WindowSize::kLarge:  return fftLarge;
				default: jassertfalse; return fftMedium;
			}
		}
	};

	WindowSize windowSize = WindowSize::kSmall;
	RingBuffer buffer; //
	CosineFade fader;
	FFTSet fftSet;
	std::vector<float> fftmem1; // Fft buffers used for phase alignment (only needed when use_acf = 1).
	std::vector<float> fftmem2;
	float readHead1 = 0.f;
	float readHead2 = 0.f;
	float lastHeadSpeed = 0.f;
	float readHeadSpeed = 0.f;
	int fadeCount = 0;
	float speed = 0.f;
	bool fade = false;
	bool acf = false;
	int crossFadeSamples = 0;
	float outL = 0.f;
	float outR = 0.f;

	void init(bool use_acf, WindowSize size);
	float computeMaxACFPosition();
	void update(float l, float r);
	void setPitchShifterSpeed(float newHeadSpeed);
};