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
#include "ui/EQWidget.h"
#include "ui/Meter.h"
#include "ui/DistWidget.h"
#include "ui/TapeWidget.h"
#include "dsp/Delay.h"
#include "PresetMgr.h"

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
    void setEQTab(bool feedbackOrInput);
    void showSettings();
    void showRightTabMenu();
    void showPresetsMenu();
    void savePreset();

    QDelayAudioProcessor& audioProcessor;
private:
    bool init = false;

    TextButton logo;
    TextButton settingsBtn;
    TextButton presetBtn;
    TextButton prevPresetBtn;
    TextButton nextPresetBtn;
    TextButton saveBtn;

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

    std::unique_ptr<Rotary> diffAmt;
    std::unique_ptr<Rotary> diffSize;
    std::unique_ptr<Rotary> modDepth;
    std::unique_ptr<Rotary> modRate;
    std::unique_ptr<Rotary> distFeedbk;
    std::unique_ptr<Rotary> distPost;
    std::unique_ptr<Rotary> tapeAmt;
    std::unique_ptr<Rotary> pitchShift;
    std::unique_ptr<Rotary> duckThres;
    std::unique_ptr<Rotary> duckAmt;
    std::unique_ptr<Rotary> duckAtk;
    std::unique_ptr<Rotary> duckRel;
    Slider pitchMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchMixAttachment;

    TextButton rightTabBtn;
    std::unique_ptr<EQWidget> eqInput;
    std::unique_ptr<EQWidget> eqFeedbk;
    std::unique_ptr<DistWidget> distWidget;
    std::unique_ptr<TapeWidget> tapeWidget;
    std::unique_ptr<Meter> meter;

    CustomLookAndFeel* customLookAndFeel = nullptr;
    std::unique_ptr<About> about;
    TooltipWindow tooltipWindow;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QDelayAudioProcessorEditor)
};
