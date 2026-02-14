#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"
#include "Rotary.h"

using namespace globals;
class QDelayAudioProcessorEditor;

class PhaserWidget
	: public juce::Component
	, private juce::AudioProcessorValueTreeState::Listener
{
public:
	TextButton pathBtn;
	std::unique_ptr<Rotary> mix;
	std::unique_ptr<Rotary> stereo;
	std::unique_ptr<Rotary> morph;
	std::unique_ptr<Rotary> res;
	std::unique_ptr<Rotary> center;
	std::unique_ptr<Rotary> rate;
	std::unique_ptr<Rotary> depth;

	PhaserWidget(QDelayAudioProcessorEditor& e);
	~PhaserWidget();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void paint(Graphics& g) override;
	void resized() override;
	void showPathMenu();

private:
	QDelayAudioProcessorEditor& editor;
};