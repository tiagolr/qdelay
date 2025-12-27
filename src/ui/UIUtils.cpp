#include "UIUtils.h"

void UIUtils::drawBevel(Graphics& g, Rectangle<float> bounds, float corner, Colour bg)
{
    bounds = bounds.translated(0.5f, 0.5f);
    juce::ColourGradient gradient(
        Colour(0xff0F0F0F).withAlpha(0.35f), bounds.getX(), bounds.getY(),
        Colours::white.withAlpha(0.12f), bounds.getX(), bounds.getBottom(), false
    );
    gradient.addColour(0.5, Colour(0xff0F0F0F).withAlpha(0.02f));
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, corner);

    g.setColour(bg);
    g.fillRoundedRectangle(bounds.expanded(-1.f), corner);
}

void UIUtils::drawMenu(Graphics& g, Rectangle<float>bounds, Colour c, bool drawEllipsis)
{
    Path p;
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    if (drawEllipsis) {
        float r = 1.5f;
        p.addEllipse(cx - r, cy - r, r, r);
        p.addEllipse(cx - r, cy - 7 - r, r, r);
        p.addEllipse(cx - r, cy + 7 - r, r, r);
    }
    else {
        p.startNewSubPath(bounds.getX(), bounds.getCentreY());
        p.lineTo(bounds.getRight(), bounds.getCentreY());
        p.startNewSubPath(bounds.getX(), bounds.getCentreY() - 6);
        p.lineTo(bounds.getRight(), bounds.getCentreY() - 6);
        p.startNewSubPath(bounds.getX(), bounds.getCentreY() + 6);
        p.lineTo(bounds.getRight(), bounds.getCentreY() + 6);
    }

    g.setColour(c);
    g.strokePath(p, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));
}

void UIUtils::drawTriangle(Graphics& g, Rectangle<float> bounds, int direction, Colour c)
{
    g.setColour(c);
    Path p;
    if (direction == 0) { // top
        p.startNewSubPath(bounds.getBottomLeft());
        p.lineTo({ bounds.getCentreX(), bounds.getY() });
        p.lineTo(bounds.getBottomRight());
    }
    else if (direction == 1) { // right
        p.startNewSubPath(bounds.getTopLeft());
        p.lineTo({ bounds.getRight(), bounds.getCentreY() });
        p.lineTo(bounds.getBottomLeft());
    }
    else if (direction == 2) { // bottom
        p.startNewSubPath(bounds.getTopLeft());
        p.lineTo({ bounds.getCentreX(), bounds.getBottom() });
        p.lineTo(bounds.getTopRight());
    }
    else { // right
        p.startNewSubPath(bounds.getTopRight());
        p.lineTo({ bounds.getX(), bounds.getCentreY() });
        p.lineTo(bounds.getBottomRight());
    }
    p.closeSubPath();
    g.fillPath(p);
}

void UIUtils::drawClock(Graphics& g, Rectangle<float> bounds, Colour color)
{
    g.setColour(color);
    g.drawEllipse(bounds, 1.f);
    auto cx = bounds.getCentreX();
    auto cy = bounds.getCentreY();
    g.drawLine(cx, cy, cx, cy - bounds.getHeight() * 0.25f);
    g.drawLine(cx, cy, cx + bounds.getWidth() * .25f, cy + bounds.getHeight() * .25f);
}

// mode = 0, straight, 1 = tripplet, 2 = dotted
void UIUtils::drawNote(Graphics& g, Rectangle<float> bounds, int mode, Colour color)
{
    auto r = 3;
    g.setColour(color);
    g.fillEllipse(bounds.getCentreX() - r, bounds.getBottom() - r * 2, r * 2.f, r * 2.f);
    g.drawVerticalLine((int)bounds.getCentreX() + r - 1, bounds.getBottom() - 10 - r, bounds.getBottom() - r);

    g.setFont(FontOptions(12.f));
    if (mode == 1)
        g.drawText("T", (int)bounds.getCentreX() + r + 2, (int)bounds.getBottom() - 12, 12, 12, Justification::centredLeft);
    if (mode == 2) {
        g.setFont(FontOptions(16.f));
        g.drawText(".", (int)bounds.getCentreX() + r + 2, (int)bounds.getBottom() - 16, 16, 16, Justification::centredLeft);
    }
}

