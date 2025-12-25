#pragma once
#include <JuceHeader.h>
#include "../Globals.h"

using namespace globals;
class QDelayAudioProcessorEditor;

class TimePicker
	: public juce::Component
	, juce::AudioProcessorValueTreeState::Listener
{
public:
	int mode = 0; // seconds, straight, tripplet, dotted

	TimePicker(QDelayAudioProcessorEditor& e, String paramId);
	~TimePicker();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	String getSyncText(float val);

	void paint(juce::Graphics& g) override;
	void mouseDown(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseDoubleClick(const juce::MouseEvent& e) override;
	void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

private:
	String paramId;
	float pixels_per_percent{ 300.0f };
	float cur_normed_value{ 0.0f };
	juce::Point<int> last_mouse_position;
	juce::Point<int> start_mouse_pos;
	bool mouse_down = false;
	bool mouse_hover = false;

	QDelayAudioProcessorEditor& editor;
};