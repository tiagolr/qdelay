#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"
#include "Rotary.h"

using namespace globals;
class QDelayAudioProcessorEditor;

class DistWidget
	: public juce::Component
	, private juce::AudioProcessorValueTreeState::Listener
{
public:

	TextButton modeBtn;
	TextButton upsampleBtn;
	TextButton crushBtn;
	std::unique_ptr<Rotary> drive;
	std::unique_ptr<Rotary> trim;
	std::unique_ptr<Rotary> color;
	std::unique_ptr<Rotary> bias;
	std::unique_ptr<Rotary> dynamics;
	std::unique_ptr<Rotary> srate;
	std::unique_ptr<Rotary> bits;

	DistWidget(QDelayAudioProcessorEditor& e);
	~DistWidget();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void paint(Graphics& g) override;
	void resized() override;
	void showModeMenu();

private:
	QDelayAudioProcessorEditor& editor;
};