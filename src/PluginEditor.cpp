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
    logo.onClick = [this]
        {
            about->setVisible(true);
        };

    addAndMakeVisible(settingsBtn);
    settingsBtn.setAlpha(0.f);
    settingsBtn.setBounds(col + 100 - 5, row, 25, 25);
    settingsBtn.onClick = [this]
        {
            showSettings();
        };

    row += HEADER_HEIGHT + 10;

    delayView = std::make_unique<DelayView>(*this);
    addAndMakeVisible(delayView.get());
    delayView->setBounds(col, row, KNOB_WIDTH * 3, KNOB_HEIGHT);

    row += KNOB_HEIGHT;

    delayWidget = std::make_unique<DelayWidget>(*this);
    addAndMakeVisible(delayWidget.get());
    delayWidget->setBounds(col, row+5, KNOB_WIDTH * 3, KNOB_HEIGHT);

    row += KNOB_HEIGHT + 15;

    addAndMakeVisible(mixTabBtn);
    mixTabBtn.setComponentID("button-noborder");
    mixTabBtn.setButtonText("Mix");
    mixTabBtn.setBounds(col, row, KNOB_WIDTH, VSEPARATOR);
    mixTabBtn.onClick = [this]
        {
            audioProcessor.delayTab = 0;
            toggleUIComponents();
        };

    addAndMakeVisible(patTabBtn);
    patTabBtn.setComponentID("button-noborder");
    patTabBtn.setButtonText("Pattern");
    patTabBtn.setBounds(col + KNOB_WIDTH, row, KNOB_WIDTH, VSEPARATOR);
    patTabBtn.onClick = [this]
        {
            audioProcessor.delayTab = 2;
            toggleUIComponents();
        };

    addAndMakeVisible(panTabBtn);
    panTabBtn.setComponentID("button-noborder");
    panTabBtn.setButtonText("Pan");
    panTabBtn.setBounds(col + KNOB_WIDTH*2, row, KNOB_WIDTH, VSEPARATOR);
    panTabBtn.onClick = [this]
        {
            audioProcessor.delayTab = 1;
            toggleUIComponents();
        };

    row += VSEPARATOR + 5;

    mix = std::make_unique<Rotary>(audioProcessor, "mix", "Mix", Rotary::percx100, true);
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

    panDry = std::make_unique<Rotary>(audioProcessor, "pan_dry", "Dry", Rotary::pan, true);
    addChildComponent(panDry.get());
    panDry->setBounds(mix->getBounds());

    panWet = std::make_unique<Rotary>(audioProcessor, "pan_wet", "Wet", Rotary::pan, true);
    addChildComponent(panWet.get());
    panWet->setBounds(feedback->getBounds());

    stereo = std::make_unique<Rotary>(audioProcessor, "stereo", "Stereo", Rotary::percx100, true);
    addChildComponent(stereo.get());
    stereo->setBounds(haasWidth->getBounds());

    swing = std::make_unique<Rotary>(audioProcessor, "swing", "Swing", Rotary::percx100, true);
    addChildComponent(swing.get());
    swing->setBounds(mix->getBounds());

    feel = std::make_unique<Rotary>(audioProcessor, "feel", "Feel", Rotary::percx100, true);
    addChildComponent(feel.get());
    feel->setBounds(feedback->getBounds());

    accent = std::make_unique<Rotary>(audioProcessor, "accent", "Accent", Rotary::percx100, true);
    addChildComponent(accent.get());
    accent->setBounds(haasWidth->getBounds());

    // MID SECTION
    row = PLUG_PADDING;
    col = PLUG_PADDING + KNOB_WIDTH * 3 + HSEPARATOR;

    row += HEADER_HEIGHT + 10;

    diffAmt = std::make_unique<Rotary>(audioProcessor, "diff_amt", "Amount", Rotary::percx100);
    addAndMakeVisible(diffAmt.get());
    diffAmt->setBounds(col, row, KNOB_WIDTH, KNOB_HEIGHT);

    diffSize = std::make_unique<Rotary>(audioProcessor, "diff_size", "Size", Rotary::percx100);
    addAndMakeVisible(diffSize.get());
    diffSize->setBounds(col, row + KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);

    modDepth = std::make_unique<Rotary>(audioProcessor, "mod_depth", "Depth", Rotary::percx100);
    addAndMakeVisible(modDepth.get());
    modDepth->setBounds(col+KNOB_WIDTH, row, KNOB_WIDTH, KNOB_HEIGHT);

    modRate = std::make_unique<Rotary>(audioProcessor, "mod_rate", "Rate", Rotary::hz1f);
    addAndMakeVisible(modRate.get());
    modRate->setBounds(col+KNOB_WIDTH, row+KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);

    distFeedbk = std::make_unique<Rotary>(audioProcessor, "dist_pre", "Pre", Rotary::percx100);
    addAndMakeVisible(distFeedbk.get());
    distFeedbk->setBounds(col+KNOB_WIDTH*2, row, KNOB_WIDTH, KNOB_HEIGHT);

    distPost = std::make_unique<Rotary>(audioProcessor, "dist_post", "Post", Rotary::percx100);
    addAndMakeVisible(distPost.get());
    distPost->setBounds(col+KNOB_WIDTH*2, row+KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);

    tapeAmt = std::make_unique<Rotary>(audioProcessor, "tape_amt", "Tape", Rotary::percx100);
    addAndMakeVisible(tapeAmt.get());
    tapeAmt->setBounds(col + KNOB_WIDTH * 3, row, KNOB_WIDTH, KNOB_HEIGHT);

    pitchShift = std::make_unique<Rotary>(audioProcessor, "pitch_shift", "Pitch", Rotary::pitchSemis, true);
    addAndMakeVisible(pitchShift.get());
    pitchShift->setBounds(col + KNOB_WIDTH * 3, row + KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);

    duckThres = std::make_unique<Rotary>(audioProcessor, "duck_thres", "Thresh", Rotary::gainTodB1fInv);
    addAndMakeVisible(duckThres.get());
    duckThres->setBounds(col, row + KNOB_HEIGHT * 2 + 20 + VSEPARATOR, KNOB_WIDTH, KNOB_HEIGHT);

    duckAmt = std::make_unique<Rotary>(audioProcessor, "duck_amt", "Amount", Rotary::gainTodB1fInv);
    addAndMakeVisible(duckAmt.get());
    duckAmt->setBounds(col+KNOB_WIDTH, row + KNOB_HEIGHT * 2 + 20 + VSEPARATOR, KNOB_WIDTH, KNOB_HEIGHT);

    duckAtk = std::make_unique<Rotary>(audioProcessor, "duck_atk", "Atk", Rotary::kMillis);
    addAndMakeVisible(duckAtk.get());
    duckAtk->setBounds(col + KNOB_WIDTH * 2, row + KNOB_HEIGHT * 2 + 20 + VSEPARATOR, KNOB_WIDTH, KNOB_HEIGHT);

    duckRel = std::make_unique<Rotary>(audioProcessor, "duck_rel", "Rel", Rotary::kMillis);
    addAndMakeVisible(duckRel.get());
    duckRel->setBounds(col + KNOB_WIDTH * 3, row + KNOB_HEIGHT * 2 + 20 + VSEPARATOR, KNOB_WIDTH, KNOB_HEIGHT);

    addAndMakeVisible(pitchMix);
    pitchMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.params, "pitch_mix", pitchMix);
    pitchMix.setComponentID("pitch_mix");
    pitchMix.setSliderStyle(Slider::LinearBar);
    pitchMix.setTooltip("Set the pitch shift mix amount");
    pitchMix.setBounds(col + KNOB_WIDTH * 3 + (KNOB_WIDTH - 60) / 2, row + KNOB_HEIGHT * 2 + 15 - 1, 60, 22);
    pitchMix.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    pitchMix.setTextBoxStyle(Slider::NoTextBox, true, 10, 10);

    // RIGHT SECTION
    row = PLUG_PADDING;
    col = PLUG_PADDING + KNOB_WIDTH * 7 + HSEPARATOR * 2;

    eqInput = std::make_unique<EQWidget>(*this, SVF::ParamEQ);
    addChildComponent(eqInput.get());
    eqInput->setBounds(col, row, KNOB_WIDTH * 3, getHeight() - row - PLUG_PADDING);

    eqFeedbk = std::make_unique<EQWidget>(*this, SVF::DecayEQ);
    addChildComponent(eqFeedbk.get());
    eqFeedbk->setBounds(col, row, KNOB_WIDTH * 3, getHeight() - row - PLUG_PADDING);

    meter = std::make_unique<Meter>(audioProcessor);
    addAndMakeVisible(meter.get());
    meter->setBounds(Rectangle<int>(0, row, METER_WIDTH, KNOB_HEIGHT * 3 + VSEPARATOR + 30 + HEADER_HEIGHT)
        .withRightX(getWidth() - PLUG_PADDING));

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
    panDry->setVisible(audioProcessor.delayTab == 1);
    panWet->setVisible(audioProcessor.delayTab == 1);
    stereo->setVisible(audioProcessor.delayTab == 1);
    swing->setVisible(audioProcessor.delayTab == 2);
    feel->setVisible(audioProcessor.delayTab == 2);
    accent->setVisible(audioProcessor.delayTab == 2);

    eqInput->setVisible(audioProcessor.eqTab == 0 && audioProcessor.rightTab == 0);
    eqFeedbk->setVisible(audioProcessor.eqTab == 1 && audioProcessor.rightTab == 0);

    MessageManager::callAsync([this] { repaint(); });
}

