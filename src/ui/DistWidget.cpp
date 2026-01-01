#include "DistWidget.h"
#include "../PluginEditor.h"

DistWidget::DistWidget(QDelayAudioProcessorEditor& e)
	: editor(e)
{
	editor.audioProcessor.params.addParameterListener("dist_mode", this);

	addAndMakeVisible(modeBtn);
	modeBtn.setAlpha(0.f);
	modeBtn.onClick = [this]
		{
			showModeMenu();
		};

	drive = std::make_unique<Rotary>(editor.audioProcessor, "dist_drive", "Drive", Rotary::percx100);
	addAndMakeVisible(drive.get());

	trim = std::make_unique<Rotary>(editor.audioProcessor, "dist_trim", "Trim", Rotary::dBfloat1, true);
	addAndMakeVisible(trim.get());

	color = std::make_unique<Rotary>(editor.audioProcessor, "dist_color", "Color", Rotary::percx100);
	addAndMakeVisible(color.get());

	bias = std::make_unique<Rotary>(editor.audioProcessor, "dist_bias", "Bias", Rotary::percx100);
	bias->setTooltip("Bias shifts the waveshaper curve producing odd harmonics.");
	addAndMakeVisible(bias.get());

	dynamics = std::make_unique<Rotary>(editor.audioProcessor, "dist_dyn", "Dynam", Rotary::percx100);
	dynamics->setTooltip("Dynamics makes distortion react to signal level - louder hits harder, quieter less.");
	addAndMakeVisible(dynamics.get());
}

DistWidget::~DistWidget()
{
	editor.audioProcessor.params.removeParameterListener("dist_mode", this);
}

void DistWidget::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
	MessageManager::callAsync([this] { repaint(); });
}

void DistWidget::paint(Graphics& g)
{
	int mode = (int)editor.audioProcessor.params.getRawParameterValue("dist_mode")->load();

	UIUtils::drawBevel(g, modeBtn.getBounds().toFloat().reduced(0.5f), BEVEL_CORNER, Colour(COLOR_BEVEL));
	g.setFont(FontOptions(16.f));
	g.setColour(Colours::white);
	g.drawText(mode == 0 ? "Tape" : "Tanh", modeBtn.getBounds().toFloat(), Justification::centred);
	
}

void DistWidget::resized()
{
	auto b = getLocalBounds();

	modeBtn.setBounds(Rectangle<int>(b.getX(), b.getY() + KNOB_HEIGHT / 2 - 25 / 2, KNOB_WIDTH, 25));

	drive->setBounds(b.getX() + KNOB_WIDTH, b.getY(), KNOB_WIDTH, KNOB_HEIGHT);
	trim->setBounds(b.getX() + KNOB_WIDTH * 2, b.getY(), KNOB_WIDTH, KNOB_HEIGHT);
	color->setBounds(b.getX(), b.getY() + KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);
	bias->setBounds(b.getX() + KNOB_WIDTH, b.getY() + KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);
	dynamics->setBounds(b.getX() + KNOB_WIDTH * 2, b.getY() + KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);
}

void DistWidget::showModeMenu()
{
	int mode = (int)editor.audioProcessor.params.getRawParameterValue("dist_mode")->load();

	PopupMenu menu;
	menu.addItem(1, "Tape", true, mode == 0);
	menu.addItem(2, "Tanh", true, mode == 1);

	auto menuPos = localPointToGlobal(modeBtn.getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this](int result)
		{
			if (result == 0) return;
			auto param = editor.audioProcessor.params.getParameter("dist_mode");
			param->setValueNotifyingHost(param->convertTo0to1(float(result - 1)));
		}
	);
}