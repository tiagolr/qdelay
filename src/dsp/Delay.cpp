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
    swingL.clear();
    swingR.clear();
    auto time = getTimeSamples();
    auto swing = audioProcessor.params.getRawParameterValue("swing")->load();
    timeL.reset((float)time[0]);
    timeR.reset((float)time[1]);
    feelSmooth.reset((float)getFeelOffset(time[0], time[1], swing));
    swingSmooth.reset(swing);
    haasL.clear();
    haasR.clear();
    haasSwingL.clear();
    haasSwingR.clear();
    diffusor.clear();
    diffusorSwing.clear();
}

void Delay::prepare(float _srate)
{
    srate = _srate;
    israte = 1.f / srate;
    auto time = getTimeSamples();
    float swing = audioProcessor.params.getRawParameterValue("swing")->load();
    timeL.setup(0.15f, srate);
    timeR.setup(0.15f, srate);
    feelSmooth.setup(0.15f, srate);
    swingSmooth.setup(0.15f, srate);
    feelSmooth.reset((float)getFeelOffset(time[0], time[1], swing));
    swingSmooth.reset(swing);
    timeL.reset((float)time[0]);
    timeR.reset((float)time[1]);
    delayL.resize((int)(srate * 6));
    delayR.resize((int)(srate * 6));
    swingL.resize((int)(srate * 6));
    swingR.resize((int)(srate * 6));
    haasL.resize((int)std::ceil(srate * MAX_HAAS / 1000.f));
    haasR.resize((int)std::ceil(srate * MAX_HAAS / 1000.f));
    haasSwingL.resize((int)std::ceil(srate * MAX_HAAS / 1000.f));
    haasSwingR.resize((int)std::ceil(srate * MAX_HAAS / 1000.f));
    delayL.clear();
    delayR.clear();
    swingL.clear();
    swingR.clear();
    predelayL.clear();
    predelayR.clear();
    haasL.clear();
    haasR.clear();
    haasSwingL.clear();
    haasSwingR.clear();
    diffusor.prepare(srate);
    diffusor.clear();
    diffusorSwing.prepare(srate);
    diffusorSwing.clear();
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

    return { std::max(1, tl), std::max(1, tr) };
}

