/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Globals.h"
#include "ui/UIUtils.h"
#include "ui/CustomLookAndFeel.h"
#include "ui/DelayWidget.h"
#include "ui/DelayView.h"
#include "ui/About.h"
#include "ui/Rotary.h"
#include "dsp/Delay.h"

using namespace globals;

class QDelayAudioProcessorEditor
	: public juce::AudioProcessorEditor
	, private juce::AudioProcessorValueTreeState::Listener
	, public juce::ChangeListener
{
public:

    QDelayAudioProcessorEditor (QDelayAudioProcessor&);
    ~QDelayAudioProcessorEditor() override;

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void toggleUIComponents ();
    void paint (juce::Graphics&) override;
    void resized() override;
    void changeListenerCallback(ChangeBroadcaster* source) override;

    QDelayAudioProcessor& audioProcessor;
private:
    bool init = false;

    TextButton logo;
    std::unique_ptr<DelayView> delayView;
    std::unique_ptr<DelayWidget> delayWidget;
    TextButton mixTabBtn;
    TextButton panTabBtn;
    TextButton patTabBtn;
    std::unique_ptr<Rotary> mix;
    std::unique_ptr<Rotary> feedback;
    std::unique_ptr<Rotary> haasWidth;
    std::unique_ptr<Rotary> pipoWidth;
    std::unique_ptr<Rotary> panDry;
    std::unique_ptr<Rotary> panWet;
    std::unique_ptr<Rotary> stereo;
    std::unique_ptr<Rotary> swing;
    std::unique_ptr<Rotary> feel;
    std::unique_ptr<Rotary> accent;

    CustomLookAndFeel* customLookAndFeel = nullptr;
    std::unique_ptr<About> about;
    TooltipWindow tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QDelayAudioProcessorEditor)
};
