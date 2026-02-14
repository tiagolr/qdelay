#include "PhaserFilter.h"

void PhaserFilter::init(float srate, float freq, float q)
{
    g.set(getCoeff(freq, srate));
    k = q;
    float gg = g.get();
    remove_lows_stage.coeff = juce::jmin(gg * kClearRatio, 0.9f);
    remove_highs_stage.coeff = gg * (1.0f / kClearRatio);
    for (int i = 0; i < kMaxStages; ++i) {
        stages[i].coeff = gg;
    }
}

float PhaserFilter::eval(float sample)
{
    float peak1 = std::clamp(1.0f - 2.0f * morph, 0.0f, 1.0f);
    float peak5 = std::clamp(2.0f * morph - 1.0f, 0.0f, 1.0f);
    float peak3 = -peak1 - peak5 + 1.0f;
    int invert = 1; // disabled

    float lows = remove_lows_stage.eval(allpass_output);
    float highs = remove_highs_stage.eval(lows);
    float state = k.get() * (lows - highs);

    float input = sample + invert * state;
    float output;

    for (int i = 0; i < kPeakStage; ++i) {
        output = stages[i].eval(input);
        input = input + output * -2.0f;
    }

    float peak1out = input;

    for (int i = kPeakStage; i < 2 * kPeakStage; ++i) {
        output = stages[i].eval(input);
        input = input + output * -2.0f;
    }

    float peak3out = input;

    for (int i = 2 * kPeakStage; i < 3 * kPeakStage; ++i) {
        output = stages[i].eval(input);
        input = input + output * -2.0f;
    }

    float peak5out = input;
    float peak13out = (peak1 * peak1out) + peak3 * peak3out;
    allpass_output = peak13out + peak5 * peak5out;

    return (sample + invert * allpass_output) * 0.5f;
}

void PhaserFilter::reset(float sample)
{
    remove_highs_stage.reset(sample);
    remove_lows_stage.reset(sample);
    for (int i = 0; i < kMaxStages; ++i) {
        stages[i].reset(sample);
    }
}

void PhaserFilter::tick()
{
    g.tick();
    k.tick();
}

void PhaserFilter::setLerp(int duration)
{
    g.setDuration(duration);
    k.setDuration(duration);
}