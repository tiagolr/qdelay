#include "DistWidget.h"

DistWidget::DistWidget(QDelayAudioProcessorEditor& e)
	: editor(e)
{

}

DistWidget::~DistWidget()
{
}

void DistWidget::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
}

void DistWidget::paint(Graphics& g)
{
	(void)g;
}

void DistWidget::resized()
{

}

void DistWidget::toggleUIComponents()
{
}