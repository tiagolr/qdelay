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
	clear();
}

float Phaser::getLfoRate(int sync)
{
	if (sync == 0)
		return audioProcessor.params.getRawParameterValue("phaser_rate")->load();

	auto rateSync = audioProcessor.params.getRawParameterValue("phaser_rate_sync")->load();

	auto secondsPerBeat = audioProcessor.secondsPerBeat;
	if (secondsPerBeat == 0.f)
		secondsPerBeat = 0.25f;

	float qn = 1.f;
	if (rateSync == 0) qn = 1.f / 16.f; // 1/64
	if (rateSync == 1) qn = 1.f / 8.f; // 1/32
	if (rateSync == 2) qn = 1.f / 4.f; // 1/16
	if (rateSync == 3) qn = 1.f / 2.f; // 1/8
	if (rateSync == 4) qn = 1.f / 1.f; // 1/4
	if (rateSync == 5) qn = 1.f * 2.f; // 1/2
	if (rateSync == 6) qn = 1.f * 4.f; // 1/1
	if (rateSync == 7) qn = 2.f * 4.f; // 2/1
	if (rateSync == 8) qn = 4.f * 4.f; // 4/1
	if (rateSync == 9) qn = 8.f * 4.f; // 8/1
	if (rateSync == 10) qn = 16.f * 4.f; // 16/1
	if (rateSync == 11) qn = 32.f * 4.f; // 32/1
	if (sync == 2) qn *= 2 / 3.f; // tripplet
	if (sync == 3) qn *= 1.5f; // dotted

	float seconds = (float)(qn * secondsPerBeat);
	return 1.f / seconds;
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
	lfo_stereo = audioProcessor.params.getRawParameterValue("phaser_stereo")->load() * 0.5f;
	lfo_sync = (int)audioProcessor.params.getRawParameterValue("phaser_sync")->load();
	lfo_rate = getLfoRate(lfo_sync);
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

void Phaser::syncToSongTime(float elapsed)
{
	lfoL.syncToSongTime(elapsed);
	lfoR.syncToSongTime(elapsed);
}

void Phaser::process(float& left, float& right)
{
	float leftLFO  = lfoL.tick(0.0f);
	float rightLFO = lfoR.tick(lfo_stereo);
	float freqL = std::clamp(lfo_center.get() * std::powf(2.0f, lfo_depth * leftLFO), 1.f, srate * 0.48f);
	float freqR = std::clamp(lfo_center.get() * std::powf(2.0f, lfo_depth * rightLFO), 1.f, srate * 0.48f);
	lphaser.init(srate, freqL, res);
	rphaser.init(srate, freqR, res);

	float _mix = mix.get();
	left = _mix * lphaser.eval(left) + (1-_mix) * left;
	right = _mix * rphaser.eval(right) + (1-_mix) * right;

	mix.tick();
	lfo_center.tick();
}