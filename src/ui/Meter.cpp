#include "Meter.h"
#include "../PluginProcessor.h"

static float gainToScale(float g)
{
	float db = 20.0f * std::log10(std::max(g, 1e-9f));
	constexpr float minDb = -60.0f;
	constexpr float maxDb = 0.0f;
	return juce::jlimit(
		0.0f, 1.0f,
		(db - minDb) / (maxDb - minDb)
	);
}

Meter::Meter(QDelayAudioProcessor& p)
	: audioProcessor(p)
{
	db6 = gainToScale(0.5f);
	db18 = gainToScale(0.126f);
	db30 = gainToScale(0.0316f);
	db42 = gainToScale(0.00794f);
    startTimerHz(60);
}

void Meter::timerCallback()
{
    repaint();
}

Meter::~Meter()
{
}

void drawMarkers(Graphics& g, Rectangle<float>barbounds, Colour c, float db6, float db18, float db30, float db42)
{
	g.setColour(Colour(c));
	g.drawHorizontalLine((int)(barbounds.getBottom() - db6 * barbounds.getHeight()), barbounds.getX(), barbounds.getX() + barbounds.getWidth() * 2);
	g.drawHorizontalLine((int)(barbounds.getBottom() - db18 * barbounds.getHeight()), barbounds.getX(), barbounds.getX() + barbounds.getWidth() * 2);
	g.drawHorizontalLine((int)(barbounds.getBottom() - db30 * barbounds.getHeight()), barbounds.getX(), barbounds.getX() + barbounds.getWidth() * 2);
	g.drawHorizontalLine((int)(barbounds.getBottom() - db42 * barbounds.getHeight()), barbounds.getX(), barbounds.getX() + barbounds.getWidth() * 2);


	g.setFont(FontOptions(12.f));
	auto r = Rectangle<float>(barbounds.getX(), 0, barbounds.getWidth() * 2, 16.f);
	r = r.withBottomY(barbounds.getBottom() - db6 * barbounds.getHeight());
	g.drawText("-6", r, Justification::centred);
	r = r.withBottomY(barbounds.getBottom() - db18 * barbounds.getHeight());
	g.drawText("-18", r, Justification::centred);
	r = r.withBottomY(barbounds.getBottom() - db30 * barbounds.getHeight());
	g.drawText("-30", r, Justification::centred);
	r = r.withBottomY(barbounds.getBottom() - db42 * barbounds.getHeight());
	g.drawText("-42", r, Justification::centred);
}

void Meter::paint(juce::Graphics& g) {
	auto bounds = getLocalBounds().toFloat();
	//UIUtils::drawBevel(g, bounds.reduced(0.5f), BEVEL_CORNER, Colour(0xff1a1a1a));
	g.setColour(Colour(COLOR_BEVEL).brighter(0.05f));
	g.fillRoundedRectangle(bounds.reduced(0.5f), BEVEL_CORNER);
	g.setColour(Colour(COLOR_BEVEL));
	g.drawRoundedRectangle(bounds.reduced(0.5f), BEVEL_CORNER, 1.f);

	auto barbounds = bounds.reduced(4.f);
	barbounds.setWidth(std::ceil(barbounds.getWidth() / 2));

	float rawRmsL = audioProcessor.rmsLeft.load();
	float rawRmsR = audioProcessor.rmsRight.load();

	constexpr float alpha = 0.15f;
	rmsSmoothedL = rmsSmoothedL < rawRmsL ? rawRmsL
		: (1.0f - alpha) * rmsSmoothedL + alpha * rawRmsL;
	rmsSmoothedR = rmsSmoothedR < rawRmsR ? rawRmsR
		: (1.0f - alpha) * rmsSmoothedR + alpha * rawRmsR;
	float rmsLeft = gainToScale(rmsSmoothedL);
	float rmsRight = gainToScale(rmsSmoothedR);

	drawMarkers(g, barbounds, Colour(COLOR_NEUTRAL), db6, db18, db30, db42);

	auto lbar = barbounds
		.withTrimmedRight(1.f)
		.withHeight(barbounds.getHeight() * std::min(1.f, rmsLeft))
		.withBottomY(barbounds.getBottom());
    if (rmsLeft > 0.f) {
		g.setColour(Colour(COLOR_ACTIVE));
		g.fillRect(lbar);
	}
	auto rbar = barbounds
		.translated(barbounds.getWidth(), 0)
		.withTrimmedLeft(1.f)
		.withHeight(barbounds.getHeight() * std::min(1.f, rmsRight))
		.withBottomY(barbounds.getBottom());
    if (rmsRight > 0.f) {
		g.setColour(Colour(COLOR_ACTIVE));
		
        g.fillRect(rbar);
	}

	g.saveState();
	g.reduceClipRegion(lbar.expanded(2.f, 0).toNearestInt());
	drawMarkers(g, barbounds, Colour(COLOR_BEVEL), db6, db18, db30, db42);
	g.restoreState();
	g.saveState();
	g.reduceClipRegion(rbar.expanded(2.f, 0).toNearestInt());
	drawMarkers(g, barbounds, Colour(COLOR_BEVEL), db6, db18, db30, db42);
	g.restoreState();
}