void UIUtils::drawGear(Graphics& g, Rectangle<int> bounds, float radius, int segs, Colour color, Colour background)
{
    float x = bounds.toFloat().getCentreX();
    float y = bounds.toFloat().getCentreY();
    float oradius = radius;
    float iradius = radius / 3.f;
    float cradius = iradius / 1.5f;
    float coffset = MathConstants<float>::twoPi;
    float inc = MathConstants<float>::twoPi / segs;

    g.setColour(color);
    g.fillEllipse(x - oradius, y - oradius, oradius * 2.f, oradius * 2.f);

    g.setColour(background);
    for (int i = 0; i < segs; i++) {
        float angle = coffset + i * inc;
        float cx = x + std::cos(angle) * oradius;
        float cy = y + std::sin(angle) * oradius;
        g.fillEllipse(cx - cradius, cy - cradius, cradius * 2, cradius * 2);
    }
    g.fillEllipse(x - iradius, y - iradius, iradius * 2.f, iradius * 2.f);
}

void UIUtils::drawChain(Graphics& g, Rectangle<float> bounds, Colour color, float scale)
{
    Path p;

    p.startNewSubPath(0.5f, 7.37234f);
    p.lineTo(0.5f, 4.80818f);
    p.cubicTo(0.517043f, 3.65918f, 0.945923f, 2.56357f, 1.69414f, 1.75762f);
    p.cubicTo(2.44236f, 0.95168f, 3.44995f, 0.5f, 4.4996f, 0.5f);
    p.cubicTo(5.54925f, 0.5f, 6.55685f, 0.951681f, 7.30506f, 1.75762f);
    p.cubicTo(8.05328f, 2.56357f, 8.48216f, 3.65918f, 8.4992f, 4.80817f);
    p.lineTo(8.4992f, 7.37234f);

    p.startNewSubPath(8.4992f, 12.6268f);
    p.lineTo(8.5f, 15.1918f);
    p.cubicTo(8.48296f, 16.3408f, 8.05408f, 17.4364f, 7.30586f, 18.2424f);
    p.cubicTo(6.55765f, 19.0483f, 5.55005f, 19.5f, 4.5004f, 19.5f);
    p.cubicTo(3.45075f, 19.5f, 2.44316f, 19.0483f, 1.69494f, 18.2424f);
    p.cubicTo(0.946723f, 17.4364f, 0.517842f, 16.3408f, 0.5008f, 15.1918f);
    p.lineTo(0.5f, 12.6268f);

    p.startNewSubPath(4.4996f, 6.4966f);
    p.lineTo(4.4996f, 13.5025f);

    p.applyTransform(AffineTransform::scale(scale));
    p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY()));
    g.setColour(color);
    g.strokePath(p, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));
}

void UIUtils::drawPeak(Graphics& g, Rectangle<float> bounds, Colour c, float scale)
{
	Path p;
	p.startNewSubPath(0.0f, 11.5f);
	p.cubicTo(9.0f, 11.5f, 6.5f, 0.5f, 9.0f, 0.5f);
	p.cubicTo(11.5f, 0.5f, 9.0f, 11.5f, 18.0f, 11.5f);
	p.applyTransform(AffineTransform::scale(scale));
	p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY()));
	g.setColour(c);
	g.strokePath(p, PathStrokeType(1.0f, PathStrokeType::curved));
}

void UIUtils::drawBandPass(Graphics& g, Rectangle<float> bounds, Colour c, float scale)
{
	Path p;
	p.startNewSubPath(0.0f, 11.5f);
	p.cubicTo(0.5f, 6.0f,4.0f, 0.5f,7.0f, 0.5f);
	p.cubicTo(10.f, 0.5f,13.5f, 6.0f,13.5f, 11.5f);
	p.applyTransform(AffineTransform::scale(scale));
	p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY()));
	g.setColour(c);
	g.strokePath(p, PathStrokeType(1.0f, PathStrokeType::curved));
}

