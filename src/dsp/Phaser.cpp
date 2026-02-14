#include "Phaser.h"
#include "../PluginProcessor.h"

Phaser::Phaser (QDelayAudioProcessor& p)
	: audioProcessor(p)
{
}

void Phaser::prepare(float _srate)
{
	srate = _srate;
	lphaser.setLerp(0);
	rphaser.setLerp(0);
	lfoL.prepare(srate);
	lfoR.prepare(srate);
	int lerp_samps = (int)(srate * PHASER_LERP_MILLIS / 1000.0);
	lfo_center.setDuration(lerp_samps);
	lfo_center.set(audioProcessor.params.getRawParameterValue("phaser_center")->load());
	lfo_center.reset();
	mix.setDuration(lerp_samps);
	mix.set(audioProcessor.params.getRawParameterValue("phaser_mix")->load());
	mix.reset();
}

void Phaser::onSlider()
{
	float mx = audioProcessor.params.getRawParameterValue("phaser_mix")->load();
	mix.set(mx);
	isOn = mx > 0.f;
	float morph = audioProcessor.params.getRawParameterValue("phaser_morph")->load();
	res = audioProcessor.params.getRawParameterValue("phaser_res")->load();
	lfo_center.set(audioProcessor.params.getRawParameterValue("phaser_center")->load());
	lfo_depth = audioProcessor.params.getRawParameterValue("phaser_depth")->load() / 12.f;
	lfo_rate = audioProcessor.params.getRawParameterValue("phaser_rate")->load();
	lfo_stereo = audioProcessor.params.getRawParameterValue("phaser_stereo")->load() * 0.5f;
	lphaser.setMorph(morph);
	rphaser.setMorph(morph);
	lfoL.setRate(lfo_rate);
	lfoR.setRate(lfo_rate);
}

void Phaser::clear()
{
	lphaser.reset(0.f);
	rphaser.reset(0.f);
}

void Phaser::resetPhase(float elapsed)
{
	(void)elapsed;
	lfoL.clear();
	lfoR.clear();
}

void Phaser::process(float& left, float& right)
{
	float leftLFO  = lfoL.tick(0.0f);
	float rightLFO = lfoR.tick(lfo_stereo);
	float freqL = std::clamp(lfo_center.get() * pow(2.0f, lfo_depth * leftLFO), 1.f, srate * 0.48f);
	float freqR = std::clamp(lfo_center.get() * pow(2.0f, lfo_depth * rightLFO), 1.f, srate * 0.48f);
	lphaser.init(srate, freqL, res);
	rphaser.init(srate, freqR, res);

	float _mix = mix.get();
	left = _mix * lphaser.eval(left) + (1-_mix) * left;
	right = _mix * rphaser.eval(right) + (1-_mix) * right;

	mix.tick();
	lfo_center.tick();
}