int Delay::getFeelOffset(int tl, int tr, float swing)
{
    auto mode = (DelayMode)audioProcessor.params.getRawParameterValue("mode")->load();
    float feel = audioProcessor.params.getRawParameterValue("feel")->load();
    
    float secsbeat = (float)audioProcessor.secondsPerBeat;
    if (secsbeat == 0.f) secsbeat = 0.25f;
    int feelOffset = (int)std::round(secsbeat * MAX_FEEL_QN_OFFSET * feel * srate); // max 1/16 note offset
    int maxFeelOffset = mode == Tap ? tr : std::min(tl, tr);
    maxFeelOffset += (int)std::round(swing * 0.5f * maxFeelOffset);
    if (maxFeelOffset > 0) maxFeelOffset -= 1;
    if (maxFeelOffset < 0) maxFeelOffset += 1;
    return std::clamp(feelOffset, -maxFeelOffset, maxFeelOffset);
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

    float swing = audioProcessor.params.getRawParameterValue("swing")->load();
    float accent = audioProcessor.params.getRawParameterValue("accent")->load();
    float accentDelay = accent < 0 ? 1.f + accent * MAX_ACCENT : 1.f;
    float accentSwing = accent > 0 ? 1.f - accent * MAX_ACCENT : 1.f;

    float feelOffset = (float)getFeelOffset(time[0], time[1], swing);

    if (mode != PingPong) {
        float haasWidth = audioProcessor.params.getRawParameterValue("haas_width")->load();
        int haasLeft = haasWidth < 0.f ? (int)std::ceil(-haasWidth * MAX_HAAS * srate / 1000.f) : 0;
        int haasRight = haasWidth > 0.f ? (int)std::ceil(haasWidth * MAX_HAAS * srate / 1000.f) : 0;
        haasL.resize(haasLeft);
        haasR.resize(haasRight);
        haasSwingL.resize(haasLeft);
        haasSwingR.resize(haasRight);
    }

    if (diffamt > 0.f) {
        float diffsize = audioProcessor.params.getRawParameterValue("diff_size")->load();
        diffsize = (0.9f - 0.9f * diffsize);
        diffusor.setSize(diffsize);
    }

    // balance feedback between left and right delays
    float e = mode == Tap ? 1.f : (float)time[0] / (float)time[1];
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

    // resize buffers if they are too short for the delay time just in case
    int sizeL = (int)std::ceil(time[0]);
    int maxSizeL = (int)std::ceil(time[0] * 1.5); // 1.5 for swing
    int maxSizeR = (int)std::ceil(time[0] * 1.5);
    if (mode == DelayMode::Tap) { // tap delay, left time is the tap length (predelay), right is the delay time
        if (predelayL.size < sizeL)
        {
            predelayL.resize(sizeL);
            predelayR.resize(sizeL);
        }
        if (delayL.size < maxSizeR) delayL.resize(maxSizeR);
        if (delayR.size < maxSizeR) delayR.resize(maxSizeR);
        if (swingL.size < maxSizeR) swingL.resize(maxSizeR);
        if (swingR.size < maxSizeR) swingR.resize(maxSizeR);
    }
    else
    {
        if (delayL.size < maxSizeL) delayL.resize(maxSizeL);
        if (delayR.size < maxSizeR) delayR.resize(maxSizeR);
        if (swingL.size < maxSizeL) swingL.resize(maxSizeL);
        if (swingR.size < maxSizeR) swingR.resize(maxSizeR);
    }

    // modulation
    float modDepth = audioProcessor.params.getRawParameterValue("mod_depth")->load();
    float modRate = audioProcessor.params.getRawParameterValue("mod_rate")->load();
    float maxDepth = mode == Tap
        ? time[1] - std::fabs(swing) * 0.5f * time[1]
        : std::min(
            time[0] - std::fabs(swing) * 0.5f * time[0],
            time[1] - std::fabs(swing) * 0.5f * time[1]
        );
    maxDepth *= 0.5f;
    modDepth = modDepth * std::min(srate / 250.f, maxDepth);

    // Process samples
    for (int i = 0; i < nsamps; ++i)
    {
        float mod = 0.f;
        if (modDepth > 0.f) 
        { 
            modPhase += modRate * israte;
            if (modPhase > 1.f) modPhase -= 1.f;
            mod = std::sin(modPhase * MathConstants<float>::twoPi);
        }
        mod *= modDepth;

        // smoothed params
        auto timeLeft = timeL.process((float)time[0]);
        auto timeRight = timeR.process((float)time[1]);
        auto swingEff = swingSmooth.process(swing);
        auto feelEff = (int)std::round(feelSmooth.process(feelOffset));

        auto tap1L = mode == Tap ? timeRight : timeLeft;
        tap1L += swingEff * 0.5f * tap1L;
        auto tap1R = timeRight;
        tap1R += swingEff * 0.5f * tap1R;

        auto tap2L = mode == Tap ? timeRight : timeLeft;
        tap2L -= swingEff * 0.5f * tap2L;
        auto tap2R = timeRight;
        tap2R -= swingEff * 0.5f * tap2R;

        // two serial delay lines to support swing
        auto v0 = delayL.read3(tap1L + mod);
        auto v1 = delayR.read3(tap1R + mod);
        auto s0 = swingL.read3(tap2L + mod);
        auto s1 = swingR.read3(tap2R + mod);

        if (diffamt > 0) {
            diffusor.process(v0, v1, diffdry, diffwet);
            diffusorSwing.process(s0, s1, diffdry, diffwet);
        }

        if (mode == Normal)
        {
            delayL.writeOffset(left[i], feelEff, feelEff < 0);
            delayR.writeOffset(right[i], feelEff, feelEff < 0);
            delayL.write(s0 * feedbackL, feelEff >= 0);
            delayR.write(s1 * feedbackR, feelEff >= 0);
            swingL.write(v0 * feedbackL);
            swingR.write(v1 * feedbackR);
        }
        else if (mode == PingPong)
        {
            delayL.writeOffset(left[i] * lfactor, feelEff, feelEff < 0);
            delayR.writeOffset(right[i] * rfactor, feelEff, feelEff < 0);
            delayL.write(s1 * feedbackL, feelEff >= 0);
            delayR.write(s0 * feedbackR, feelEff >= 0);
            swingL.write(v1 * feedbackL);
            swingR.write(v0 * feedbackR);
        }
        else if (mode == Tap)
        {
            float preL = predelayL.read(timeLeft);
            float preR = predelayR.read(timeLeft);
            predelayL.write(left[i]);
            predelayR.write(right[i]);
            delayL.writeOffset(preL, feelEff, feelEff < 0);
            delayR.writeOffset(preR, feelEff, feelEff < 0);
            delayL.write(s0 * feedbackL, feelEff >= 0);
            delayR.write(s1 * feedbackR, feelEff >= 0);
            swingL.write(v0 * feedbackL);
            swingR.write(v1 * feedbackR);
        }

        // apply haas
        if (mode != PingPong) {
            haasL.write(v0);
            haasR.write(v1);
            v0 = haasL.read(haasL.size - 1);
            v1 = haasR.read(haasR.size - 1);
            haasSwingL.write(s0);
            haasSwingR.write(s1);
            s0 = haasSwingL.read(haasSwingL.size - 1);
            s1 = haasSwingR.read(haasSwingR.size - 1);
        }

        left[i] = v0 * accentDelay + s0 * accentSwing;
        right[i] = v1 * accentDelay + s1 * accentSwing;
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