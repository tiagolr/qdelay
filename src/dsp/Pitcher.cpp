#include "Pitcher.h"

void Pitcher::init (bool use_acf, WindowSize size)
{
	windowSize = size;
	int winSize = windowSizes[size];
	buffer.init(winSize * 2);

	acf = use_acf;
	lastHeadSpeed = readHeadSpeed = 1;
	crossFadeSamples = winSize;
	fftmem1.resize(winSize * 2);
	fftmem2.resize(winSize * 2);

	readHead1 = buffer.size*.5f;
	readHead2 = -1.f;
	speed = 1.f;
}

// calculates the ACF and finds its maximum.
float Pitcher::computeMaxACFPosition()
{

	int fftSize = windowSizes[windowSize] * 2;
	float* B1 = fftmem2.data();
	float* B2 = fftmem1.data();
	auto& fft = fftSet.get(windowSize);

	fft.performRealOnlyForwardTransform(B1);
	fft.performRealOnlyForwardTransform(B2);

	// Conjugate B2 (imaginary parts)
	for (int i = 1; i < fftSize; i += 2)
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
	int winSize = windowSizes[windowSize];

	for (int idx = 0; idx < winSize / 4 - 1; idx++)
	{
		float current = B1[idx];
		if (current > cmax)
		{
			cmax = current * 1.05f;
			maxIdx = idx;
		}
	}

	// Quadratic interpolation for sub-sample accuracy
	// Improve resolution and reduce jitter by a quadratic fit of the peak
	float yc = B1[maxIdx];
	float yl = B1[maxIdx - 1];
	float yr = B1[maxIdx + 1];

	float correction = (-2.0f * maxIdx * yc + maxIdx * yl + maxIdx * yr + yl - yr)
					/ (-2.0f * yc + yl + yr) - maxIdx;

    float peakIdx = (float)maxIdx;
	if (std::abs(correction) < 4.0f)
		peakIdx += correction;

	return std::max(peakIdx, 0.f);
}

void Pitcher::setPitchShifterSpeed(float newHeadSpeed)
{
    readHeadSpeed = readHeadSpeed *.999f + .001f * newHeadSpeed;
    if (std::fabs(readHeadSpeed - lastHeadSpeed) > .0001f)
        fader.next(crossFadeSamples / std::max(1.1f, std::fabs(readHeadSpeed)) - 16);

    lastHeadSpeed = readHeadSpeed;
}

void Pitcher::update(float l, float r)
{
    readHead1 -= readHeadSpeed;

    if (fadeCount <= 0)
    {
        fade = false;
        fadeCount = 1;
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

        fadeCount -= 1;

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
            target = crossFadeSamples * 2;
        }
        else  // pitching up
        {
            src = crossFadeSamples;
            crit = readHead1 <= src;
            target = buffer.size - crossFadeSamples;
        }

        if (crit)
        {
            // Determine crossfade offset using autocorrelation
            float cmax_position = 0.f;
            if (acf)
            {
                buffer.copyFromBuffer(fftmem1.data(), src, crossFadeSamples, true);   // left
                buffer.copyFromBuffer(fftmem2.data(), target, crossFadeSamples, false); // right
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