#include "TapeWidget.h"
#include "../PluginEditor.h"

TapeWidget::TapeWidget(QDelayAudioProcessorEditor& e)
	: editor(e)
{
	frate = std::make_unique<Rotary>(editor.audioProcessor, "flutter_rate", "Rate", Rotary::hz1f);
	addAndMakeVisible(frate.get());

	fdepth = std::make_unique<Rotary>(editor.audioProcessor, "flutter_depth", "Depth", Rotary::percx100);
	addAndMakeVisible(fdepth.get());

	wrate = std::make_unique<Rotary>(editor.audioProcessor, "wow_rate", "Rate", Rotary::hz1f);
	addAndMakeVisible(wrate.get());

	wdepth = std::make_unique<Rotary>(editor.audioProcessor, "wow_depth", "Depth", Rotary::percx100);
	addAndMakeVisible(wdepth.get());

	wvar = std::make_unique<Rotary>(editor.audioProcessor, "wow_var", "Var", Rotary::percx100);
	addAndMakeVisible(wvar.get());

	wdrift = std::make_unique<Rotary>(editor.audioProcessor, "wow_drift", "Drift", Rotary::percx100);
	addAndMakeVisible(wdrift.get());
}

TapeWidget::~TapeWidget()
{
}

void TapeWidget::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
	MessageManager::callAsync([this] { repaint(); });
}

void TapeWidget::paint(Graphics& g)
{
	g.setFont(FontOptions(16.f));
	g.setColour(Colour(COLOR_NEUTRAL));
	g.drawText("FLUTTER", fdepth->getBounds().translated(-KNOB_WIDTH, 0), Justification::centred);
	g.drawText("WOW", wdepth->getBounds().translated(-KNOB_WIDTH, 0), Justification::centred);
}

void TapeWidget::resized()
{
	auto b = getLocalBounds();

	fdepth->setBounds(b.getX() + KNOB_WIDTH, b.getY(), KNOB_WIDTH, KNOB_HEIGHT);
	frate->setBounds(b.getX() + KNOB_WIDTH * 2, b.getY(), KNOB_WIDTH, KNOB_HEIGHT);
	wdepth->setBounds(b.getX() + KNOB_WIDTH, b.getY() + KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);
	wrate->setBounds(b.getX() + KNOB_WIDTH * 2, b.getY() + KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);
	wvar->setBounds(b.getX() + KNOB_WIDTH, b.getY() + KNOB_HEIGHT * 2, KNOB_WIDTH, KNOB_HEIGHT);
	wdrift->setBounds(b.getX() + KNOB_WIDTH * 2, b.getY() + KNOB_HEIGHT * 2, KNOB_WIDTH, KNOB_HEIGHT);
}