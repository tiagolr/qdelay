#include "DelayWidget.h"
#include "../PluginEditor.h"

DelayWidget::DelayWidget(QDelayAudioProcessorEditor& e)
	:editor(e)
{
	editor.audioProcessor.params.addParameterListener("mode", this);
	editor.audioProcessor.params.addParameterListener("link", this);
	editor.audioProcessor.params.addParameterListener("sync_l", this);
	editor.audioProcessor.params.addParameterListener("sync_r", this);

	addAndMakeVisible(syncModeLBtn);
	syncModeLBtn.setAlpha(0.f);
	addAndMakeVisible(syncModeRBtn);
	syncModeRBtn.setAlpha(0.f);
	addAndMakeVisible(linkBtn);
	linkBtn.setAlpha(0.f);
	linkBtn.setTooltip("Link delays");
	addAndMakeVisible(modeBtn);
	modeBtn.setAlpha(0.f);

	syncModeLBtn.onClick = [this]
		{
			showSyncMenu(true);
		};

	syncModeRBtn.onClick = [this]
		{
			showSyncMenu(false);
		};

	modeBtn.onClick = [this]
		{
			showModeMenu();
		};

	linkBtn.onClick = [this]
		{
			auto param = editor.audioProcessor.params.getParameter("link");
			param->setValueNotifyingHost(param->getValue() > 0.f ? 0.f : 1.f);
		};

	rateL = std::make_unique<TimePicker>(editor, "rate_l");
	rateSyncL = std::make_unique<TimePicker>(editor, "rate_sync_l");
	rateR = std::make_unique<TimePicker>(editor, "rate_r");
	rateSyncR = std::make_unique<TimePicker>(editor, "rate_sync_r");

	addAndMakeVisible(rateL.get());
	addAndMakeVisible(rateR.get());
	addAndMakeVisible(rateSyncL.get());
	addAndMakeVisible(rateSyncR.get());
}

DelayWidget::~DelayWidget()
{
	editor.audioProcessor.params.removeParameterListener("mode", this);
	editor.audioProcessor.params.removeParameterListener("link", this);
	editor.audioProcessor.params.removeParameterListener("sync_l", this);
	editor.audioProcessor.params.removeParameterListener("sync_r", this);
}

void DelayWidget::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
	MessageManager::callAsync([this] { toggleUIComponents(); });
}

void DelayWidget::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();

	auto mode = (Delay::DelayMode)editor.audioProcessor.params.getRawParameterValue("mode")->load();
	auto modeL = (int)editor.audioProcessor.params.getRawParameterValue("sync_l")->load();
	auto modeR = (int)editor.audioProcessor.params.getRawParameterValue("sync_r")->load();
	auto link = (bool)editor.audioProcessor.params.getRawParameterValue("link")->load();

	auto ratebounds = rateL->getBounds();

	if (modeL == 0)
		UIUtils::drawClock(g, syncModeLBtn.getBounds().reduced(6).toFloat(), Colours::white);
	else
		UIUtils::drawNote(g, syncModeLBtn.getBounds().reduced(6).toFloat(), modeL - 1, Colours::white);
	if (modeR == 0)
		UIUtils::drawClock(g, syncModeRBtn.getBounds().reduced(6).toFloat(), Colours::white);
	else
		UIUtils::drawNote(g, syncModeRBtn.getBounds().reduced(6).toFloat(), modeR - 1, Colours::white);

	g.setColour(Colours::white);
	g.setFont(FontOptions(14.f));
	g.drawText("L", rateL->getBounds().toFloat().translated(-10.f, 0.f), Justification::centredLeft);
	g.drawText("R", rateR->getBounds().toFloat().translated(-10.f, 0.f), Justification::centredLeft);

	UIUtils::drawChain(g, linkBtn.getBounds().toFloat().translated(-1.f + 10.f - 2.f, 18.f - 3.f), link ? Colour(COLOR_ACTIVE) : Colour(COLOR_NEUTRAL), 1.2f);
	UIUtils::drawBevel(g, modeBtn.getBounds().toFloat().expanded(0.5f), BEVEL_CORNER, Colour(COLOR_BEVEL));

	g.setColour(Colours::white);
	g.setFont(FontOptions(15.f));
	g.drawText(mode == Delay::Normal ? "Normal"
		: mode == Delay::PingPong ? "PiPo"
		: "Tap", modeBtn.getBounds(), Justification::centred);
}

