#include "EQWidget.h"
#include "../PluginEditor.h"

// ==============================================================================

EQWidget::EQWidget(QDelayAudioProcessorEditor& e, SVF::EQType _type)
	: editor(e)
	, type(_type)
	, prel(_type == SVF::ParamEQ ? "input" : "decay")
{
	eq = std::make_unique<EQDisplay>(editor, type);
	addAndMakeVisible(eq.get());
	eq->onMouseDownBand = [this](int band)
		{
			selband = band;
			freqknobs[selband]->forceLabelShowValue = true;
			gainknobs[selband]->forceLabelShowValue = true;
			freqknobs[selband]->repaint();
			gainknobs[selband]->repaint();
			toggleUIComponents();
		};
	eq->onMouseUp = [this]
		{
			freqknobs[selband]->forceLabelShowValue = false;
			gainknobs[selband]->forceLabelShowValue = false;
			freqknobs[selband]->repaint();
			gainknobs[selband]->repaint();
		};
	eq->onMouseDrag = [this]
		{
			freqknobs[selband]->repaint();
			gainknobs[selband]->repaint();
		};

	for (int i = 0; i < EQ_BANDS; ++i) {
		auto pre = prel + String("eq_band") + String(i + 1);
		if (i == 0 || i == EQ_BANDS - 1) {
			editor.audioProcessor.params.addParameterListener(pre + "_mode", this);
		}
		auto freq = std::make_unique<Rotary>(editor.audioProcessor, pre + "_freq", "Freq", Rotary::hz);
		auto q = std::make_unique<Rotary>(editor.audioProcessor, pre + "_q", "Q", Rotary::float1);
		auto gain = std::make_unique<Rotary>(editor.audioProcessor, pre + "_gain", "Gain", Rotary::dBfloat1, true);
		addChildComponent(freq.get());
		addChildComponent(q.get());
		addChildComponent(gain.get());
		freqknobs.push_back(std::move(freq));
		qknobs.push_back(std::move(q));
		gainknobs.push_back(std::move(gain));
	}

	addAndMakeVisible(bandBtn);
	bandBtn.setAlpha(0.f);
	bandBtn.onClick = [this]
		{
			showBandModeMenu();
		};

	addAndMakeVisible(inputBtn);
	inputBtn.setComponentID("button-noborder");
	inputBtn.setToggleState(_type == SVF::ParamEQ, dontSendNotification);
	inputBtn.setButtonText("Input");
	inputBtn.onClick = [this]
		{
			editor.setEQTab(false);
		};

	addAndMakeVisible(feedbkBtn);
	feedbkBtn.setComponentID("button-noborder");
	feedbkBtn.setButtonText("Feedback");
	feedbkBtn.setToggleState(_type == SVF::DecayEQ, dontSendNotification);
	feedbkBtn.onClick = [this]
		{
			editor.setEQTab(true);
		};
}

EQWidget::~EQWidget()
{
	for (int i = 0; i < EQ_BANDS; ++i) {
		auto pre = prel + String("eq_band") + String(i + 1);
		if (i == 0 || i == EQ_BANDS - 1) {
			editor.audioProcessor.params.removeParameterListener(pre + "_mode", this);
		}
	}
}

void EQWidget::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
	MessageManager::callAsync([this] { repaint(); });
}

void EQWidget::resized()
{
	auto b = getLocalBounds();
	eq->setBounds(Rectangle<int>(b.getX(), b.getY() + HEADER_HEIGHT + 10, KNOB_WIDTH*3, KNOB_HEIGHT*2)
		.withTrimmedTop(10));

	for (int i = 0; i < EQ_BANDS; ++i) {
		freqknobs[i]->setBounds(b.getX(), b.getBottom() - KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);
		gainknobs[i]->setBounds(freqknobs[i]->getBounds().translated(KNOB_WIDTH,0));
		qknobs[i]->setBounds(freqknobs[i]->getBounds().translated(KNOB_WIDTH*2, 0));
	}

	bandBtn.setBounds(Rectangle<int>(30, 30)
		.withX(b.getRight() - 30)
		.withY(b.getY() + 10));

	inputBtn.setBounds(b.getX(), eq->getBottom() + 15, eq->getWidth() / 2, VSEPARATOR);
	feedbkBtn.setBounds(inputBtn.getBounds().translated(inputBtn.getWidth(), 0));

	toggleUIComponents();
}

