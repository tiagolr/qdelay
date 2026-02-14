#include "PhaserWidget.h"
#include "../PluginEditor.h"

PhaserWidget::PhaserWidget(QDelayAudioProcessorEditor& e)
	: editor(e)
{
	editor.audioProcessor.params.addParameterListener("phaser_path", this);
	editor.audioProcessor.params.addParameterListener("phaser_sync", this);

	mix = std::make_unique<Rotary>(editor.audioProcessor, "phaser_mix", "Mix", Rotary::percx100);
	addAndMakeVisible(mix.get());

	stereo = std::make_unique<Rotary>(editor.audioProcessor, "phaser_stereo", "Stereo", Rotary::percx100);
	addAndMakeVisible(stereo.get());

	morph = std::make_unique<Rotary>(editor.audioProcessor, "phaser_morph", "Morph", Rotary::percx100);
	addAndMakeVisible(morph.get());

	res = std::make_unique<Rotary>(editor.audioProcessor, "phaser_res", "Res", Rotary::percx100);
	addAndMakeVisible(res.get());

	center = std::make_unique<Rotary>(editor.audioProcessor, "phaser_center", "Center", Rotary::hz);
	addAndMakeVisible(center.get());

	rate = std::make_unique<Rotary>(editor.audioProcessor, "phaser_rate", "Rate", Rotary::hz1f);
	addAndMakeVisible(rate.get());

	rateSync = std::make_unique<Rotary>(editor.audioProcessor, "phaser_rate_sync", "Rate", Rotary::kChoice);
	addAndMakeVisible(rateSync.get());

	depth = std::make_unique<Rotary>(editor.audioProcessor, "phaser_depth", "Depth", Rotary::semis2f);
	addAndMakeVisible(depth.get());

	addAndMakeVisible(pathBtn);
	pathBtn.setAlpha(0.f);
	pathBtn.onClick = [this] { showPathMenu(); };

	addAndMakeVisible(syncBtn);
	syncBtn.setAlpha(0.f);
	syncBtn.onClick = [this] { showSyncMenu(); };

	toggleUIComponents();
}

PhaserWidget::~PhaserWidget()
{
	editor.audioProcessor.params.removeParameterListener("phaser_path", this);
	editor.audioProcessor.params.removeParameterListener("phaser_sync", this);
}

void PhaserWidget::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
	MessageManager::callAsync([this] { toggleUIComponents(); });
}

void PhaserWidget::toggleUIComponents()
{
	auto mode = (int)editor.audioProcessor.params.getRawParameterValue("phaser_sync")->load();
	rate->setVisible(mode == 0);
	rateSync->setVisible(mode > 0);
	repaint();
}

void PhaserWidget::paint(Graphics& g)
{
	g.setColour(Colour(COLOR_NEUTRAL));
	int path = (int)editor.audioProcessor.params.getRawParameterValue("phaser_path")->load();

	UIUtils::drawBevel(g, pathBtn.getBounds().toFloat().reduced(0.5f), BEVEL_CORNER, Colour(COLOR_BEVEL));
	g.setFont(FontOptions(16.f));
	g.setColour(Colours::white);
	g.drawText(path == 0 ? "Feedbk" : "Post", pathBtn.getBounds().toFloat(), Justification::centred);
	g.setColour(Colour(COLOR_NEUTRAL));
	auto bounds = center->getBounds().withHeight(30).translated(0, -30);
	g.drawText("LFO", bounds, Justification::centred);

	auto mode = (int)editor.audioProcessor.params.getRawParameterValue("phaser_sync")->load();
	if (mode == 0)
		UIUtils::drawClock(g, syncBtn.getBounds().toFloat(), Colours::white);
	else
		UIUtils::drawNote(g, syncBtn.getBounds().toFloat(), mode - 1, Colours::white);
}

void PhaserWidget::resized()
{
	auto b = getLocalBounds();

	pathBtn.setBounds(Rectangle<int>(b.getX(), b.getY() + KNOB_HEIGHT / 2 - 25 / 2, KNOB_WIDTH, 25));
	mix->setBounds(b.getX() + KNOB_WIDTH, b.getY(), KNOB_WIDTH, KNOB_HEIGHT);
	stereo->setBounds(mix->getBounds().translated(KNOB_WIDTH, 0));
	morph->setBounds(mix->getBounds().translated(0, KNOB_HEIGHT));
	res->setBounds(stereo->getBounds().translated(0, KNOB_HEIGHT));
	center->setBounds(mix->getBounds().translated(-KNOB_WIDTH, KNOB_HEIGHT * 2 + VSEPARATOR + 20));
	rate->setBounds(center->getBounds().translated(KNOB_WIDTH, 0));
	rateSync->setBounds(rate->getBounds());
	depth->setBounds(rate->getBounds().translated(KNOB_WIDTH, 0));
	syncBtn.setBounds(Rectangle<int>(15, 15).withY(rate->getY()).withRightX(rate->getRight()));
}

void PhaserWidget::showPathMenu()
{
	int path = (int)editor.audioProcessor.params.getRawParameterValue("phaser_path")->load();

	PopupMenu menu;
	menu.addItem(1, "Feedback", true, path == 0);
	menu.addItem(2, "Post", true, path == 1);

	auto menuPos = localPointToGlobal(pathBtn.getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this](int result)
		{
			if (result == 0) return;
			auto param = editor.audioProcessor.params.getParameter("phaser_path");
			param->setValueNotifyingHost(param->convertTo0to1(float(result - 1)));
			repaint();
		}
	);
}

void PhaserWidget::showSyncMenu()
{
	auto mode = (int)editor.audioProcessor.params.getRawParameterValue("phaser_sync")->load();

	PopupMenu menu;
	menu.addItem(1, "Hertz", true, mode == 0);
	menu.addItem(2, "Straight", true, mode == 1);
	menu.addItem(3, "Triplet", true, mode == 2);
	menu.addItem(4, "Dotted", true, mode == 3);

	auto menuPos = localPointToGlobal(syncBtn.getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this](int result) {
			if (result == 0) return;
			auto param = editor.audioProcessor.params.getParameter("phaser_sync");
			param->setValueNotifyingHost(param->convertTo0to1((float)(result - 1)));
		});
}