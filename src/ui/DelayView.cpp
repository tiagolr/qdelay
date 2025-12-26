#include "DelayView.h"
#include "../PluginEditor.h"
#include <array>

DelayView::DelayView(QDelayAudioProcessorEditor& e)
	:editor(e)
{
	editor.audioProcessor.params.addParameterListener("mode", this);
	editor.audioProcessor.params.addParameterListener("feedback", this);
	editor.audioProcessor.params.addParameterListener("sync_l", this);
	editor.audioProcessor.params.addParameterListener("sync_r", this);
	editor.audioProcessor.params.addParameterListener("rate_l", this);
	editor.audioProcessor.params.addParameterListener("rate_r", this);
	editor.audioProcessor.params.addParameterListener("rate_sync_l", this);
	editor.audioProcessor.params.addParameterListener("rate_sync_r", this);
	editor.audioProcessor.params.addParameterListener("mix", this);
	editor.audioProcessor.params.addParameterListener("pan_dry", this);
	editor.audioProcessor.params.addParameterListener("pan_wet", this);
	editor.audioProcessor.params.addParameterListener("swing", this);
	editor.audioProcessor.params.addParameterListener("feel", this);
	editor.audioProcessor.params.addParameterListener("accent", this);
	editor.audioProcessor.params.addParameterListener("haas_width", this);
	editor.audioProcessor.params.addParameterListener("pipo_width", this);
}

DelayView::~DelayView()
{
	editor.audioProcessor.params.removeParameterListener("mode", this);
	editor.audioProcessor.params.removeParameterListener("feedback", this);
	editor.audioProcessor.params.removeParameterListener("sync_l", this);
	editor.audioProcessor.params.removeParameterListener("sync_r", this);
	editor.audioProcessor.params.removeParameterListener("rate_l", this);
	editor.audioProcessor.params.removeParameterListener("rate_r", this);
	editor.audioProcessor.params.removeParameterListener("rate_sync_l", this);
	editor.audioProcessor.params.removeParameterListener("rate_sync_r", this);
	editor.audioProcessor.params.removeParameterListener("mix", this);
	editor.audioProcessor.params.removeParameterListener("pan_dry", this);
	editor.audioProcessor.params.removeParameterListener("pan_wet", this);
	editor.audioProcessor.params.removeParameterListener("swing", this);
	editor.audioProcessor.params.removeParameterListener("feel", this);
	editor.audioProcessor.params.removeParameterListener("accent", this);
	editor.audioProcessor.params.removeParameterListener("haas_width", this);
	editor.audioProcessor.params.removeParameterListener("pipo_width", this);
}

void DelayView::parameterChanged(const juce::String& parameterID, float newValue)
{
	(void)parameterID;
	(void)newValue;
	MessageManager::callAsync([this] { repaint(); });
}

static std::array<float, 2> getDelayTimes(
	double secondsPerBeat, Delay::SyncMode syncL, Delay::SyncMode syncR, 
	float rate_l, float rate_r,
	float rate_sync_l, float rate_sync_r
)
{
	if (secondsPerBeat == 0.f) secondsPerBeat = 0.25f;
	auto getSamplesSync = [secondsPerBeat](int rate, Delay::SyncMode sync)
		{
			float qn = 1.f;
			if (rate == 0) qn = 1.f / 16.f; // 1/64
			if (rate == 1) qn = 1.f / 8.f; // 1/32
			if (rate == 2) qn = 1.f / 4.f; // 1/16
			if (rate == 3) qn = 1.f / 2.f; // 1/8
			if (rate == 4) qn = 1.f / 1.f; // 1/4
			if (rate == 5) qn = 1.f * 2.f; // 1/2
			if (rate == 6) qn = 1.f * 4.f; // 1/1
			if (sync == Delay::Triplet) qn *= 2 / 3.f;
			if (sync == Delay::Dotted) qn *= 1.5f;
			return qn * secondsPerBeat;
		};

	auto tl = syncL == 0 ? rate_l : getSamplesSync(rate_sync_l, syncL);
	auto tr = syncR == 0 ? rate_r : getSamplesSync(rate_sync_r, syncR);
	return { (float)tl, (float)tr };
}

