#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

using namespace globals;
class RipplerAudioProcessorEditor;

class UIUtils {
public:

static void UIUtils::drawBevel(Graphics& g, Rectangle<float> bounds, float corner, Colour bg);
static void UIUtils::drawMenu(Graphics& g, Rectangle<float>bounds, Colour c, bool drawEllipsis);
static void UIUtils::drawTriangle(Graphics& g, Rectangle<float> bounds, int direction, Colour c);
static void UIUtils::drawClock(Graphics& g, Rectangle<float> bounds, Colour color);
static void UIUtils::drawNote(Graphics& g, Rectangle<float> bounds, int mode, Colour color);
static void UIUtils::drawGear(Graphics& g, Rectangle<int> bounds, float radius, int segs, Colour color, Colour background);
static void drawPeak(Graphics& g, Rectangle<float> bounds, Colour c, float scale = 1.f);
static void drawBandPass(Graphics& g, Rectangle<float> bounds, Colour c, float scale = 1.f);
static void drawLowShelf(Graphics& g, Rectangle<float> bounds, Colour c, float scale = 1.f);
static void drawNotch(Graphics& g, Rectangle<float> bounds, Colour c, float scale = 1.f);
static void drawHighShelf(Graphics& g, Rectangle<float> bounds, Colour c, float scale = 1.f);
static void drawHighpass(Graphics& g, Rectangle<float> bounds, Colour c, float scale = 1.f);
static void drawLowpass(Graphics& g, Rectangle<float> bounds, Colour c, float scale = 1.f);
};
