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

	// build feedback replicas
	std::vector<float> leftTime;
	std::vector<float> leftGain;
	std::vector<float> rightTime;
	std::vector<float> rightGain;

	float dryMix = mix < 0.5 ? 1.f : 1.f - (mix - 0.5f) * 2.f;
	float wetMix = mix > 0.5 ? 1.f : mix * 2.f;
	leftTime.push_back(0.f);
	leftGain.push_back(1.f * dryMix * (pan_dry > 0.5 ? 1.f : pan_dry * 2.f));
	rightTime.push_back(0.f);
	rightGain.push_back(1.f * dryMix * (pan_dry < 0.5 ? 1.f : 1.f - (pan_dry - 0.5f) * 2.f));

	auto [timeL, timeR] = getDelayTimes(editor.audioProcessor.secondsPerBeat, sync_l, sync_r, rate_l, rate_r, rate_sync_l, rate_sync_r);
	float leftAmp = 1.f;
	float rightAmp = 1.f;
	float leftDelay = timeL;
	float rightDelay = timeR;

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

	float leftPan = (pan_wet < 0.5 ? 1.f : pan_wet * 2.f);
	float rightPan = (pan_wet > 0.5 ? 1.f : 1.f - (pan_wet - 0.5f) * 2.f);

	int j = 0;
	while ((leftAmp > 1e-3 || rightAmp > 1e-3) && (leftDelay < 2.f || rightDelay < 2.f))
	{
		if (mode == Delay::Normal)
		{
			leftTime.push_back(leftDelay);
			rightTime.push_back(rightDelay);
			leftGain.push_back(leftAmp * wetMix * leftPan);
			rightGain.push_back(rightAmp * wetMix * rightPan);

			leftDelay += timeL;
			rightDelay += timeR;
			leftAmp *= feedbackL;
			rightAmp *= feedbackR;
		}
		else if (mode == Delay::PingPong)
		{
			if (j == 0) {
				leftAmp = leftAmp * lfactor;
				rightAmp = rightAmp * rfactor;
				leftGain.push_back(leftAmp * wetMix * leftPan);
				rightGain.push_back(rightAmp * wetMix * rightPan);
				leftTime.push_back(leftDelay);
				rightTime.push_back(rightDelay);
			}
			else {
				std::swap(leftAmp, rightAmp);
				leftAmp *= feedbackL;
				rightAmp *= feedbackR;
				leftGain.push_back(leftAmp * wetMix * leftPan);
				rightGain.push_back(rightAmp * wetMix * rightPan);
				leftTime.push_back(leftDelay);
				rightTime.push_back(rightDelay);
			}

			leftDelay += timeL;
			rightDelay += timeR;
		}
		else
		{
			break;
		}

		j += 1;
	}

	// apply haas offset
	if (mode != Delay::PingPong)
	{
		if (haas_width > 0.f)
		{
			for (int i = 0; i < rightTime.size(); ++i)
			{
				if (i != 0) rightTime[i] += haas_width;
			}
		}
		else
		{
			for (int i = 0; i < leftTime.size(); ++i)
			{
				if (i != 0) leftTime[i] -= haas_width;
			}
		}
	}

	// paint
	float totalTime = std::max(std::max(0.25f, leftTime.back()), rightTime.back());
	for (int i = 0; i < leftTime.size(); ++i)
	{
		float time = leftTime[i];
		float gain = leftGain[i];
		float w = 2.f;
		float h = b.getHeight() / 2.f * gain;
		g.setColour(i == 0 ? Colour(COLOR_ACTIVE).darker(0.5f) : Colour(COLOR_ACTIVE));
		g.fillRect(b.getX() + (time / totalTime) * b.getWidth() - w / 2, b.getCentreY() - h, w, h);
	}

	for (int i = 0; i < rightTime.size(); ++i)
	{
		float time = rightTime[i];
		float gain = rightGain[i];
		float w = 2.f;
		float h = b.getHeight() / 2.f * gain;
		g.setColour(i == 0 ? Colour(COLOR_ACTIVE).darker(0.5f) : Colour(COLOR_ACTIVE));
		g.fillRect(b.getX() + (time / totalTime) * b.getWidth() - w / 2, b.getCentreY(), w, h);
	}
}