void DelayView::paint(Graphics& g)
{
	auto b = getLocalBounds().toFloat();
	UIUtils::drawBevel(g, b, BEVEL_CORNER, Colour(COLOR_BEVEL));
	b = b.reduced(8.f);

	auto mode = (Delay::DelayMode)editor.audioProcessor.params.getRawParameterValue("mode")->load();
	auto feedback = editor.audioProcessor.params.getRawParameterValue("feedback")->load();
	auto sync_l = (Delay::SyncMode)editor.audioProcessor.params.getRawParameterValue("sync_l")->load();
	auto sync_r = (Delay::SyncMode)editor.audioProcessor.params.getRawParameterValue("sync_r")->load();
	auto rate_l = editor.audioProcessor.params.getRawParameterValue("rate_l")->load();
	auto rate_r = editor.audioProcessor.params.getRawParameterValue("rate_r")->load();
	auto rate_sync_l = editor.audioProcessor.params.getRawParameterValue("rate_sync_l")->load();
	auto rate_sync_r = editor.audioProcessor.params.getRawParameterValue("rate_sync_r")->load();
	auto mix = editor.audioProcessor.params.getRawParameterValue("mix")->load();
	auto pan_dry = editor.audioProcessor.params.getRawParameterValue("pan_dry")->load();
	auto pan_wet = editor.audioProcessor.params.getRawParameterValue("pan_wet")->load();
	auto swing = editor.audioProcessor.params.getRawParameterValue("swing")->load();
	auto feel = editor.audioProcessor.params.getRawParameterValue("feel")->load();
	auto accent = editor.audioProcessor.params.getRawParameterValue("accent")->load();
	auto haas_width = editor.audioProcessor.params.getRawParameterValue("haas_width")->load() * MAX_HAAS / 1000.f;
	auto pipo_width = editor.audioProcessor.params.getRawParameterValue("pipo_width")->load();
	float lfactor = pipo_width > 0.f ? 1.f - pipo_width : 1.f;
	float rfactor = pipo_width < 0.f ? 1.f + pipo_width : 1.f;

	auto [timeL, timeR] = getDelayTimes(editor.audioProcessor.secondsPerBeat, sync_l, sync_r, rate_l, rate_r, rate_sync_l, rate_sync_r);

	// balance feedback between left and right delays
	float feedbackR, feedbackL;
	float e = (float)timeL / (float)timeR;
	if (timeL < timeR)
	{
		feedbackR = feedback;
		feedbackL = std::pow(feedback, e);
	}
	else
	{
		e = 1.f / e;
		feedbackL = feedback;
		feedbackR = std::pow(feedback, e);
	}

	VirtualDelay delayL(timeL, feedbackL);
	VirtualDelay delayR(timeR, feedbackR);
	std::vector<VirtualDelay::Tap> leftTaps, rightTaps;

	if (mode == Delay::Normal)
	{
		delayL.seed(0.f, 1.f);
		delayR.seed(0.f, 1.f);

		while (true)
		{
			auto el = delayL.read();
			if (!el.empty) leftTaps.push_back(el);
			auto er = delayR.read();
			if (!er.empty) rightTaps.push_back(er);
			if ((er.empty && el.empty) || (er.time > 2 && el.time > 2))
				break;
			delayL.write(el);
			delayR.write(er);
		}
	}
	else if (mode == Delay::PingPong)
	{
		delayL.seed(0.f, 1.f);
		delayR.seed(0.f, 1.f);

		while (true)
		{
			auto el = delayL.read();
			if (!el.empty) leftTaps.push_back(el);

			auto er = delayR.read();
			if (!er.empty) rightTaps.push_back(er);

			if ((er.empty && el.empty) || (er.time > 2 && el.time > 2))
				break;

			if (el.dry) el.gain *= rfactor;
			if (er.dry) er.gain *= lfactor;
			delayL.write(er);
			delayR.write(el);
		}
	}

	// apply haas offset
	if (mode != Delay::PingPong)
	{
		if (haas_width > 0.f)
		{
			for (int i = 0; i < rightTaps.size(); ++i)
			{
				if (i != 0) rightTaps[i].time += haas_width;
			}
		}
		else
		{
			for (int i = 0; i < leftTaps.size(); ++i)
			{
				if (i != 0) leftTaps[i].time -= haas_width;
			}
		}
	}

	// apply mix and pan
	float dryMix = mix < 0.5 ? 1.f : 1.f - (mix - 0.5f) * 2.f;
	float wetMix = mix > 0.5 ? 1.f : mix * 2.f;
	float rightPan = (pan_wet > 0.5 ? 1.f : pan_wet * 2.f);
	float leftPan = (pan_wet < 0.5 ? 1.f : 1.f - (pan_wet - 0.5f) * 2.f);
	for (auto& tap : leftTaps)
	{
		if (tap.time == 0.f)
			tap.gain *= dryMix * (pan_dry < 0.5 ? 1.f : 1.f - (pan_dry - 0.5f) * 2.f);
		else
			tap.gain *= wetMix * leftPan;
	}
	for (auto& tap : rightTaps)
	{
		if (tap.time == 0.f)
			tap.gain *= dryMix * (pan_dry > 0.5 ? 1.f : pan_dry * 2.f);
		else
			tap.gain *= wetMix * rightPan;
	}

	// paint
	float totalTime = std::max(std::max(0.25f, 
		leftTaps.size() ? leftTaps.back().time : 0.f), 
		rightTaps.size() ? rightTaps.back().time : 0.f
	);
	for (int i = 0; i < leftTaps.size(); ++i)
	{
		float time = leftTaps[i].time;
		float gain = leftTaps[i].gain;
		float w = 3.f;
		float h = b.getHeight() / 2.f * gain;
		g.setColour(time == 0.f ? Colour(COLOR_ACTIVE).darker(0.5f) : Colour(COLOR_ACTIVE));
		g.fillRect(b.getX() + (time / totalTime) * b.getWidth() - w / 2, b.getCentreY() - h, w, h);
	}

	for (int i = 0; i < rightTaps.size(); ++i)
	{
		float time = rightTaps[i].time;
		float gain = rightTaps[i].gain;
		float w = 3.f;
		float h = b.getHeight() / 2.f * gain;
		g.setColour(time == 0.f ? Colour(COLOR_ACTIVE).darker(0.5f) : Colour(COLOR_ACTIVE));
		g.fillRect(b.getX() + (time / totalTime) * b.getWidth() - w / 2, b.getCentreY(), w, h);
	}
}