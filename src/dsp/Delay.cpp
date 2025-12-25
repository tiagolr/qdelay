#include "Delay.h"
#include "../PluginProcessor.h"

Delay::Delay(QDelayAudioProcessor& p)
	: audioProcessor(p)
{
	audioProcessor.params.addParameterListener("link", this);
	audioProcessor.params.addParameterListener("sync_l", this);
	audioProcessor.params.addParameterListener("sync_r", this);
	audioProcessor.params.addParameterListener("rate_l", this);
	audioProcessor.params.addParameterListener("rate_sync_l", this);
	audioProcessor.params.addParameterListener("rate_r", this);
	audioProcessor.params.addParameterListener("rate_sync_r", this);
}

Delay::~Delay()
{
}

void Delay::clear()
{
    predelayL.clear();
    predelayR.clear();
    delayL.clear();
    delayR.clear();
    auto time = getTimeSamples();
    timeL.reset((float)time[0]);
    timeR.reset((float)time[1]);
}

void Delay::prepare(float _srate)
{
    srate = _srate;
    auto time = getTimeSamples();
    timeL.setup(0.5f, srate);
    timeR.setup(0.5f, srate);
    timeL.reset((float)time[0]);
    timeR.reset((float)time[1]);
    delayL.resize((int)(srate * 11));
    delayR.resize((int)(srate * 11));
    delayL.clear();
    delayR.clear();
    predelayL.clear();
    predelayR.clear();
    diffusor.prepare(srate);
}

std::array<int, 2> Delay::getTimeSamples(bool forceSync)
{
    auto getSamplesSync = [this](int rate, SyncMode sync)
        {
            auto secondsPerBeat = audioProcessor.secondsPerBeat;
            if (secondsPerBeat == 0.f)
                secondsPerBeat = 0.25f;

            float qn = 1.f;
            if (rate == 0) qn = 1.f / 16.f; // 1/64
            if (rate == 1) qn = 1.f / 8.f; // 1/32
            if (rate == 2) qn = 1.f / 4.f; // 1/16
            if (rate == 3) qn = 1.f / 2.f; // 1/8
            if (rate == 4) qn = 1.f / 1.f; // 1/4
            if (rate == 5) qn = 1.f * 2.f; // 1/2
            if (rate == 6) qn = 1.f * 4.f; // 1/1
            if (sync == Triplet) qn *= 2 / 3.f;
            if (sync == Dotted) qn *= 1.5f;

            return (int)std::ceil(qn * secondsPerBeat * srate);
        };

    auto syncL = forceSync ? Straight : (SyncMode)audioProcessor.params.getRawParameterValue("sync_l")->load();
    auto syncR = forceSync ? Straight : (SyncMode)audioProcessor.params.getRawParameterValue("sync_r")->load();

    auto tl = syncL == 0
        ? (int)(std::ceil(audioProcessor.params.getRawParameterValue("rate_l")->load() * srate))
        : getSamplesSync((int)audioProcessor.params.getRawParameterValue("rate_sync_l")->load(), syncL);

    auto tr = syncR == 0
        ? (int)(std::ceil(audioProcessor.params.getRawParameterValue("rate_r")->load() * srate))
        : getSamplesSync((int)audioProcessor.params.getRawParameterValue("rate_sync_r")->load(), syncR);

    return { tl, tr };
}

void Delay::processBlock(float* left, float* right, int nsamps)
{
    auto mode = (DelayMode)audioProcessor.params.getRawParameterValue("mode")->load();
    auto time = getTimeSamples();

    auto feedback = audioProcessor.params.getRawParameterValue("feedback")->load();
    auto pipoWidth = audioProcessor.params.getRawParameterValue("pipo_width")->load();
    float lfactor = pipoWidth > 0.f ? 1.f - pipoWidth : 1.f;
    float rfactor = pipoWidth < 0.f ? 1.f + pipoWidth : 1.f;

    float diffamt = audioProcessor.params.getRawParameterValue("diff_amt")->load();
    float diffdry = Utils::cosHalfPi()(diffamt);
    float diffwet = Utils::sinHalfPi()(diffamt);

    if (diffamt > 0.f) {
        float diffsize = audioProcessor.params.getRawParameterValue("diff_size")->load();
        diffsize = (0.9f - 0.9f * diffsize);
        diffusor.setSize(diffsize);
    }

    // balance feedback between left and right delays
    float e = (float)time[0] / (float)time[1];
    if (time[0] < time[1])
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

    if (mode == DelayMode::Tap) { // tap delay, first time is the tap length (predelay), second is the delay time
        if (predelayL.size < (int)time[0])
        {
            predelayL.resize((int)std::ceil(time[0]));
            predelayR.resize((int)std::ceil(time[0]));
        }
        if (delayL.size < (int)time[1])
            delayL.resize((int)std::ceil(time[1]));
        if (delayR.size < (int)time[1])
            delayR.resize((int)std::ceil(time[1]));
    }
    else
    {
        if (delayL.size < (int)time[0])
            delayL.resize((int)std::ceil(time[0]));
        if (delayR.size < (int)time[1])
            delayR.resize((int)std::ceil(time[1]));
    }

    for (int i = 0; i < nsamps; ++i)
    {
        auto timeLeft = timeL.process((float)time[0]);
        auto timeRight = timeR.process((float)time[1]);

        auto v0 = delayL.read3(mode == 2 ? timeRight : timeLeft);
        auto v1 = delayR.read3(timeRight);

        if (diffamt > 0) {
            diffusor.process(v0, v1, diffdry, diffwet);
        }

        if (mode == Normal)
        {
            delayL.write(left[i] + v0 * feedbackL);
            delayR.write(right[i] + v1 * feedbackR);
        }
        else if (mode == PingPong)
        {
            delayL.write(left[i] * lfactor + v1 * feedbackL);
            delayR.write(right[i] * rfactor + v0 * feedbackR);
        }
        else if (mode == Tap)
        {
            float preL = predelayL.read(timeLeft);
            float preR = predelayR.read(timeLeft);
            predelayL.write(left[i]);
            predelayR.write(right[i]);
            delayL.write(preL + v0 * feedback);
            delayR.write(preR + v1 * feedback);
        }

        left[i] = v0;
        right[i] = v1;
    }
}

