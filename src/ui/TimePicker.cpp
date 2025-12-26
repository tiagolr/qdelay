#include "TimePicker.h"
#include "../PluginEditor.h"

TimePicker::TimePicker(QDelayAudioProcessorEditor& e, String paramId)
	: editor(e)
	, paramId(paramId)
{
	editor.audioProcessor.params.addParameterListener(paramId, this);
}

TimePicker::~TimePicker()
{
	editor.audioProcessor.params.removeParameterListener(paramId, this);
}

void TimePicker::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
	MessageManager::callAsync([this] { repaint(); });
};

String TimePicker::getSyncText(float value)
{
	auto i = (int)value;
	String text = "1/64";
	if (i == 0) text = "1/64";
	if (i == 1) text = "1/32";
	if (i == 2) text = "1/16";
	if (i == 3) text = "1/8";
	if (i == 4) text = "1/4";
	if (i == 5) text = "1/2";
	if (i == 6) text = "1/1";

	if (mode == 2) text += " T";
	if (mode == 3) text += " .";

	return text;
}

void TimePicker::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat().reduced(.5f);
	UIUtils::drawBevel(g, bounds, BEVEL_CORNER, Colour(COLOR_BEVEL));

	g.setColour(Colours::white);
	g.setFont(FontOptions(15.f));

	auto value = editor.audioProcessor.params.getRawParameterValue(paramId)->load();
	auto text = mode > 0
		? getSyncText(value)
		: value <= 1
		? String(std::round(value * 1000)) + " ms"
		: String(std::round(value * 100) / 100.f) + " s";

	g.drawText(text, bounds, Justification::centred, false);

	auto modLane = bounds.withHeight(2.f).withBottomY(bounds.getBottom() - 2).toFloat();
	//auto param = editor.audioProcessor.params.getParameter(paramId);
	//auto normValue = param->getValue();

	g.saveState();
	Path pp;
	pp.addRoundedRectangle(bounds.reduced(1), BEVEL_CORNER, BEVEL_CORNER);
	g.reduceClipRegion(pp);
	g.restoreState();
}

void TimePicker::mouseDown(const juce::MouseEvent& e)
{
	mouse_down = true;
	e.source.enableUnboundedMouseMovement(true);
	auto param = editor.audioProcessor.params.getParameter(paramId);
	cur_normed_value = param->getValue();
	setMouseCursor(MouseCursor::NoCursor);
	start_mouse_pos = Desktop::getInstance().getMousePosition();
	last_mouse_position = e.getPosition();
	param->beginChangeGesture();
}

void TimePicker::mouseUp(const juce::MouseEvent& e)
{
	if (!mouse_down) return;
	mouse_down = false;
	setMouseCursor(MouseCursor::NormalCursor);
	e.source.enableUnboundedMouseMovement(false);
	Desktop::getInstance().setMousePosition(start_mouse_pos);
	auto param = editor.audioProcessor.params.getParameter(paramId);
	param->endChangeGesture();
}

void TimePicker::mouseDrag(const juce::MouseEvent& e)
{
	if (!mouse_down) return;
	auto change = e.getPosition() - last_mouse_position;
	last_mouse_position = e.getPosition();
	auto speed = (e.mods.isShiftDown() ? 40.0f : 4.0f) * pixels_per_percent;
	if (mode == 0) speed *= 2;
	auto slider_change = float(change.getX() - change.getY()) / speed;
	cur_normed_value += slider_change;
	auto param = editor.audioProcessor.params.getParameter(paramId);
	param->setValueNotifyingHost(cur_normed_value);
}

void TimePicker::mouseDoubleClick(const juce::MouseEvent& e)
{
	(void)e;
	auto param = editor.audioProcessor.params.getParameter(paramId);
	param->setValueNotifyingHost(param->getDefaultValue());
}

void TimePicker::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
	if (e.mods.isLeftButtonDown() || e.mods.isRightButtonDown()) {
		return; // prevent crash, param is already mutating
	}

	auto slider_change = wheel.deltaY > 0 ? 1.0f : wheel.deltaY < 0 ? -1.0f : 0;
	auto param = editor.audioProcessor.params.getParameter(paramId);
	auto val = param->convertFrom0to1(param->getValue());
	param->setValueNotifyingHost(param->convertTo0to1(val + slider_change));
}