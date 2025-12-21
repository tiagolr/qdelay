// Copyright 2025 tilr

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Globals.h"

QDelayAudioProcessorEditor::QDelayAudioProcessorEditor (QDelayAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
{
    setResizable(false, false);
    setResizeLimits(PLUG_WIDTH, PLUG_HEIGHT, MAX_PLUG_WIDTH, MAX_PLUG_HEIGHT);
    setSize (audioProcessor.plugWidth, audioProcessor.plugHeight);
    setScaleFactor(audioProcessor.scale);

    audioProcessor.addChangeListener(this);

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
    MessageManager::callAsync([this] { repaint(); });
}

//==============================================================================

void QDelayAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour(COLOR_BACKGROUND));
}

void QDelayAudioProcessorEditor::resized()
{
    if (!init) return; // defer resized() call during constructor
    // audioProcessor.plugWidth = getWidth();
    // audioProcessor.plugHeight = getHeight();
}