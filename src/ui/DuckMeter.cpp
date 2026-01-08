#include "DuckMeter.h"
#include "../PluginProcessor.h"

DuckMeter::DuckMeter(QDelayAudioProcessor& p)
	: audioProcessor(p)
{
    startTimerHz(60);
}

void DuckMeter::timerCallback()
{
    repaint();
}

DuckMeter::~DuckMeter()
{
}

void DuckMeter::paint(juce::Graphics& g) {
	auto bounds = getLocalBounds().toFloat();

	g.setColour(Colour(COLOR_BEVEL).brighter(0.05f));
	g.fillRoundedRectangle(bounds.reduced(0.5f), BEVEL_CORNER);
	g.setColour(Colour(COLOR_BEVEL));
	g.drawRoundedRectangle(bounds.reduced(0.5f), BEVEL_CORNER, 1.f);

	auto barbounds = bounds.reduced(4.f);
	float env = audioProcessor.duckEnv.load();

    if (env > 0.f) {
		auto bar = barbounds.withWidth(barbounds.getWidth() * std::min(1.f, env));
		g.setColour(Colour(COLOR_ACTIVE));
		g.fillRect(bar);
	}
}