//==============================================================================

void QDelayAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour(COLOR_BACKGROUND));
    g.setFont(FontOptions(26.f));
    g.setColour(Colours::white);
    g.drawText("QDELAY", logo.getBounds().expanded(0, 10), Justification::centredLeft);
    UIUtils::drawGear(g, settingsBtn.getBounds(), 9, 6, Colours::white, Colour(COLOR_BACKGROUND));

    g.setColour(Colour(COLOR_NEUTRAL));
    g.setFont(FontOptions(16.f));
    g.drawText("DIFF", diffAmt->getX(), diffAmt->getY() - 16 - 10, KNOB_WIDTH, 16, Justification::centred);
    g.drawText("MOD", modDepth->getX(), modDepth->getY() - 16 - 10, KNOB_WIDTH, 16, Justification::centred);
    g.drawText("SAT", distFeedbk->getX(), distFeedbk->getY() - 16 - 10, KNOB_WIDTH, 16, Justification::centred);
    g.drawText("OTHER", tapeAmt->getX(), tapeAmt->getY() - 16 - 10, KNOB_WIDTH, 16, Justification::centred);
    g.drawText("DUCK", duckThres->getX(), duckThres->getY() - 10 - HSEPARATOR, KNOB_WIDTH, HSEPARATOR, Justification::centred);
}

