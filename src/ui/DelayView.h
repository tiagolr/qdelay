#pragma once

#include <JuceHeader.h>
#include <deque>
#include "../Globals.h"
#include "UIUtils.h"

using namespace globals;
class QDelayAudioProcessorEditor;

class VirtualDelay
{
public:
    struct Tap
    {
        bool empty = false;
        bool dry = false;
        float time;
        float gain;

        Tap withAccent(float accent) const
        {
            Tap t;
            t.empty = empty;
            t.dry = dry;
            t.time = time;
            t.gain = dry ? gain : gain * accent;
            return t;
        }
    };
    float delay = 0.f;
    float feedback = 0.f;

    VirtualDelay(float _delay, float _feedback)
    {
        delay = _delay;
        feedback = _feedback;
    }

    void seed(float time, float gain)
    {
        taps.push_back({ false, true, time, gain });
    }

    void write(Tap tap, bool firstTap = false)
    {
        taps.push_back({ 
            false, 
            false, 
            tap.time + delay, 
            tap.gain * (firstTap ? 1.f : feedback)
        });
        std::sort(taps.begin(), taps.end(), [](const Tap& a, const Tap& b) 
            {
                return a.time < b.time;
            });
    }

    Tap read()
    {
        if (taps.size()) {
            auto e = taps.front();
            taps.pop_front();
            if (e.gain < 0.001) 
                return { true };
            else
                return e;
        }
        return {true};
    }

private:
    std::deque<Tap> taps;
};

class DelayView
	: public juce::Component
	, private juce::AudioProcessorValueTreeState::Listener
{
public:
	DelayView(QDelayAudioProcessorEditor& e);
	~DelayView();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void paint(Graphics& g) override;

private:
	QDelayAudioProcessorEditor& editor;
};