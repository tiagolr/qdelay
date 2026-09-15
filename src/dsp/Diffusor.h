#pragma once
#include <array>
#include <vector>
#include "Utils.h"
#include "DelayLine.h"

class DiffusorBase
{
public:
	virtual ~DiffusorBase() = default;
	virtual void prepare(float _srate) = 0;
	virtual void setSize(float size, bool smooth = true) = 0;
	virtual void processBlock(float* left, float* right, int nsamps, float drymix, float wetmix) = 0;
	virtual void clear() = 0;
};

class Diffusor : public DiffusorBase
{
public:
	static constexpr int NUM_ALLPASS = 8;
	static constexpr float REF_SRATE = 44100.f;
	static constexpr float MOD_MAX_DEPTH = 2.f; // max 5 samples of modulation per all pass
	static constexpr float MOD_MAX_RATE = 2.f; // max 2 hz of modulation rate

	struct AllPass {
		DelayLine delay;
		RCFilter size;
		float sizeTarg = 0.f;

		AllPass()
		{
			delay.resize(1 << 15);
			size.eps = 0.1f;
		}

		void init(float srate) {
			size.setup(.25f * (REF_SRATE / srate), srate);
		}

		void setSize(int _size, bool smooth = true)
		{
			sizeTarg = (float)_size;
			if (!smooth)
				size.reset(sizeTarg);
		}

		inline float allPass(float in, float g, float modulation = 0.f) {
			float offset = size.process(sizeTarg);
			float feedbk = delay.read(offset + modulation);

			auto out = feedbk - in * g;
			delay.write(in + out * g);

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

	std::array<float, NUM_ALLPASS> modPhases{};
	std::array<float, NUM_ALLPASS> modPhasesInc{};

	Diffusor()
	{
		for (int i = 0; i < NUM_ALLPASS; ++i) {
			modPhases[i] = rand() / (float)RAND_MAX;
			delays[i].resize(1 << 15);
		}
	}
	~Diffusor() {}

	int getNextPrime(int value);
	void prepare(float _srate) override;
	void setSize(float size, bool smooth = true) override;
	void processBlock(float* left, float* right, int nsamps, float drymix, float wetmix) override;
	void setSmear(float _smear) { smear = _smear; }
	void clear() override;


	std::array<float, 8> hadamard_8x8(std::array<float, 8> x);

private:
	float srateFactor = 1.f;
	float smear = 0.5f;
	float sizenorm = 0.f;
	float t60 = 0.f; // decay
	std::array<AllPass, NUM_ALLPASS> allpass;
	std::array<DelayLine, NUM_ALLPASS> delays;
	std::array<float, NUM_ALLPASS> fb = { 0 }; // feedback

	float triangle(float phase);
};

class DiffusorLegacy : public DiffusorBase
{
public:
	static constexpr int NUM_ALLPASS = 8;

	struct AllPass {
		float srate = 0.f;
		int size = 1;
		float dist = 0.f;
		std::vector<float> buf{};
		RCFilter offsetSmooth{};
		float offset = 0.f;
		int pos = 0;

		void init(float _srate, float apdist, float distance) {
			srate = _srate;
			buf.clear();
			dist = apdist;
			size = (int)(apdist * distance);
			buf.resize(size, 0.f);
			offsetSmooth.setup(0.1f, srate);
		}

		void setSizeOffsets(float _size) {
			offset = (int)buf.size() * _size;
		}

		inline float allPass(float in, float feedback) {
			auto fp = pos + offset;
			auto ip = (int)floor(fp);
			auto frc = fp - ip;
			ip = ip % size;

			auto out = (buf[ip] + (buf[(ip + 1) % size] - buf[ip]) * frc) - in * feedback;
			buf[pos] = in + out * feedback;
			pos = (pos + 1) % size;

			return out;
		}

		void clear() {
			std::fill(buf.begin(), buf.end(), 0.f);
			pos = 0;
		}
	};

	DiffusorLegacy() {}
	~DiffusorLegacy() {}

	void prepare(float _srate) override;
	void setSize(float size, bool) override;
	void processBlock(float* left, float* right, int nsamps, float drymix, float wetmix) override;
	void clear() override;

private:
	std::array<AllPass, NUM_ALLPASS> allpassL;
	std::array<AllPass, NUM_ALLPASS> allpassR;

	float mps = 0.007f; // meters per second
	float distance = 0.5; // distance in meters
	float srate = 44100.f;
	float smear = 0.75f;
};