void DelayWidget::resized()
{
	auto b = getLocalBounds();

	rateL->setBounds(Rectangle<int>(75, 25).withX(b.getCentreX() + 5).withY(b.getCentreY() - 25 - 2));
	rateR->setBounds(Rectangle<int>(75, 25).withX(b.getCentreX() + 5).withY(b.getCentreY() + 2));
	rateSyncL->setBounds(rateL->getBounds());
	rateSyncR->setBounds(rateR->getBounds());

	syncModeLBtn.setBounds(rateL->getBounds().translated(rateL->getWidth(), 0).withWidth(25));
	syncModeRBtn.setBounds(rateR->getBounds().translated(rateR->getWidth(), 0).withWidth(25));
	linkBtn.setBounds(Rectangle<int>(25, 25)
		.withX(rateL->getX() - 20 - 15)
		.withY(rateL->getY())
		.withBottom(rateR->getBottom()));

	modeBtn.setBounds(Rectangle<int>(75, 25)
		.withRightX(linkBtn.getX())
		.withY((int)b.getCentreY() - 25 / 2));

	toggleUIComponents();
}

void DelayWidget::toggleUIComponents()
{
	auto modeL = (int)editor.audioProcessor.params.getRawParameterValue("sync_l")->load();
	auto modeR = (int)editor.audioProcessor.params.getRawParameterValue("sync_r")->load();

	rateL->setVisible(modeL == 0);
	rateSyncL->setVisible(modeL > 0);
	rateR->setVisible(modeR == 0);
	rateSyncR->setVisible(modeR > 0);

	rateSyncL->mode = modeL;
	rateSyncR->mode = modeR;
	rateSyncL->repaint();
	rateSyncR->repaint();

	repaint();
}

void DelayWidget::showSyncMenu(bool isLeft)
{
	auto mode = (Delay::SyncMode)editor.audioProcessor.params.getRawParameterValue(isLeft ? "sync_l" : "sync_r")->load();

	PopupMenu menu;
	menu.addItem(1, "Seconds", true, mode == Delay::RateHz);
	menu.addItem(2, "Straight", true, mode == Delay::Straight);
	menu.addItem(3, "Triplet", true, mode == Delay::Triplet);
	menu.addItem(4, "Dotted", true, mode == Delay::Dotted);

	auto menuPos = localPointToGlobal((isLeft ? syncModeLBtn : syncModeRBtn).getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this, isLeft](int result) {
			if (result == 0) return;
			auto param = editor.audioProcessor.params.getParameter(isLeft ? "sync_l" : "sync_r");
			param->setValueNotifyingHost(param->convertTo0to1((float)(result - 1)));
			toggleUIComponents();
		});
}

void DelayWidget::showModeMenu()
{
	auto mode = (Delay::DelayMode)editor.audioProcessor.params.getRawParameterValue("mode")->load();

	PopupMenu menu;
	menu.addItem(1, "Normal", true, mode == Delay::Normal);
	menu.addItem(2, "PingPong", true, mode == Delay::PingPong);
	menu.addItem(3, "Tap", true, mode == Delay::Tap);

	auto menuPos = localPointToGlobal(modeBtn.getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this](int result) {
			if (result == 0) return;
			auto param = editor.audioProcessor.params.getParameter("mode");
			param->setValueNotifyingHost(param->convertTo0to1((float)result - 1));
			toggleUIComponents();
		});
}