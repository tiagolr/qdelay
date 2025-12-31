#include "Pitcher.h"

void Pitcher::init (WindowMode _mode)
{
	mode = _mode;
	int winSize = windowSizes[_mode];
	buffer.init(winSize * 2);

	acf = acfFlags[_mode];
	lastHeadSpeed = readHeadSpeed = 1.f;
	crossFadeSamples = winSize / 2;
	fftmem1.resize(winSize * 2, 0.f);
	fftmem2.resize(winSize * 2, 0.f);

	readHead1 = buffer.size * .5f;
	readHead2 = -1.f;
}

// calculates the ACF and finds its maximum.
float Pitcher::computeMaxACFPosition()
{
    int winsize = windowSizes[mode];
	int fftSize = winsize * 2;
	float* B1 = fftmem2.data();
	float* B2 = fftmem1.data();
	auto& fft = fftSet.get(mode);

	fft.performRealOnlyForwardTransform(B1);
	fft.performRealOnlyForwardTransform(B2);

	// Conjugate B2 (imaginary parts)
    for (int i = 3; i < fftSize; i += 2)  // imaginary parts start at index 3
        B2[i] = -B2[i];

	// Element-wise complex multiplication (frequency domain convolution)
	for (int i = 0; i < fftSize; i += 2)
	{
		float a_real = B1[i];
		float a_imag = B1[i + 1];
		float b_real = B2[i];
		float b_imag = B2[i + 1];

		B1[i]     = a_real * b_real - a_imag * b_imag;
		B1[i + 1] = a_real * b_imag + a_imag * b_real;
	}

	// Inverse FFT to get autocorrelation
	fft.performRealOnlyInverseTransform(B1);

	// Find maximum in first quarter of the buffer
	int maxIdx = 0;
	float cmax = -1e7f;
	int winSize = windowSizes[mode];

	for (int idx = 0; idx < winSize / 4 - 1; idx++)
	{
		float current = B1[idx];
		if (current > cmax)
		{
			cmax = current * 1.05f;
			maxIdx = idx;
		}
	}

	// Improve resolution and reduce jitter by a quadratic fit of the peak
	float yc = B1[maxIdx];
	float yl = B1[std::max(maxIdx - 1, 0)];
	float yr = B1[std::min(maxIdx + 1, winSize / 4 - 2)];
    float denom = -2.f * yc + yl + yr;
    float correction = (denom == 0.f) ? 0 : (yl - yr) / denom;

    float peakIdx = (float)maxIdx;
	if (std::abs(correction) < 4.0f)
		peakIdx += correction;

	return std::max(peakIdx, 0.f);
}

void Pitcher::setSpeed(float newHeadSpeed)
{
    readHeadSpeed = readHeadSpeed * .999f + .001f * newHeadSpeed;

    if (std::fabs(readHeadSpeed - lastHeadSpeed) > .0001f)
        fader.resize(crossFadeSamples / std::max(1.1f, std::fabs(readHeadSpeed)) - 16);

    lastHeadSpeed = readHeadSpeed;
}

float Pitcher::getSpeedFromSemis(float semis)
{
    return std::pow(2.f, semis / 12.f) - 1.0f;
}

void Pitcher::update(float l, float r)
{
    readHead1 -= readHeadSpeed;

    if (fader.count <= 0.f)
    {
        fade = false;
        fader.count = 1.f;
        readHead1 = readHead2;
    }

    buffer.write(l, r);
    buffer.read(readHead1);
    float L1 = buffer.outL;
    float R1 = buffer.outR;

    if (fade)  // Crossfade active
    {
        buffer.read(readHead2);
        readHead2 -= readHeadSpeed;
        float L2 = buffer.outL;
        float R2 = buffer.outR;

        fader.eval();

        outL = L1 * fader.w + L2 * (1.0f - fader.w);
        outR = R1 * fader.w + R2 * (1.0f - fader.w);
    }
    else
    {
        int src, target;
        bool crit;

        if (readHeadSpeed <= 0.0f)  // pitching down
        {
            src = buffer.size - crossFadeSamples;
            crit = readHead1 > src;
            target = crossFadeSamples + crossFadeSamples;
        }
        else  // pitching up
        {
            src = crossFadeSamples;
            crit = readHead1 <= src;
            target = buffer.size - crossFadeSamples;
        }

        if (crit)
        {
            // We're over the crossFadeSample boundary. Time to quickly initialize the crossfade.
            // We determine the phase shift between the cross fade sections using an autocorrelation between
            // them. We then jump into the buffer with an offset that corresponds to the peak in the autocorrelation
            // function.
            float cmax_position = 0.f;
            if (acf)
            {
                std::fill(fftmem1.begin(), fftmem1.end(), 0.f);
                std::fill(fftmem2.begin(), fftmem2.end(), 0.f);
                buffer.copyFromBuffer(fftmem1.data(), src, crossFadeSamples);   // left
                buffer.copyFromBuffer(fftmem2.data(), target, crossFadeSamples); // right
                cmax_position = computeMaxACFPosition();
            }

            fader.prepare(std::floor(crossFadeSamples / std::max(1.1f, std::abs(readHeadSpeed))));
            readHead2 = target - cmax_position;
            fade = true;
        }

        // No fade, just normal output
        outL = L1;
        outR = R1;
    }
}