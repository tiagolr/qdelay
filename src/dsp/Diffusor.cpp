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
	setSize(std::pow(sizenorm, 4.f), false);

	for (int i = 0; i < NUM_ALLPASS; i++)
	{
		allpass[i].clear();
		delays[i].clear();
	}
}

void Diffusor::prepare(float _srate)
{
	srateFactor = _srate / REF_SRATE;

	for (int i = 0; i < NUM_ALLPASS; ++i)
	{
		modPhasesInc[i] = (rand() / (float)RAND_MAX) * MOD_MAX_RATE / srateFactor / _srate;
		allpass[i].init(_srate);
	}

	clear();
}

void Diffusor::setSize(float sizenorm_, bool smooth)
{
	sizenorm_ = std::pow(sizenorm_, 0.25f);
	if (sizenorm == sizenorm_ && smooth)
		return;

	sizenorm = sizenorm_;
	t60 = sizenorm * 0.95f;

	float size = 0.25f + sizenorm * (1.f - 0.25f); // 0.25 ... 1
	smear = 0.3f + sizenorm * (0.6f - 0.3f); // 0.3 0.6
	const float base = 100.f * (1 + size);
	const float scale = 1.5f;

	auto scaleSize = [](float val, float index, float scale)
		{
			return val * pow(scale, (float)index);
		};

	// the scaling used is and exponential with base 1.5 and exponent 3.2 3.0 2.8 etc..
	for (int i = 0; i < NUM_ALLPASS; ++i)
	{
		int apsize = getNextPrime((int)(scaleSize(std::floor(base), 3.2f - i * 0.2f, scale) * srateFactor));
		allpass[i].setSize(apsize, smooth);
	}
}

void Diffusor::processBlock(float* left, float* right, int nsamps, float drymix, float wetmix)
{
	constexpr float delay_sizes[8] = { 942, 1800, 1500, 1233, 1923, 1223, 809, 601 };
	const float delay_factor = srateFactor * 3.f * (0.5f + 0.5f * sizenorm);

	std::array<float, NUM_ALLPASS> outs{};

	for (int sample = 0; sample < nsamps; ++sample) {

		float spl0 = left[sample];
		float spl1 = right[sample];

		for (int i = 0; i < NUM_ALLPASS; ++i) {
			float mod = triangle(modPhases[i]) * MOD_MAX_DEPTH;
			modPhases[i] += modPhasesInc[i];
			if (modPhases[i] >= 1.f)
				modPhases[i] -= 1.f;

			float feedback = fb[NUM_ALLPASS - i - 1];
			outs[i] = allpass[i].allPass((i % 2 == 0 ? spl0 : spl1) + feedback * t60, smear, mod);
		}

		outs = hadamard_8x8(outs);

		// out taps
		spl0 = 0;
		spl1 = 0;

		for (int i = 0; i < NUM_ALLPASS; ++i) {
			if (i % 2 == 0) spl0 += outs[i];
			else spl1 += outs[i];
		}

		spl0 *= 0.25f;
		spl1 *= 0.25f;

		// feedback
		for (int i = 0; i < NUM_ALLPASS; ++i) {
			delays[i].write(outs[i]);
			fb[i] = delays[i].read(delay_sizes[i] * delay_factor);
		}

		left[sample] = spl0 * wetmix + left[sample] * drymix;
		right[sample] = spl1 * wetmix + right[sample] * drymix;
	}
}

float Diffusor::triangle(float phase)
{
	return 2.f - std::abs(2.0f * (phase - std::floor(phase)) - 1.0f) - 1.0f;
}

std::array<float, 8> Diffusor::hadamard_8x8(std::array<float, 8> x)
{
	constexpr float norm = 0.3535533905932737622f; // 1 / sqrt(8)

	float a0 = x[0] + x[1], a1 = x[0] - x[1];
	float a2 = x[2] + x[3], a3 = x[2] - x[3];
	float a4 = x[4] + x[5], a5 = x[4] - x[5];
	float a6 = x[6] + x[7], a7 = x[6] - x[7];

	float b0 = a0 + a2, b1 = a1 + a3;
	float b2 = a0 - a2, b3 = a1 - a3;
	float b4 = a4 + a6, b5 = a5 + a7;
	float b6 = a4 - a6, b7 = a5 - a7;

	return {
		(b0 + b4) * norm, (b1 + b5) * norm,
		(b2 + b6) * norm, (b3 + b7) * norm,
		(b0 - b4) * norm, (b1 - b5) * norm,
		(b2 - b6) * norm, (b3 - b7) * norm
	};
}