void UIUtils::drawLowShelf(Graphics& g, Rectangle<float> bounds, Colour c, float scale)
{
	Path p;
    p.startNewSubPath(0.0f, 6.5f);
    p.cubicTo(1.0f, 6.5f, 5.0f, 6.5f, 5.0f, 6.5f);
    p.cubicTo(9.0f, 6.5f, 9.0f, 0.5f, 13.0f, 0.5f);
    p.cubicTo(17.0f, 0.5f, 18.0f, 0.5f, 18.0f, 0.5f);
    p.startNewSubPath(0.0f, 6.5f);
    p.cubicTo(1.0f, 6.5f, 5.0f, 6.5f, 5.0f, 6.5f);
    p.cubicTo(9.0f, 6.5f, 9.0f, 12.5f, 13.0f, 12.5f);
    p.cubicTo(17.0f, 12.5f, 18.0f, 12.5f, 18.0f, 12.5f);
	p.applyTransform(AffineTransform::scale(scale));
	p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY() + 1));
	g.setColour(c);
	g.strokePath(p, PathStrokeType(1.0f, PathStrokeType::curved));
}

void UIUtils::drawNotch(Graphics& g, Rectangle<float> bounds, Colour c, float scale)
{
	Path p;
	p.startNewSubPath(0.0f, .5f);
	p.cubicTo(9.0f, 0.5f,6.5f, 11.5f,9.0f, 11.5f);
	p.cubicTo(11.5f, 11.5f,9.0f, 0.5f,18.0f, 0.5f);
	p.applyTransform(AffineTransform::scale(scale));
	p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY()));
	g.setColour(c);
	g.strokePath(p, PathStrokeType(1.0f, PathStrokeType::curved));
}

void UIUtils::drawHighShelf(Graphics& g, Rectangle<float> bounds, Colour c, float scale)
{
	Path p;
    p.startNewSubPath(18.0f, 6.5f);
    p.cubicTo(17.0f, 6.5f, 13.0f, 6.5f, 13.0f, 6.5f);
    p.cubicTo(9.0f, 6.5f, 9.0f, 0.5f, 5.0f, 0.5f);
    p.cubicTo(1.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f);
    p.startNewSubPath(18.0f, 6.5f);
    p.cubicTo(17.0f, 6.5f, 13.0f, 6.5f, 13.0f, 6.5f);
    p.cubicTo(9.0f, 6.5f, 9.0f, 12.5f, 5.0f, 12.5f);
    p.cubicTo(1.0f, 12.5f, 0.0f, 12.5f, 0.0f, 12.5f);
	p.applyTransform(AffineTransform::scale(scale));
	p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY() + 1));
	g.setColour(c);
	g.strokePath(p, PathStrokeType(1.0f, PathStrokeType::curved));
}

void UIUtils::drawHighpass(Graphics& g, Rectangle<float> bounds, Colour c, float scale)
{
	Path p;
	p.startNewSubPath(18.f, 0.f);
	p.cubicTo(10.f, 0.f, 8.f, 0.5f, 8.f, 0.f);
	p.cubicTo(5.f, 0.f, 5.f, 1.f, 4.f, 4.0f);
	p.cubicTo(3.f, 6.f, 0.f, 11.f, 0.f, 11.f);
	p.applyTransform(AffineTransform::scale(scale));
	p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY()));
	g.setColour(c);
	g.strokePath(p, PathStrokeType(1.0f, PathStrokeType::curved));
}

void UIUtils::drawLowpass(Graphics& g, Rectangle<float> bounds, Colour c, float scale)
{
	Path p;
	p.startNewSubPath(0.0f, 0.f);
	p.cubicTo(8.0f, 0.f, 10.f, 0.f, 10.f, 0.f);
	p.cubicTo(13.0f, 0.f, 13.f, 1.f, 14.f, 4.0f);
	p.cubicTo(15.f, 6.f, 18.0f, 11.f, 18.0f, 11.f);
	p.applyTransform(AffineTransform::scale(scale));
	p.applyTransform(AffineTransform::translation(bounds.getX(), bounds.getY()));
	g.setColour(c);
	g.strokePath(p, PathStrokeType(1.0f, PathStrokeType::curved));
}