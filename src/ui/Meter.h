#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"

using namespace globals;
class QDelayAudioProcessor;

class Meter : public juce::Component, private juce::Timer
{
public:
    Meter(QDelayAudioProcessor& p, int _source);
    ~Meter() override;
    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    int source = 0; // Master, Layer1, Layer2
    QDelayAudioProcessor& audioProcessor;
    float zeroMeter = 0.0f;
};