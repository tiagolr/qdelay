#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"
#include "Rotary.h"

using namespace globals;
class QDelayAudioProcessorEditor;

class TapeWidget
	: public juce::Component
	, private juce::AudioProcessorValueTreeState::Listener
{
public:

	std::unique_ptr<Rotary> fdepth;
	std::unique_ptr<Rotary> frate;

	std::unique_ptr<Rotary> wdepth;
	std::unique_ptr<Rotary> wrate;
	std::unique_ptr<Rotary> wvar;
	std::unique_ptr<Rotary> wdrift;

	TapeWidget(QDelayAudioProcessorEditor& e);
	~TapeWidget();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void paint(Graphics& g) override;
	void resized() override;

private:
	QDelayAudioProcessorEditor& editor;
};