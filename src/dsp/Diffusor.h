#pragma once
#include <array>
#include <vector>
#include "Utils.h"
#include "DelayLine.h"

class Diffusor
{
public:
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
			size.setup(0.1f, srate);
		}

		void setSize(float _size, bool smooth=true)
		{
			sizeTarg = _size;
			if (smooth)
				size.reset(sizeTarg);
		}

		inline float allPass(float in, float feedback) {
			float offset = size.process(sizeTarg);
			float fb = delay.read(offset);

			auto out = fb - in * feedback;
			delay.write(in + out * feedback);

			return out;
		}

		void clear() {
			delay.clear();
			size.reset(sizeTarg);
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



	static constexpr int NUM_ALLPASS = 8;
	static constexpr float REF_SRATE = 44100.f;

	Diffusor() {}
	~Diffusor() {}

	int getNextPrime(int value);
	void prepare(float _srate);
	void setSize(float size);
	void process(float& left, float& right);
	void processBlock(float* left, float* right, int nsamps);
	void setSmear(float _smear) { smear = _smear; }
	void clear();

private:
	float srateFactor = 1.f;
	float currSize = 0.25f;
	float smear = 0.5f;
	std::array<AllPass, NUM_ALLPASS> allpassL;
	std::array<AllPass, NUM_ALLPASS> allpassR;
};