#include "Meter.h"
#include "../PluginProcessor.h"

float gainToScale(float g)
{
    return (std::log10(std::max(g, 0.001f)));
}

Meter::Meter(QDelayAudioProcessor& p, int _source)
	: audioProcessor(p)
	, source(_source)
{
    zeroMeter = gainToScale(1.0f);

    startTimerHz(60);
}

void Meter::timerCallback()
{
    repaint();
}

Meter::~Meter()
{
}

void Meter::paint(juce::Graphics& g) {
	auto bounds = getLocalBounds().toFloat();
	UIUtils::drawBevel(g, bounds.reduced(0.5f), 6.f, Colour(COLOR_BEVEL));

	auto barbounds = bounds
		.withTrimmedBottom(6.f)
		.withWidth(6.f);

	float rmsLeft = audioProcessor.rmsLeft.load();
	float rmsRight = audioProcessor.rmsRight.load();
    rmsLeft = gainToScale(rmsLeft);
    rmsRight = gainToScale(rmsRight);

	//g.setColour(COLOR_HIGHLIGHT_MID());
    //g.drawVerticalLine((int)(barbounds.getX() + barbounds.getWidth() * zeroMeter),
	//	(float)bounds.getY() + 3, (float)bounds.getBottom() - 3);

	g.saveState();
	g.reduceClipRegion(bounds.reduced(3.f, 0.f).toNearestInt());

    if (rmsLeft) {
		g.setColour(Colour(COLOR_ACTIVE));
		auto lbar = barbounds
			.withHeight(barbounds.getHeight() * std::min(zeroMeter, rmsLeft))
			.withY(bounds.getY() + bounds.getHeight() / 4 - barbounds.getHeight() / 2 + 1.f);
		g.fillRect(lbar);
		DropShadow ds(Colour(COLOR_ACTIVE).withAlpha(0.5f), 6, {0, 0});
		ds.drawForRectangle(g, lbar.toNearestInt());
	}

    if (rmsRight) {
		g.setColour(Colour(COLOR_ACTIVE));
		auto rbar = barbounds
			.translated(0.f, 6.f * 2)
			.withHeight(barbounds.getHeight() * std::min(zeroMeter, rmsRight))
			.withY(bounds.getBottom() - bounds.getHeight() / 4 - barbounds.getHeight() / 2 - 1.f);
        g.fillRect(rbar);
		DropShadow ds3(Colour(COLOR_ACTIVE).withAlpha(0.5f), 6, {0, 0});
		ds3.drawForRectangle(g, rbar.toNearestInt());
	}

	g.restoreState();
}