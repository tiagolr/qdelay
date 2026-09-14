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
	for (int i = 0; i < NUM_ALLPASS; i++)
	{
		allpassL[i].clear();
		allpassR[i].clear();
	}
}

void Diffusor::prepare(float _srate)
{
	srateFactor = _srate / REF_SRATE;
	clear();
	setSize(currSize);
}

void Diffusor::setSize(float size)
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
		int sizeL = getNextPrime(scaleSize((int)(SIZE_L * size * 2), NUM_ALLPASS - 1 - i, scaleFactorL));
		int sizeR = getNextPrime(scaleSize((int)(SIZE_R * size * 2), NUM_ALLPASS - 1 - i, scaleFactorR));
		allpassL[i].setSize(sizeL);
		allpassR[i].setSize(sizeR);
	}
}

void Diffusor::process(float& left, float& right)
{
	float spl0 = left;
	float spl1 = right;

	for (int i = 0; i < NUM_ALLPASS; ++i) {
		spl0 = allpassL[i].allPass(spl0, i % 2 == 0 ? smear : -smear);
		spl1 = allpassR[i].allPass(spl1, i % 2 == 0 ? -smear : smear);
	}

	left = spl0;
	right = spl1;
}

void Diffusor::processBlock(float* left, float* right, int nsamps)
{
	for (int sample = 0; sample < nsamps; ++sample) {

		float spl0 = left[sample];
		float spl1 = right[sample];

		for (int i = 0; i < NUM_ALLPASS; ++i) {
			spl0 = allpassL[i].allPass(spl0, i % 2 == 0 ? smear : -smear);
			spl1 = allpassR[i].allPass(spl1, i % 2 == 0 ? -smear : smear);
		}

		left[sample] = spl0;
		right[sample] = spl1;
	}
}