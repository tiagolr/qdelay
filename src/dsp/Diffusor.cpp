#include "Diffusor.h"

int Diffusor::getNextPrime(int value)
{
	const auto primes = get_primes();
    if (value <= primes.front()) return primes.front();
    if (value >= primes.back()) return value;
    auto it = std::lower_bound(primes.begin(), primes.end(), value);
    return *it;
}

void Diffusor::clear()
{
	setSize(currSize, false);

	for (int i = 0; i < NUM_ALLPASS / 2; i++)
	{
		allpassL[i].clear();
		allpassR[i].clear();
	}
}

void Diffusor::prepare(float _srate)
{
	srateFactor = _srate / REF_SRATE;

	for (int i = 0; i < NUM_ALLPASS; ++i)
	{
		modPhasesIncL[i] = (rand() / (float)RAND_MAX) * MOD_MAX_RATE / srateFactor / _srate;
		modPhasesIncR[i] = (rand() / (float)RAND_MAX) * MOD_MAX_RATE / srateFactor / _srate;
		allpassL[i].init(_srate);
		allpassR[i].init(_srate);
	}

	clear();
}

void Diffusor::setSize(float size, bool smooth)
{
	constexpr int SIZE_L = 180;
	constexpr int SIZE_R = 132;
	constexpr float scaleFactorL = 1.5123f; // geometric scale exponent
	constexpr float scaleFactorR = 1.6132f; // geometric scale exponent

	auto scaleSize = [](float val, int index, float scale)
	{
		return val * pow(scale, (float)index);
	};

	for (int i = 0; i < NUM_ALLPASS; ++i)
	{
		int sizeL = getNextPrime((int)scaleSize(std::floor(SIZE_L * size * 2), NUM_ALLPASS - 1 - i, scaleFactorL));
		int sizeR = getNextPrime((int)scaleSize(std::floor(SIZE_R * size * 2), NUM_ALLPASS - 1 - i, scaleFactorR));
		allpassL[i].setSize(sizeL, smooth);
		allpassR[i].setSize(sizeR, smooth);
	}
}

void Diffusor::processBlock(float* left, float* right, int nsamps)
{
	for (int sample = 0; sample < nsamps; ++sample) {

		float spl0 = left[sample];
		float spl1 = right[sample];

		for (int i = 0; i < NUM_ALLPASS; ++i) {
			float mod0 = triangle(modPhasesL[i]) * MOD_MAX_DEPTH;
			float mod1 = triangle(modPhasesR[i]) * MOD_MAX_DEPTH;
			spl0 = allpassL[i].allPass(spl0, i % 2 == 0 ? smear : -smear, mod0);
			spl1 = allpassR[i].allPass(spl1, i % 2 == 0 ? -smear : smear, mod1);
			modPhasesL[i] += modPhasesIncL[i];
			modPhasesR[i] += modPhasesIncR[i];
			if (modPhasesL[i] >= 1.f) modPhasesL[i] -= 1.f;
			if (modPhasesR[i] >= 1.f) modPhasesR[i] -= 1.f;
		}

		left[sample] = spl0;
		right[sample] = spl1;
	}
}

float Diffusor::triangle(float phase)
{
	return 2.f - std::abs(2.0f * (phase - std::floor(phase)) - 1.0f) - 1.0f;
}