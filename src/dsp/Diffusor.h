#pragma once
#include <array>
#include <vector>
#include "Utils.h"
#include "DelayLine.h"

class Diffusor
{
public:
	static constexpr int NUM_ALLPASS = 8;
	static constexpr float REF_SRATE = 44100.f;
	static constexpr float MOD_MAX_DEPTH = 5.f; // max 5 samples of modulation per all pass
	static constexpr float MOD_MAX_RATE = 2.f; // max 5 hz of modulation rate

	struct AllPass {
		DelayLine delay;
		RCFilter size;
		float g = 0.f;
		float sizeTarg = 0.f;

		AllPass()
		{
			delay.resize(1<<15);
			size.eps = 0.01f;
		}

		void init(float srate) {
			size.setup(0.25f * (REF_SRATE / srate), srate);
		}

		void setSize(int _size, bool smooth=true)
		{
			sizeTarg = (float)_size;
			if (smooth)
				size.reset(sizeTarg);
		}

		inline float allPass(float in, float feedback, float modulation = 0.f) {
			float offset = size.process(sizeTarg);
			float fb = delay.read(offset + modulation);

			auto out = fb - in * feedback;
			delay.write(in + out * feedback);

			return out;
		}

		void clear() {
			delay.clear();
			size.reset(sizeTarg);
		}
	};

	struct NestedAllPass {
		DelayLine delay1;
		DelayLine delay2;
		RCFilter size1;
		RCFilter size2;
		float g = 0.f;
		float sizeTarg1 = 0.f;
		float sizeTarg2 = 0.f;

		NestedAllPass()
		{
			delay1.resize(1 << 15);
			delay2.resize(1 << 15);
			size1.eps = 0.01f;
			size2.eps = 0.01f;
		}

		void init(float srate) {
			size1.setup(0.25f * (REF_SRATE / srate), srate);
			size2.setup(0.25f * (REF_SRATE / srate), srate);
		}

		void setSize(int _size1, int _size2, bool smooth = true)
		{
			sizeTarg1 = (float)_size1;
			sizeTarg2 = (float)_size2;
			if (smooth) {
				size1.reset(sizeTarg1);
				size2.reset(sizeTarg2);
			}
		}

		inline float allPass(float in, float fb, float mod1 = 0.f, float mod2 = 0.f) {
			const float offset1 = size1.process(sizeTarg1);
			const float offset2 = size2.process(sizeTarg2);

			const float outerDelay = delay1.read(offset1 + mod1);
			const float outerIn = in + outerDelay * fb;

			const float innerDelay = delay2.read(offset2 + mod2);
			const float innerOut = innerDelay - outerIn * fb;
			delay2.write(outerIn + innerOut * fb);
			delay1.write(innerOut);

			return outerDelay - in * fb;
		}

		void clear() {
			delay1.clear();
			delay2.clear();
			size1.reset(sizeTarg1);
			size2.reset(sizeTarg2);
		}
	};

    static constexpr int MAX_PRIME = 100000;

    static const std::vector<int>& get_primes() {
        static const std::vector<int> primes = []() {
            std::vector<bool> is_prime(MAX_PRIME + 1, true);
            is_prime[0] = is_prime[1] = false;

            for (int p = 2; p * p <= MAX_PRIME; ++p) {
                if (is_prime[p]) {
                    for (int i = p * p; i <= MAX_PRIME; i += p) {
                        is_prime[i] = false;
                    }
                }
            }

            std::vector<int> list;
            for (int p = 2; p <= MAX_PRIME; ++p) {
                if (is_prime[p]) {
                    list.push_back(p);
                }
            }
            return list;
        }();

        return primes;
    }

	std::array<float, NUM_ALLPASS> modPhasesL{};
	std::array<float, NUM_ALLPASS> modPhasesIncL{};
	std::array<float, NUM_ALLPASS> modPhasesR{};
	std::array<float, NUM_ALLPASS> modPhasesIncR{};

	Diffusor()
	{
		for (int i = 0; i < NUM_ALLPASS; ++i) {
			modPhasesL[i] = rand() / (float)RAND_MAX;
			modPhasesR[i] = rand() / (float)RAND_MAX;
		}
	}
	~Diffusor() {}

	int getNextPrime(int value);
	void prepare(float _srate);
	void setSize(float size, bool smooth = true);
	void processBlock(float* left, float* right, int nsamps);
	void setSmear(float _smear) { smear = _smear; }
	void clear();

private:
	float srateFactor = 1.f;
	float currSize = 0.25f;
	float smear = 0.5f;
	std::array<NestedAllPass, NUM_ALLPASS / 2> allpassL;
	std::array<NestedAllPass, NUM_ALLPASS / 2> allpassR;

	float triangle(float phase);
};