void Delay::parameterChanged(const String& paramId, float value)
{
    // keep linked delay Left and Right rates in sync
    auto link = (bool)audioProcessor.params.getRawParameterValue("link")->load();

    if (paramId == "link" && (bool)value) {
        auto syncL = audioProcessor.params.getParameter("rate_sync_l");
        auto syncR = audioProcessor.params.getParameter("rate_sync_r");
        syncR->setValueNotifyingHost(syncL->getValue());
        auto rateL = audioProcessor.params.getParameter("rate_l");
        auto rateR = audioProcessor.params.getParameter("rate_r");
        rateR->setValueNotifyingHost(rateL->getValue());
        auto modeL = audioProcessor.params.getParameter("sync_l");
        auto modeR = audioProcessor.params.getParameter("sync_r");
        modeR->setValueNotifyingHost(modeL->getValue());
    }
    else if (paramId == "rate_sync_l") {
        if (link) {
            auto rateL = audioProcessor.params.getParameter("rate_sync_l");
            auto rateR = audioProcessor.params.getParameter("rate_sync_r");
            if (std::fabs(rateR->getValue() - rateL->getValue()) > 1e-5) {
                rateR->setValueNotifyingHost(rateL->getValue());
            }
        }
    }
    else if (paramId == "sync_l") {
        // if sync is turned off set rate from rate_sync
        // makes it easier for the user to offset times from project tempo
        if (value < 1.f) {
            auto time = getTimeSamples(true);
            auto param = audioProcessor.params.getParameter("rate_l");
            param->setValueNotifyingHost(param->convertTo0to1(time[0] / srate));
            if (link) {
                param = audioProcessor.params.getParameter("rate_r");
                param->setValueNotifyingHost(param->convertTo0to1(time[0] / srate));
            }
        }
        if (link) {
            auto syncR = (int)audioProcessor.params.getRawParameterValue("sync_r")->load();
            if (syncR != (int)value) {
                auto param = audioProcessor.params.getParameter("sync_r");
                param->setValueNotifyingHost(param->convertTo0to1(value));
            }
        }
    }
    else if (paramId == "sync_r") {
        // if sync is turned off set rate from rate_sync
        // makes it easier for the user to offset times from project tempo
        if (value < 1.f) {
            auto time = getTimeSamples(true);
            auto param = audioProcessor.params.getParameter("rate_r");
            param->setValueNotifyingHost(param->convertTo0to1(time[1] / srate));
            if (link) {
                param = audioProcessor.params.getParameter("rate_l");
                param->setValueNotifyingHost(param->convertTo0to1(time[1] / srate));
            }
        }
        if (link) {
            auto syncL = (int)audioProcessor.params.getRawParameterValue("sync_l")->load();
            if (syncL != (int)value) {
                auto param = audioProcessor.params.getParameter("sync_l");
                param->setValueNotifyingHost(param->convertTo0to1(value));
            }
        }
    }
    else if (paramId == "rate_sync_l") {
        if (link) {
            auto rateL = audioProcessor.params.getParameter("rate_sync_l");
            auto rateR = audioProcessor.params.getParameter("rate_sync_r");
            if (std::fabs(rateR->getValue() - rateL->getValue()) > 1e-5) {
                rateR->setValueNotifyingHost(rateL->getValue());
            }
        }
    }
    else if (paramId == "rate_l") {
        if (link) {
            auto rateL = audioProcessor.params.getParameter("rate_l");
            auto rateR = audioProcessor.params.getParameter("rate_r");
            if (std::fabs(rateR->getValue() - rateL->getValue()) > 1e-5) {
                rateR->setValueNotifyingHost(rateL->getValue());
            }
        }
    }
    else if (paramId == "rate_sync_r") {
        if (link) {
            auto rateL = audioProcessor.params.getParameter("rate_sync_l");
            auto rateR = audioProcessor.params.getParameter("rate_sync_r");
            if (std::fabs(rateR->getValue() - rateL->getValue()) > 1e-5) {
                rateL->setValueNotifyingHost(rateR->getValue());
            }
        }
    }
    else if (paramId == "rate_r") {
        if (link) {
            auto rateL = audioProcessor.params.getParameter("rate_l");
            auto rateR = audioProcessor.params.getParameter("rate_r");
            if (std::fabs(rateR->getValue() - rateL->getValue()) > 1e-5) {
                rateL->setValueNotifyingHost(rateR->getValue());
            }
        }
    }
}