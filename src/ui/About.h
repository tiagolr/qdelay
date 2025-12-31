#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

using namespace globals;
class RipplerXAudioProcessor;

class About : public juce::Component {
public:
    HyperlinkButton siteLink;

    About();
    ~About() override {}

    void mouseDown(const juce::MouseEvent& e) override;
    void paint(Graphics& g) override;
    void resized() override;
};