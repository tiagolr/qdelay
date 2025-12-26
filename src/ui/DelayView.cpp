#include "DelayView.h"
#include "../PluginEditor.h"

DelayView::DelayView(QDelayAudioProcessorEditor& e)
	:editor(e)
{

}

DelayView::~DelayView()
{

}

void DelayView::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
}

void DelayView::paint(Graphics& g)
{
	auto b = getLocalBounds().toFloat();
	UIUtils::drawBevel(g, b, BEVEL_CORNER, Colour(COLOR_BEVEL));
}