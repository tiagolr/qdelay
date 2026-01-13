#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"
#include "Rotary.h"

using namespace globals;
class QDelayAudioProcessorEditor;

class LofiWidget
	: public juce::Component
	, private juce::AudioProcessorValueTreeState::Listener
{
public:

	TextButton upsampleBtn;
	TextButton pathBtn;
	std::unique_ptr<Rotary> srate;
	std::unique_ptr<Rotary> bits;

	LofiWidget(QDelayAudioProcessorEditor& e);
	~LofiWidget();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void paint(Graphics& g) override;
	void resized() override;
	void showPathMenu();

private:
	QDelayAudioProcessorEditor& editor;
};