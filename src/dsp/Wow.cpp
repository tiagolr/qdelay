#include "Wow.h"
#include "../PluginProcessor.h"

Wow::Wow(QDelayAudioProcessor& proc)
	: audioProcessor(proc)
{
}

void Wow::prepare (float sampleRate)
{
    srate = sampleRate;
    depthSmooth.setup(0.15f, srate);
    amp = 1000.0f * 1000.0f / srate;
	phase = 0.f;
	ohProc.prepare(srate);
}

void Wow::prepareBlock ()
{
	float amt = audioProcessor.params.getRawParameterValue("tape_amt")->load();
	depth = audioProcessor.params.getRawParameterValue("wow_depth")->load() * amt;
	rate = audioProcessor.params.getRawParameterValue("wow_rate")->load();
	drift = audioProcessor.params.getRawParameterValue("wow_drift")->load();
	var = audioProcessor.params.getRawParameterValue("wow_var")->load();

	auto freqAdjust = rate * (1.0f + std::pow(driftRand.nextFloat(), 1.25f) * drift);
	angleDelta = MathConstants<float>::twoPi * freqAdjust / srate;
	ohProc.prepareBlock(var);
}