void QDelayAudioProcessorEditor::resized()
{
    if (!init) return; // defer resized() call during constructor
}

void QDelayAudioProcessorEditor::setEQTab(bool feedbackOrInput)
{
    audioProcessor.eqTab = (int)feedbackOrInput;
    toggleUIComponents();
}

void QDelayAudioProcessorEditor::showSettings()
{
    int pitchMode = (int)audioProcessor.params.getRawParameterValue("pitch_mode")->load();

    PopupMenu pitchMenu;
    pitchMenu.addItem(80, "Drums", true, pitchMode == 0);
    pitchMenu.addItem(81, "General", true, pitchMode == 1);
    pitchMenu.addItem(82, "Audio", true, pitchMode == 2);

    PopupMenu menu;
    menu.addSubMenu("Pitch Shifter", pitchMenu);
    menu.addSeparator();
    menu.addItem(9999, "About");

    auto menuPos = localPointToGlobal(settingsBtn.getBounds().getBottomLeft());
    menu.showMenuAsync(PopupMenu::Options()
        .withTargetComponent(*this)
        .withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
        [this](int result) 
        {
            if (result == 0) return;
            else if (result == 9999) about->setVisible(true);
            else if (result >= 80 && result <= 82) 
            {
                auto param = audioProcessor.params.getParameter("pitch_mode");
                param->setValueNotifyingHost(param->convertTo0to1(float(result - 80)));
            }
        }
    );
}