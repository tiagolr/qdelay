#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"
#include "TimePicker.h"
#include "../dsp/Delay.h"

using namespace globals;
class QDelayAudioProcessorEditor;

class DelayWidget
	: public juce::Component
	, private juce::AudioProcessorValueTreeState::Listener
{
public:
	std::unique_ptr<TimePicker> rateL;
	std::unique_ptr<TimePicker> rateSyncL;
	std::unique_ptr<TimePicker> rateR;
	std::unique_ptr<TimePicker> rateSyncR;

	TextButton modeBtn;
	TextButton syncModeLBtn;
	TextButton syncModeRBtn;
	TextButton linkBtn;

	DelayWidget(QDelayAudioProcessorEditor& e);
	~DelayWidget();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void paint(juce::Graphics& g) override;
	void resized() override;
	void toggleUIComponents();

	void showSyncMenu(bool isLeft);
	void showModeMenu();

private:
	QDelayAudioProcessorEditor& editor;
};