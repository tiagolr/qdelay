#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"

using namespace globals;
class QDelayAudioProcessorEditor;

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