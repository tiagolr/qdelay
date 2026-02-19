#include "Shifter.h"
#include "../PluginProcessor.h"

Shifter::Shifter (QDelayAudioProcessor& p)
	: audioProcessor(p)
{
}

void Shifter::prepare(float _srate)
{
	srate = _srate;
	clear();
	init_freq_shifter();
}

void Shifter::onSlider()
{
	shift = audioProcessor.params.getRawParameterValue("freq_shift")->load();
    if (!isOn && shift != 0.f) clear();
	isOn = shift != 0.f;
	float mix = audioProcessor.params.getRawParameterValue("pitch_mix")->load();
	drymix = Utils::cosHalfPi()(mix);
    wetmix = Utils::sinHalfPi()(mix);
	init_freq_shifter();
}

void Shifter::clear()
{
	l1.clear();
	l2.clear();
	r1.clear();
	r2.clear();
}


void Shifter::process(float& left, float& right)
{
	t1 += dt1;
  	t2 += dt2;

  	double ct1 = osc_coeff_t1 * cos_t1_1 - cos_t1_2;
  	cos_t1_2 = cos_t1_1;
  	cos_t1_1 = ct1;

  	double ct2 = osc_coeff_t2 * cos_t2_1 - cos_t2_2;
  	cos_t2_2 = cos_t2_1;
  	cos_t2_1 = ct2;

  	double st1 = osc_coeff_t1 * sin_t1_1 - sin_t1_2;
  	sin_t1_2 = sin_t1_1;
  	sin_t1_1 = st1;

  	double st2 = osc_coeff_t2 * sin_t2_1 - sin_t2_2;
  	sin_t2_2 = sin_t2_1;
  	sin_t2_1 = st2;

	double outL = 2 * (l1.quick_ellip(left * ct1) * ct2 + l2.quick_ellip(left * st1) * st2);
	double outR = 2 * (r1.quick_ellip(right * ct1) * ct2 + r2.quick_ellip(right * st1) * st2);

	left = (float)(drymix * left + wetmix * outL);
	right = (float)(drymix * right + wetmix * outR);
}

void Shifter::init_freq_shifter()
{
	double piblock = 200 * juce::MathConstants<double>::pi;
  	dt1 = juce::MathConstants<double>::twoPi * 0.251;  // oscillating at srate / 4
  	dt2 = dt1 + juce::MathConstants<double>::twoPi * shift / srate;
  	if (t1 > piblock) t1 -= piblock;
  	if (t2 > piblock) t2 -= piblock;

  	double w = dt1;
  	osc_coeff_t1 = 2.0 * std::cos(w);
  	t1 += dt1;
  	cos_t1_1 = std::sin(- w + t1);
  	cos_t1_2 = std::sin(- 2.0*w + t1);
  	sin_t1_1 = - std::cos(- w + t1);
  	sin_t1_2 = - std::cos(- 2.0*w + t1);
  	t1 -= dt1;

  	w = dt2;
  	osc_coeff_t2 = 2.0 * std::cos(w);
  	t2 += dt2;
  	cos_t2_1 = std::sin(- w + t2);
  	cos_t2_2 = std::sin(- 2.0 * w + t2);
  	sin_t2_1 = - std::cos(- w + t2);
  	sin_t2_2 = - std::cos(- 2.0 * w + t2);
  	t2 -= dt2;
}