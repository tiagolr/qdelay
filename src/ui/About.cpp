#include "About.h"

About::About()
{
	addAndMakeVisible(siteLink);
	siteLink.setURL(URL("https://github.com/tiagolr/qdelay"));
	siteLink.setButtonText("github.com/tiagolr/qdelay");
	siteLink.setFont(FontOptions(20.f), false, Justification::centred);
	siteLink.setColour(HyperlinkButton::ColourIds::textColourId, Colour(COLOR_ACTIVE));
}

void About::resized()
{
	auto b = getLocalBounds();
	siteLink.setBounds(b.getCentreX() - 150, b.getY() + 50 + 30 + 20 + 20 + 15, 300, 25);
}

void About::mouseDown(const juce::MouseEvent& e)
{
	(void)e;
	setVisible(false);
};

void About::paint(Graphics& g)
{
	auto bounds = getLocalBounds();
	g.setColour(Colour(0xdd000000));
	g.fillRect(bounds);

	bounds.reduce(50,50);
	g.setColour(Colours::white);
	g.setFont(FontOptions(30.f));
	g.drawText("QDelay", bounds.removeFromTop(35), Justification::centred);
	g.setFont(FontOptions(20.f));
	g.drawText(std::string("v") + PROJECT_VERSION, bounds.removeFromTop(25), Justification::centred);
	g.setFont(FontOptions(20.0f));
	g.drawText("Copyright (C) Tilr 2026", bounds.removeFromTop(25), Justification::centred);
	g.setColour(Colour(COLOR_ACTIVE));
	bounds.removeFromTop(25); // site link
	g.setColour(Colours::white);
	bounds.removeFromTop(40);
	auto w = PLUG_WIDTH - 100;
	bounds.setWidth(w);
	bounds.setX(getWidth() / 2 - w/2);
};