void EQWidget::paint(Graphics& g)
{
	g.fillAll(Colour(COLOR_BACKGROUND));

	// draw band curve button
	g.setColour(Colour(COLOR_NEUTRAL));
	g.drawRoundedRectangle(bandBtn.getBounds().toFloat().reduced(0.5f), 3.f, 1.f);
	auto mode = eq->bandFilters[selband].mode;
	if (mode == SVF::PK) {
		UIUtils::drawPeak(g, bandBtn.getBounds().toFloat().translated(4.5f, 6.5f), Colours::white, 1.2f);
	}
	else if (mode == SVF::LP) {
		UIUtils::drawLowpass(g, bandBtn.getBounds().toFloat().translated(4.5f, 8.5f), Colours::white, 1.2f);
	}
	else if (mode == SVF::BP) {
		UIUtils::drawBandPass(g, bandBtn.getBounds().toFloat().translated(7.5f, 7.5f), Colours::white, 1.2f);
	}
	else if (mode == SVF::HP) {
		UIUtils::drawHighpass(g, bandBtn.getBounds().toFloat().translated(4.5f, 8.5f), Colours::white, 1.2f);
	}
	else if (mode == SVF::LS) {
		UIUtils::drawLowShelf(g, bandBtn.getBounds().toFloat().translated(4.5f, 6.5f), Colours::white, 1.2f);
	}
	else if (mode == SVF::HS) {
		UIUtils::drawHighShelf(g, bandBtn.getBounds().toFloat().translated(4.5f, 6.5f), Colours::white, 1.2f);
	}

	g.setFont(FontOptions(16.f));
	g.setColour(Colour(COLOR_NEUTRAL));
	g.drawText("Band " + String(selband + 1), bandBtn.getBounds().withWidth(50)
		.translated(-60,0), Justification::centredRight);
}

void EQWidget::toggleUIComponents()
{
	for (int i = 0; i < EQ_BANDS; ++i) {
		freqknobs[i]->setVisible(selband == i);
		qknobs[i]->setVisible(selband == i);
		gainknobs[i]->setVisible(selband == i);
	}

	repaint();
}

void EQWidget::showBandModeMenu()
{
	auto mode = SVF::PK;
	auto m = (int)editor.audioProcessor.params.getRawParameterValue(prel + "eq_band" + String(selband + 1) + "_mode")->load();
	if (selband == 0 && m == 0) mode = SVF::HP;
	else if (selband == 0 && m > 0) mode = SVF::LS;
	else if (selband == EQ_BANDS - 1 && m == 0) mode = SVF::LP;
	else if (selband == EQ_BANDS - 1 && m > 0) mode = SVF::HS;
	else if (m == 0) mode = SVF::BP;
	else if (m == 2) mode = SVF::BS;
	else mode = SVF::PK;

	PopupMenu menu;
	if (selband == 0) {
		menu.addItem(1, "Low Cut", true, mode == SVF::HP);
		menu.addItem(2, "Low Shelf", true, mode == SVF::LS);
	}
	else if (selband == EQ_BANDS - 1) {
		menu.addItem(3, "High Cut", true, mode == SVF::LP);
		menu.addItem(4, "High Shelf", true, mode == SVF::HS);
	}
	else {
		menu.addItem(5, "Band pass", true, mode == SVF::BP);
		menu.addItem(6, "Peak", true, mode == SVF::PK);
	}

	auto menuPos = localPointToGlobal(bandBtn.getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this](int result) {
			if (result == 0) return;
			if (result == 1 || result == 2) {
				auto param = editor.audioProcessor.params.getParameter(prel + "eq_band1_mode");
				param->setValueNotifyingHost(param->convertTo0to1((float)result - 1));
				eq->updateEQCurve();
				toggleUIComponents();
			}
			if (result == 3 || result == 4) {
				auto param = editor.audioProcessor.params.getParameter(prel + "eq_band" + String(EQ_BANDS) + "_mode");
				param->setValueNotifyingHost(param->convertTo0to1((float)result - 3));
				eq->updateEQCurve();
				toggleUIComponents();
			}
			else if (result == 5 || result == 6) {
				auto param = editor.audioProcessor.params.getParameter(prel + "eq_band" + String(selband+1) + "_mode");
				param->setValueNotifyingHost(param->convertTo0to1((float)result - 5));
				eq->updateEQCurve();
				toggleUIComponents();
			}
		});
}