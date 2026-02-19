#include "LofiWidget.h"
#include "../PluginEditor.h"

LofiWidget::LofiWidget(QDelayAudioProcessorEditor& e)
	: editor(e)
{
	editor.audioProcessor.params.addParameterListener("crush_upsample", this);

	srate = std::make_unique<Rotary>(editor.audioProcessor, "crush_srate", "Srate", Rotary::lofiSrate);
	addAndMakeVisible(srate.get());

	bits = std::make_unique<Rotary>(editor.audioProcessor, "crush_bits", "Bits", Rotary::lofiBits);
	addAndMakeVisible(bits.get());

	addAndMakeVisible(pathBtn);
	pathBtn.setAlpha(0.f);
	pathBtn.onClick = [this]
		{
			showPathMenu();
		};

	addAndMakeVisible(upsampleBtn);
	upsampleBtn.setComponentID("button-noborder");
	upsampleBtn.setButtonText("Upsamp");
	bool upsample = editor.audioProcessor.params.getRawParameterValue("crush_upsample")->load();
	upsampleBtn.setToggleState(upsample, dontSendNotification);
	upsampleBtn.onClick = [this]
		{
			auto param = editor.audioProcessor.params.getParameter("crush_upsample");
			param->setValueNotifyingHost(param->getValue() > 0.f ? 0.f : 1.f);
		};
}

LofiWidget::~LofiWidget()
{
	editor.audioProcessor.params.removeParameterListener("dist_mode", this);
	editor.audioProcessor.params.removeParameterListener("crush_upsample", this);
}

void LofiWidget::parameterChanged(const juce::String& parameterID, float newValue)
{
	if (parameterID  == "crush_upsample")
		upsampleBtn.setToggleState((bool)newValue, dontSendNotification);
}

void LofiWidget::paint(Graphics& g)
{
	g.setColour(Colour(COLOR_NEUTRAL));
	int path = (int)editor.audioProcessor.params.getRawParameterValue("crush_path")->load();

	UIUtils::drawBevel(g, pathBtn.getBounds().toFloat().reduced(0.5f), BEVEL_CORNER, Colour(COLOR_BEVEL));
	g.setFont(FontOptions(16.f));
	g.setColour(Colours::white);
	g.drawText(path == 0 ? "Pre" : "Post", pathBtn.getBounds().toFloat(), Justification::centred);
}

void LofiWidget::resized()
{
	auto b = getLocalBounds();

	upsampleBtn.setBounds(b.getX() + KNOB_WIDTH * 2, b.getY() + 5, KNOB_WIDTH, 20);
	pathBtn.setBounds(Rectangle<int>(b.getX(), b.getY() + KNOB_HEIGHT / 2 - 25 / 2 + 35, KNOB_WIDTH, 25));
	srate->setBounds(b.getX() + KNOB_WIDTH, b.getY() + 35, KNOB_WIDTH, KNOB_HEIGHT);
	bits->setBounds(srate->getBounds().translated(KNOB_WIDTH, 0));
}

void LofiWidget::showPathMenu()
{
	int path = (int)editor.audioProcessor.params.getRawParameterValue("crush_path")->load();

	PopupMenu menu;
	menu.addItem(1, "Pre", true, path == 0);
	menu.addItem(2, "Post", true, path == 1);

	auto menuPos = localPointToGlobal(pathBtn.getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this](int result)
		{
			if (result == 0) return;
			auto param = editor.audioProcessor.params.getParameter("crush_path");
			param->setValueNotifyingHost(param->convertTo0to1(float(result - 1)));
			repaint();
		}
	);
}