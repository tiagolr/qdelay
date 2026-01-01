#include "Flutter.h"
#include "../PluginProcessor.h"

Flutter::Flutter(QDelayAudioProcessor& proc)
	: audioProcessor(proc)
{
}

void Flutter::prepare (float sampleRate)
{
    srate = sampleRate;

    depthSmooth.setup(0.15f, srate);

    amp1 = -230.0f * 1000.0f / srate;
    amp2 = -80.0f * 1000.0f / srate;
    amp3 = -99.0f * 1000.0f / srate;
	dcOffset = 350.0f * 1000.0f / srate;

	phase1 = 0.f;
	phase2 = 0.f;
	phase3 = 0.f;
}

void Flutter::prepareBlock ()
{
	float amt = audioProcessor.params.getRawParameterValue("tape_amt")->load();
	depth = audioProcessor.params.getRawParameterValue("flutter_depth")->load() * amt;
	rate = audioProcessor.params.getRawParameterValue("flutter_rate")->load();

	angleDelta1 = MathConstants<float>::twoPi * rate / srate;
    angleDelta2 = 2.0f * angleDelta1;
    angleDelta3 = 3.0f * angleDelta1;
}