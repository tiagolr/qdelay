// Copyright 2025 tilr

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Globals.h"

QDelayAudioProcessorEditor::QDelayAudioProcessorEditor (QDelayAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
{
    setResizable(false, false);
    setSize (PLUG_WIDTH, PLUG_HEIGHT);
    setScaleFactor(audioProcessor.scale);

    audioProcessor.addChangeListener(this);
    audioProcessor.params.addParameterListener("mode", this);

    // LEFT SECTION
    int col = PLUG_PADDING;
    int row = PLUG_PADDING;

    addAndMakeVisible(logo);
    logo.setAlpha(0.f);
    logo.setBounds(col, row, 100, HEADER_HEIGHT);

    row += HEADER_HEIGHT + 10;

    delayView = std::make_unique<DelayView>(*this);
    addAndMakeVisible(delayView.get());
    delayView->setBounds(col, row, KNOB_WIDTH * 3, KNOB_HEIGHT);

    row += KNOB_HEIGHT;

    delayWidget = std::make_unique<DelayWidget>(*this);
    addAndMakeVisible(delayWidget.get());
    delayWidget->setBounds(col, row+5, KNOB_WIDTH * 3, KNOB_HEIGHT);

    row += KNOB_HEIGHT + 10;

    addAndMakeVisible(mixTabBtn);
    mixTabBtn.setComponentID("button-noborder");
    mixTabBtn.setButtonText("Mix");
    mixTabBtn.setBounds(col, row, KNOB_WIDTH, VSEPARATOR);
    mixTabBtn.onClick = [this]
        {
            audioProcessor.delayTab = 0;
            toggleUIComponents();
        };

    addAndMakeVisible(panTabBtn);
    panTabBtn.setComponentID("button-noborder");
    panTabBtn.setButtonText("Pan");
    panTabBtn.setBounds(col+KNOB_WIDTH, row, KNOB_WIDTH, VSEPARATOR);
    panTabBtn.onClick = [this]
        {
            audioProcessor.delayTab = 1;
            toggleUIComponents();
        };

    addAndMakeVisible(patTabBtn);
    patTabBtn.setComponentID("button-noborder");
    patTabBtn.setButtonText("Pattern");
    patTabBtn.setBounds(col + KNOB_WIDTH*2, row, KNOB_WIDTH, VSEPARATOR);
    patTabBtn.onClick = [this]
        {
            audioProcessor.delayTab = 2;
            toggleUIComponents();
        };

    row += VSEPARATOR + 10;

    mix = std::make_unique<Rotary>(audioProcessor, "mix", "Mix", Rotary::percx100);
    addChildComponent(mix.get());
    mix->setBounds(col, row, KNOB_WIDTH, KNOB_HEIGHT);

    feedback = std::make_unique<Rotary>(audioProcessor, "feedback", "Feedbk", Rotary::percx100);
    addChildComponent(feedback.get());
    feedback->setBounds(col+KNOB_WIDTH, row, KNOB_WIDTH, KNOB_HEIGHT);

    haasWidth = std::make_unique<Rotary>(audioProcessor, "haas_width", "Width", Rotary::haasWidth, true);
    addChildComponent(haasWidth.get());
    haasWidth->setBounds(col + KNOB_WIDTH*2, row, KNOB_WIDTH, KNOB_HEIGHT);

    pipoWidth = std::make_unique<Rotary>(audioProcessor, "pipo_width", "Width", Rotary::percx100, true);
    addChildComponent(pipoWidth.get());
    pipoWidth->setBounds(col + KNOB_WIDTH*2, row, KNOB_WIDTH, KNOB_HEIGHT);

    // ABOUT
    about = std::make_unique<About>();
    addChildComponent(*about);
    about->setBounds(getBounds());

    customLookAndFeel = new CustomLookAndFeel();
    setLookAndFeel(customLookAndFeel);

    init = true;
    resized();
    toggleUIComponents();
}

QDelayAudioProcessorEditor::~QDelayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    delete customLookAndFeel;
    audioProcessor.removeChangeListener(this);
    audioProcessor.params.removeParameterListener("mode", this);
}

void QDelayAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    (void)source;
    MessageManager::callAsync([this] { toggleUIComponents(); });
}

void QDelayAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float newValue)
{
    (void)parameterID;
    (void)newValue;
    MessageManager::callAsync([this]() { toggleUIComponents(); });
};

void QDelayAudioProcessorEditor::toggleUIComponents()
{
    mixTabBtn.setToggleState(audioProcessor.delayTab == 0, dontSendNotification);
    panTabBtn.setToggleState(audioProcessor.delayTab == 1, dontSendNotification);
    patTabBtn.setToggleState(audioProcessor.delayTab == 2, dontSendNotification);

    auto mode = (Delay::DelayMode)audioProcessor.params.getRawParameterValue("mode")->load();
    mix->setVisible(audioProcessor.delayTab == 0);
    feedback->setVisible(audioProcessor.delayTab == 0);
    haasWidth->setVisible(audioProcessor.delayTab == 0 && mode != Delay::PingPong);
    pipoWidth->setVisible(audioProcessor.delayTab == 0 && mode == Delay::PingPong);

    MessageManager::callAsync([this] { repaint(); });
}

//==============================================================================

void QDelayAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour(COLOR_BACKGROUND));
    g.setFont(FontOptions(26.f));
    g.setColour(Colours::white);
    g.drawText("QDELAY", logo.getBounds().expanded(0, 10), Justification::centredLeft);
}

void QDelayAudioProcessorEditor::resized()
{
    if (!init) return; // defer resized() call during constructor
}