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
	}

	setSize(currSize, false);
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

	for (int i = 0; i < NUM_ALLPASS / 2; ++i)
	{
		int sizeL1 = getNextPrime((int)scaleSize(std::floor(SIZE_L * size * 2), NUM_ALLPASS - 1 - i, scaleFactorL));
		int sizeL2 = getNextPrime((int)scaleSize(std::floor(SIZE_L * size), NUM_ALLPASS - 1 - i, scaleFactorL));
		int sizeR1 = getNextPrime((int)scaleSize(std::floor(SIZE_R * size * 2), NUM_ALLPASS - 1 - i, scaleFactorR));
		int sizeR2 = getNextPrime((int)scaleSize(std::floor(SIZE_R * size), NUM_ALLPASS - 1 - i, scaleFactorR));
		allpassL[i].setSize(sizeL1, sizeL2, smooth);
		allpassR[i].setSize(sizeR1, sizeR2, smooth);
	}
}

void Diffusor::processBlock(float* left, float* right, int nsamps)
{
	for (int sample = 0; sample < nsamps; ++sample) {

		float spl0 = left[sample];
		float spl1 = right[sample];

		for (int i = 0; i < NUM_ALLPASS / 2; ++i) {
			int i0 = i * 2;
			int i1 = i * 2 + 1;
			float mod00 = triangle(modPhasesL[i0]) * MOD_MAX_DEPTH;
			float mod01 = triangle(modPhasesL[i1]) * MOD_MAX_DEPTH;
			float mod10 = triangle(modPhasesR[i0]) * MOD_MAX_DEPTH;
			float mod11 = triangle(modPhasesR[i1]) * MOD_MAX_DEPTH;
			spl0 = allpassL[i].allPass(spl0, i % 2 == 0 ? smear : -smear, mod00, mod01);
			spl1 = allpassR[i].allPass(spl1, i % 2 == 0 ? -smear : smear, mod10, mod11);
			modPhasesL[i0] += modPhasesIncL[i0];
			modPhasesL[i1] += modPhasesIncL[i1];
			modPhasesR[i0] += modPhasesIncR[i0];
			modPhasesR[i1] += modPhasesIncR[i1];
			if (modPhasesL[i0] >= 1.f) modPhasesL[i0] -= 1.f;
			if (modPhasesL[i1] >= 1.f) modPhasesL[i1] -= 1.f;
			if (modPhasesR[i0] >= 1.f) modPhasesR[i0] -= 1.f;
			if (modPhasesR[i1] >= 1.f) modPhasesR[i1] -= 1.f;
		}

		left[sample] = spl0;
		right[sample] = spl1;
	}
}

float Diffusor::triangle(float phase)
{
	return 2.f - std::abs(2.0f * (phase - std::floor(phase)) - 1.0f) - 1.0f;
}