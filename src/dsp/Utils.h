#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <functional>
#include <algorithm>
#include "../Globals.h"

using namespace globals;

/*
* A copy of LookupTableTransform with additional cubic interpolation
*/
class LookupTable
{
public:
    LookupTable() = default;

    template <typename Func>
    LookupTable(Func fn, float min_, float max_, size_t size_)
    {
        init(fn, min_, max_, size_);
    }

    template <typename Func>
    void init(Func fn, float min_, float max_, size_t size_)
    {
        if (max_ <= min_) throw std::invalid_argument("max must be greater than min");
        if (size_ < 2) throw std::invalid_argument("size must be at least 2");
        min = min_;
        max = max_;
        size = size_;
        values.resize(size);

        scaler = (size > 1) ? (size - 1) / (max - min) : 0.0f;
        offset = -min * scaler;

        for (size_t i = 0; i < size; ++i) {
            float x = static_cast<float>(i) / (size - 1); // Normalized [0, 1]
            float mappedX = min + x * (max - min);
            mappedX = std::clamp(mappedX, min, max);
            values[i] = fn(mappedX);
        }
    }

    inline float operator()(float input) const
    {
        input = std::clamp(input, min, max);
        float normalizedIndex = input * scaler + offset;
        size_t index = static_cast<size_t>(std::floor(normalizedIndex));

        if (index >= size - 1)
            return values.back();

        float frac = normalizedIndex - index;
        return values[index] + frac * (values[index + 1] - values[index]);
    }

    inline float cubic(float input) const
    {
        input = std::clamp(input, min, max);
        float index = input * scaler + offset;
        int i = (int)index;
        float t = index - i;

        int i0 = std::max(0, i - 1);
        int i1 = i;
        int i2 = std::min((int)size - 1, i + 1);
        int i3 = std::min((int)size - 1, i + 2);

        float y0 = values[i0];
        float y1 = values[i1];
        float y2 = values[i2];
        float y3 = values[i3];

        float a0 = y3 - y2 - y0 + y1;
        float a1 = y0 - y1 - a0;
        float a2 = y2 - y0;
        float a3 = y1;

        return (a0 * t * t * t) + (a1 * t * t) + (a2 * t) + a3;
    }

    const std::vector<float>& getValues() const { return values; }
    size_t getSize() const { return size; }
    float getMin() const { return min; }
    float getMax() const { return max; }

private:
    std::vector<float> values;
    float min = 0.0f;
    float max = 1.0f;
    float scaler = 0.0f;
    float offset = 0.0f;
    size_t size = 0;
};

class Lerp {
	float value, target;
	float step = 0.0f;
	int samplesLeft = 0;
	int duration = 0;
	bool isReset = true;

public:
	Lerp(float start = 0.0) : value(start), target(start) {}

	void setDuration(int duration_) {
		duration = duration_;
	}

	void set(float target_) {
		target = target_;
		if (duration > 0 && !isReset) {
			samplesLeft = duration;
			step = (target - value) / samplesLeft;
		} else {
			value = target;
			step = 0.0;
			samplesLeft = 0;
			isReset = false;
		}
	}

	void tick() {
		if (samplesLeft > 0) {
			value += step;
			--samplesLeft;
		}
	}

	void reset() {
		isReset = true;
		value = target;
		samplesLeft = 0;
		step = 0.0;
	}
	inline float get() const { return value; }
	bool isDone() const { return samplesLeft == 0; }
};

class Utils
{
public:
    inline static double normalToFreq(double norm)
    {
        return F_MIN_FREQ * std::exp(norm * std::log(F_MAX_FREQ / F_MIN_FREQ));
    }

    inline static double freqToNormal(double norm)
    {
        return std::log(norm / F_MIN_FREQ) / std::log(F_MAX_FREQ / F_MIN_FREQ);
    }

    inline static double gainTodB(double gain)
    {
        return gain == 0 ? -60.0 : 20.0 * std::log10(gain);
    }

    static float normalToFreqf(float min, float max, float norm)
    {
        return min * std::exp(norm * std::log(max / min));
    }

    static float freqToNormalf(float min, float max, float norm)
    {
        return std::log(norm / min) / std::log(max / min);
    }

    // LUT used to balance mix without losing amplitude
    static const LookupTable& sinHalfPi()
    {
        static LookupTable table = []()
            {
                LookupTable t;
                t.init([](float norm) {
                    return std::sin(norm * MathConstants<float>::halfPi);
                    }, 0.0f, 1.0f, 256);
                return t;
            }();
        return table;
    }

    // LUT used to balance mix without losing amplitude
    static const LookupTable& cosHalfPi()
    {
        static LookupTable table = []()
            {
                LookupTable t;
                t.init([](float norm) {
                    return std::cos(norm * MathConstants<float>::halfPi);
                    }, 0.0f, 1.0f, 256);
                return t;
            }();
        return table;
    }
};

/**
 * Used to smooth out params
 */
class RCFilter
{
public:
    double r = 1.0;
    double state = 0.0;
    float eps = 1e-6f;

    void setup(float resistance, float _srate)
    {
        r = 1.0 / (resistance * _srate + 1);
    }

    float process(float input)
    {
        double targ = (double)input;
        state += r * (targ - state);
        if (std::fabs(targ - state) < eps) // snap
            state = targ;

        return (float)state;
    }

    void reset(float value = 0.0f)
    {
        state = (double)value;
    }
};

class RCFilterBlock
{
public:
    float r = 1.0f;
    float k = 1.f;
    float state = 0.0f;
    float output = 0.0f;
    float srate;
    float resistance = 0.f;

    void setup(float _resistance, float _srate)
    {
        resistance = _resistance;
        srate = _srate;
        r = 1.0f / (_resistance * srate + 1);
        k = _resistance <= 0.f ? 0.f : -srate * std::log(1.f - r);
    }

    float process(float input, float dt)
    {
        // no smoothing
        if (r >= 1.f) {
            state = input;
        }
        else {
            float alpha = 1.f - std::exp(-k * dt);
            state += alpha * (input - state);
        }

        output = state;
        return output;
    }

    void reset(float value = 0.0f)
    {
        output = state = value;
    }
};

class NoiseGen {
public:
    NoiseGen(uint32_t seed)
        : original_seed(seed), state(seed) {}

    float next()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;

        // Convert to float in [-1, 1]
        return (int32_t)x * mult; // float(x)/2^31
    }

    float gaussian()
    {
        float g = 0.0f;
        for (int i = 0; i < 8; ++i)
            g += next();

        return g * 0.612f; // unit variance
    }

    void reset()
    {
        state = original_seed;
    }

    void reseed(uint32_t seed)
    {
        state = seed;
    }

private:
    const float mult = 1.0f / 2147483648.0f;
    uint32_t original_seed;
    uint32_t state;
};