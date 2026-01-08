#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

using namespace globals;
class QDelayAudioProcessor;

class DuckMeter : public juce::Component, private juce::Timer
{
public:
    DuckMeter(QDelayAudioProcessor& p);
    ~DuckMeter() override;
    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    QDelayAudioProcessor& audioProcessor;
};