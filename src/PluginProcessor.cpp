 // Copyright 2025 tilr

#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioProcessorValueTreeState::ParameterLayout QDelayAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterChoice>("mode", "Mode", StringArray{ "Normal", "Ping Pong", "Tap" }, 0));
    layout.add(std::make_unique<AudioParameterBool>("link", "Link", true));
    layout.add(std::make_unique<AudioParameterChoice>("sync_l", "Sync L", StringArray{ "RateHz", "Straight", "Triplet", "Dotted" }, 1));
    layout.add(std::make_unique<AudioParameterChoice>("sync_r", "Sync R", StringArray{ "RateHz", "Straight", "Triplet", "Dotted" }, 1));
    layout.add(std::make_unique<AudioParameterFloat>("rate_l", "Rate L", NormalisableRange<float>(0.0f, 5.f, 0.001f, 0.3f), .5f));
    layout.add(std::make_unique<AudioParameterFloat>("rate_r", "Rate R", NormalisableRange<float>(0.0f, 5.f, 0.001f, 0.3f), .5f));
    layout.add(std::make_unique<AudioParameterChoice>("rate_sync_l", "Rate Sync L", StringArray{"1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1"}, 3));
    layout.add(std::make_unique<AudioParameterChoice>("rate_sync_r", "Rate Sync R", StringArray{"1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1"}, 3));
    layout.add(std::make_unique<AudioParameterFloat>("mix", "Mix", 0.f, 1.f, 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>("feedback", "Feedback", 0.f, 1.f, 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>("pipo_width", "Pipo Width", -1.f, 1.f, 1.f));
    layout.add(std::make_unique<AudioParameterFloat>("haas_width", "Haas Width", NormalisableRange<float>(-1.f, 1.f, 0.0001f, 0.3f, true), 0.f));

    layout.add(std::make_unique<AudioParameterFloat>("pan_dry", "Pan Dry", 0.f, 1.f, 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>("pan_wet", "Pan Wet", 0.f, 1.f, 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>("stereo", "Stereo Width", 0.f, 2.f, 1.0f));

    layout.add(std::make_unique<AudioParameterFloat>("swing", "Swing", -1.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("feel", "Feel", -1.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("accent", "Accent", -1.f, 1.f, 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>("diff_amt", "Diffusion Amt", 0.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("diff_size", "Diffusion Size", 0.f, 1.f, 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>("mod_depth", "Modulation Amt", 0.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("mod_rate", "Modulation Rate", 0.f, 10.f, 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>("dist_pre", "Saturation Pre", 0.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("dist_post", "Saturation Post", 0.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterChoice>("dist_mode", "Saturation Mode", StringArray{ "Tape", "Tube" }, 0));
    layout.add(std::make_unique<AudioParameterFloat>("dist_drive", "Saturation Drive", 0.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("dist_trim", "Saturation Trim", -24.f, 24.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("dist_color", "Saturation Color", 0.f, 1.f, 0.0f));
    layout.add(std::make_unique<AudioParameterFloat>("dist_bias", "Saturation Bias", 0.f, 1.f, 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>("tape_amt", "Tape Amount", 0.f, 1.f, 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>("pitch_shift", "Pitch Shift", NormalisableRange<float>(-12.f, 12.f, 0.01f), 0.0f));
    layout.add(std::make_unique<AudioParameterChoice>("pitch_mode", "Pitch Mode", StringArray{ "Drums", "General", "Smooth" }, 1));
    layout.add(std::make_unique<AudioParameterChoice>("pitch_path", "Pitch Path", StringArray{ "Feedback", "Post" }, 0));
    layout.add(std::make_unique<AudioParameterFloat>("pitch_mix", "Pitch Mix", 0.f, 1.f, 1.0f));

    layout.add(std::make_unique<AudioParameterFloat>("duck_thres", "Duck Threshold", NormalisableRange<float>(0.0f, 1.f-0.001f, 0.001f, 3.f), 1.f-0.001f));
    layout.add(std::make_unique<AudioParameterFloat>("duck_amt", "Duck Amount", NormalisableRange<float>(0.f, 1.f - 0.001f, 0.001f, 3.f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("duck_atk", "Duck Attack", NormalisableRange<float>(0.01f, 200.0f, 0.01f, 0.75f), 5.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("duck_hld", "Duck Hold", NormalisableRange<float>(0.0f, 1000.0f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("duck_rel", "Duck Release", NormalisableRange<float>(10.f, 5000.0f, 1.f, 0.5f), 500.f));

    auto getEQBandFreq = [](int band)
        {
            return 20.f * 2.f * std::pow(10000.f / 20.f / 2.f, band / (float)(EQ_BANDS - 1));
        };

    // Post EQ params
    for (int i = 0; i < EQ_BANDS; ++i) {
        auto paramPrefix = "inputeq_band" + String(i + 1);
        auto namePrefix = "Input EQ Band" + String(i + 1);
        layout.add(std::make_unique<AudioParameterChoice>(paramPrefix + "_mode", namePrefix + " Mode", StringArray{ "Filter", "EQ", "Filter2"}, 1));
        layout.add(std::make_unique<AudioParameterFloat>(paramPrefix + "_freq", namePrefix + " Freq", NormalisableRange<float>(20.f, 20000.f, 1.f, 0.3f), getEQBandFreq(i)));
        layout.add(std::make_unique<AudioParameterFloat>(paramPrefix + "_q", namePrefix + " Q", 0.2f, 8.f, 0.707f));
        layout.add(std::make_unique<AudioParameterFloat>(paramPrefix + "_gain", namePrefix + " Gain", -EQ_MAX_GAIN, EQ_MAX_GAIN, 0.f));
        layout.add(std::make_unique<AudioParameterBool>(paramPrefix + "_bypass", namePrefix + " Bypass", false));
    }

    // Decay EQ params
    for (int i = 0; i < EQ_BANDS; ++i) {
        auto paramPrefix = "decayeq_band" + String(i + 1);
        auto namePrefix = "Decay EQ Band" + String(i + 1);
        layout.add(std::make_unique<AudioParameterChoice>(paramPrefix + "_mode", namePrefix + " Mode", StringArray{ "Filter", "EQ", "Filter2"}, 1));
        layout.add(std::make_unique<AudioParameterFloat>(paramPrefix + "_freq", namePrefix + " Freq", NormalisableRange<float>(20.f, 20000.f, 1.f, 0.3f), getEQBandFreq(i)));
        layout.add(std::make_unique<AudioParameterFloat>(paramPrefix + "_q", namePrefix + " Q", 0.2f, 8.f, 0.707f));
        layout.add(std::make_unique<AudioParameterFloat>(paramPrefix + "_gain", namePrefix + " Gain", -EQ_MAX_GAIN, EQ_MAX_GAIN, 0.f));
        layout.add(std::make_unique<AudioParameterBool>(paramPrefix + "_bypass", namePrefix + " Bypass", false));
    }

    return layout;
}

QDelayAudioProcessor::QDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
         .withInput("Input", juce::AudioChannelSet::stereo(), true)
         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
     )
    , settings{}
    , params(*this, &undoManager, "PARAMETERS", createParameterLayout())
#endif
{
    srand(static_cast<unsigned int>(time(nullptr))); // seed random generator
    juce::PropertiesFile::Options options{};
    options.applicationName = ProjectInfo::projectName;
    options.filenameSuffix = ".settings";
#if defined(JUCE_LINUX) || defined(JUCE_BSD)
    options.folderName = "~/.config/QDelay";
#elif defined(JUCE_MAC) || defined(JUCE_IOS)
    options.folderName = "QDelay";
#endif
    options.osxLibrarySubFolder = "Application Support";
    options.storageFormat = PropertiesFile::storeAsXML;
    settings.setStorageParameters(options);

    for (auto* param : getParameters()) {
        param->addListener(this);
    }

    loadSettings();

    delay = std::make_unique<Delay>(*this);
    dist = std::make_unique<Distortion>(*this);
    diffusor = std::make_unique<Diffusor>();
    pitcher = std::make_unique<Pitcher>();
}

QDelayAudioProcessor::~QDelayAudioProcessor()
{
}

void QDelayAudioProcessor::parameterValueChanged (int parameterIndex, float newValue)
{
    (void)newValue;
    (void)parameterIndex;
    paramChanged = true;
}

void QDelayAudioProcessor::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
    (void)parameterIndex;
    (void)gestureIsStarting;
}

std::vector<SVF::EQBand> QDelayAudioProcessor::getEqualizer(SVF::EQType type) const
{
    std::vector<SVF::EQBand> bands;

    String pre = type == 0 ? "inputeq_" : "decayeq_";
    for (int i = 0; i < EQ_BANDS; i++)
    {
        SVF::EQBand band{};
        band.mode = SVF::PK;

        auto filterOrShelf = (int)params.getRawParameterValue(pre + "band" + String(i + 1) + "_mode")->load();
        if (i == 0 && filterOrShelf == 0) band.mode = SVF::HP;
        else if (i == 0 && filterOrShelf == 1) band.mode = SVF::LS;
        else if (i == 0 && filterOrShelf == 2) band.mode = SVF::HP6;
        else if (i == EQ_BANDS - 1 && filterOrShelf == 0) band.mode = SVF::LP;
        else if (i == EQ_BANDS - 1 && filterOrShelf == 1) band.mode = SVF::HS;
        else if (i == EQ_BANDS - 1 && filterOrShelf == 2) band.mode = SVF::LP6;
        else if (filterOrShelf == 0) band.mode = SVF::BP;

        band.freq = params.getRawParameterValue(pre + "band" + String(i + 1) + "_freq")->load();
        float gain = params.getRawParameterValue(pre + "band" + String(i + 1) + "_gain")->load();
        band.gain = std::exp(gain * DB2LOG);
        band.q = params.getRawParameterValue(pre + "band" + String(i + 1) + "_q")->load();
        bool bypass = params.getRawParameterValue(pre + "band" + String(i + 1) + "_bypass")->load();

        if (bypass || (filterOrShelf == 1 && std::fabs(gain) < 1e-4f))
            band.mode = SVF::Off;

        bands.push_back(band);
    }

    return bands;
}

void QDelayAudioProcessor::loadSettings ()
{
    settings.closeFiles(); // FIX files changed by other plugin instances not loading
    if (auto* file = settings.getUserSettings())
    {
        scale = (float)file->getDoubleValue("scale", 1.0f);
    }
}

void QDelayAudioProcessor::saveSettings ()
{
    settings.closeFiles(); // FIX files changed by other plugin instances not loading
    if (auto* file = settings.getUserSettings())
    {
        file->setValue("scale", scale);
    }
    settings.saveIfNeeded();
}

void QDelayAudioProcessor::setScale(float s)
{
    scale = s;
    saveSettings();
}

//==============================================================================
const juce::String QDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool QDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool QDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double QDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0; // TODO
}

int QDelayAudioProcessor::getNumPrograms()
{
    return 0;
}

int QDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QDelayAudioProcessor::setCurrentProgram (int index)
{
    (void)index;
}

void QDelayAudioProcessor::loadProgram (int index)
{
    (void)index;
}

const juce::String QDelayAudioProcessor::getProgramName (int index)
{
    (void)index;
    return {};
}

void QDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    (void)index;
    (void)newName;
}

//==============================================================================
void QDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    wetBuffer.setSize (2, samplesPerBlock);
    srate = sampleRate;
    delay->prepare((float)srate);
    dist->prepare((float)srate);
    diffusor->prepare((float)srate);
    pitcher->init((Pitcher::WindowMode)params.getRawParameterValue("pitch_mode")->load());
    onSlider();
    sendChangeMessage();
}

void QDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool QDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void QDelayAudioProcessor::onSlider()
{
    float thresh = 1.f - params.getRawParameterValue("duck_thres")->load();
    float attack = params.getRawParameterValue("duck_atk")->load();
    float hold = params.getRawParameterValue("duck_hld")->load();
    float release = params.getRawParameterValue("duck_rel")->load();
    follower.prepare((float)srate, thresh, false, attack, hold, release, true);

    // prepare input EQ
    eqBands = getEqualizer(SVF::EQType::ParamEQ);
    for (int i = 0; i < EQ_BANDS; ++i) 
    {
        if (eqL.size() < EQ_BANDS) 
        {
            eqL.push_back({});
            eqR.push_back({});
            eqL[i].mode = SVF::Off;
        }

        // only bands that change mode need to be reset
        // otherwise the frequency,gain and q are interpolated during SVF::processBlock
        auto rate = (float)srate / oversampling;
        bool rateChanged = eqL[i].srate != rate;
        auto& band = eqBands[i];
        if (band.mode != eqL[i].mode || rateChanged)
        {
            auto mode = band.mode; 
            if (mode == SVF::LP) eqL[i].lp(rate, band.freq, band.q);
            else if (mode == SVF::LP6) eqL[i].lp6(rate, band.freq);
            else if (mode == SVF::LS) eqL[i].ls(rate, band.freq, band.q, band.gain);
            else if (mode == SVF::HP) eqL[i].hp(rate, band.freq, band.q);
            else if (mode == SVF::HP6) eqL[i].hp6(rate, band.freq);
            else if (mode == SVF::HS) eqL[i].hs(rate, band.freq, band.q, band.gain);
            else if (mode == SVF::PK) eqL[i].pk(rate, band.freq, band.q, band.gain);
            else if (mode == SVF::BP) eqL[i].bp(rate, band.freq, band.q);
            else eqL[i].mode = SVF::Off;
        }

        eqR[i].copyFrom(eqL[i]);
    }

    // prepare feedback EQ
    delay->setEqualizer(getEqualizer(SVF::EQType::DecayEQ));
    delay->onSlider();

    // update distortion
    dist->onSlider();

    // diffusor
    float diffsize = params.getRawParameterValue("diff_size")->load();
    diffusor->setSize(diffsize);

    // pitch shifter
    Pitcher::WindowMode pitchMode = (Pitcher::WindowMode)params.getRawParameterValue("pitch_mode")->load();
    if (pitcher->mode != pitchMode) {
        pitcher->init(pitchMode);
        delay->pitcher->init(pitchMode);
        delay->pitcherSwing->init(pitchMode);
    }
    pitcherPath = (int)params.getRawParameterValue("pitch_path")->load();
    float pitchSemis = params.getRawParameterValue("pitch_shift")->load();
    pitcherSpeed = pitcher->getSpeedFromSemis(pitchSemis);
}

bool QDelayAudioProcessor::supportsDoublePrecisionProcessing() const
{
    return false;
}

void QDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    (void)midiMessages;
    juce::ScopedNoDenormals disableDenormals;

    // Get playhead info
    if (auto* phead = getPlayHead())
    {
        if (auto pos = phead->getPosition())
        {
            if (auto ppq = pos->getPpqPosition())
                ppqPosition = *ppq;
            if (auto tempo = pos->getBpm())
            {
                beatsPerSecond = *tempo / 60.0;
                beatsPerSample = *tempo / (60.0 * srate);
                samplesPerBeat = (int)((60.0 / *tempo) * srate);
                secondsPerBeat = 60.0 / *tempo;
            }
            if (auto timeSig = pos->getTimeSignature())
            {
                secondsPerBar = secondsPerBeat * (*timeSig).numerator * (4.0 / (*timeSig).denominator);
            }
            else
            {
                secondsPerBar = beatsPerSecond * 4;
            }
            auto play = pos->getIsPlaying();
            if (!playing && play) // onplay()
            {
                delay->clear();
                dist->clear();
                diffusor->clear();
            }
            playing = play;
        }
    }

    int audioOutputs = getTotalNumOutputChannels();
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    if (!numChannels || !audioOutputs || !numSamples)
        return;

    if (paramChanged)
    {
        paramChanged = false;
        onSlider();
    }

    // prepare wet buffer by copying the dry signal into it
    wetBuffer.setSize (2, numSamples, false, false, true);
    if (numChannels == 1)
    {
        wetBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
        wetBuffer.copyFrom (1, 0, buffer, 0, 0, numSamples);
    }
    else
    {
        wetBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
        wetBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples);
    }

    // process wetBuffer (input) through input EQ
    for (int i = 0; i < EQ_BANDS; ++i)
    {
        auto& svfL = eqL[i];
        auto& svfR = eqR[i];
        auto& band = eqBands[i];
        svfL.processBlock(wetBuffer.getWritePointer(0), numSamples, 0, numSamples, band.freq, band.q, band.gain);
        svfR.processBlock(wetBuffer.getWritePointer(1), numSamples, 1, numSamples, band.freq, band.q, band.gain);
    }

    float diffamt = params.getRawParameterValue("diff_amt")->load();
    if (diffamt > 0.f) {
        float diffdry = Utils::cosHalfPi()(diffamt);
        float diffwet = Utils::sinHalfPi()(diffamt);
        diffusor->processBlock(
            wetBuffer.getWritePointer(0),
            wetBuffer.getWritePointer(1),
            numSamples, diffdry, diffwet
        );
    }

    // process delay
    delay->processBlock(
        wetBuffer.getWritePointer(0), 
        wetBuffer.getWritePointer(1),
        numSamples
    );

    // process pitch shift
    if (std::fabs(pitcherSpeed) > 1e-6 && pitcherPath == 1) {
        float* wetl = wetBuffer.getWritePointer(0);
        float* wetr = wetBuffer.getWritePointer(1);
        float pitchMix = params.getRawParameterValue("pitch_mix")->load();
        auto pitchDry = Utils::cosHalfPi()(pitchMix);
        auto pitchWet = Utils::sinHalfPi()(pitchMix);
        for (int i = 0; i < numSamples; ++i)
        {
            pitcher->setSpeed(pitcherSpeed);
            pitcher->update(wetl[i], wetr[i]);
            wetl[i] = wetl[i] * pitchDry + pitcher->outL * pitchWet;
            wetr[i] = wetr[i] * pitchDry + pitcher->outR * pitchWet;
        }
    }

    // process post distortion
    auto dist_post = params.getRawParameterValue("dist_post")->load();
    if (dist_post > 0.f) 
    {
        auto distDry = Utils::cosHalfPi()(dist_post);
        auto distWet = Utils::sinHalfPi()(dist_post);
        dist->processBlock(wetBuffer.getWritePointer(0), wetBuffer.getWritePointer(1), numSamples, distDry, distWet);
    }

    auto mix = params.getRawParameterValue("mix")->load();
    auto drymix = mix <= 0.5f ? 1.f : 1.f - (mix - 0.5f) * 2.f;
    auto wetmix = mix <= 0.5f ? mix * 2.f : 1.f;

    float panCompensation = std::sqrt(2.f); // keep amplitude at 0db when centered, +3dB when hard panned
    auto panDry = params.getRawParameterValue("pan_dry")->load();
    auto panDryL = Utils::cosHalfPi()(panDry) * panCompensation;
    auto panDryR = Utils::sinHalfPi()(panDry) * panCompensation;

    auto panWet = params.getRawParameterValue("pan_wet")->load();
    auto panWetL = Utils::cosHalfPi()(panWet) * panCompensation;
    auto panWetR = Utils::sinHalfPi()(panWet) * panCompensation;

    // apply ducking
    float duck = params.getRawParameterValue("duck_amt")->load();
    if (duck > 0.f)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float env = follower.process(
                buffer.getSample(0, i),
                buffer.getSample(numChannels > 1 ? 1 : 0, i)
            );

            float gain = 1.f - env * duck;
            wetBuffer.setSample(0, i, wetBuffer.getSample(0, i) * gain);
            wetBuffer.setSample(1, i, wetBuffer.getSample(1, i) * gain);
        }
    }
    else
    {
        follower.clear();
    }

    // apply mix + pan
    buffer.applyGain(0, 0, numSamples, drymix * panDryL);
    if (numChannels > 1)
        buffer.applyGain(1, 0, numSamples, drymix * panDryR);
    wetBuffer.applyGain(0, 0, numSamples, wetmix * panWetL);
    wetBuffer.applyGain(1, 0, numSamples, wetmix * panWetR);

    // apply stereo width
    auto stereo = params.getRawParameterValue("stereo")->load();
    if (stereo != 1.f) {
        auto lchan = wetBuffer.getWritePointer(0);
        auto rchan = wetBuffer.getWritePointer(1);
        float norm = 1.0f / (1.0f + stereo);
        for (int sample = 0; sample < numSamples; ++sample) {
            auto lsamp = lchan[sample];
            auto rsamp = rchan[sample];
            auto mid = (lsamp + rsamp) * 0.5f;
            auto side = (lsamp - rsamp) * 0.5f;
            lchan[sample] = (mid + side * stereo) * norm;
            rchan[sample] = (mid - side * stereo) * norm;
        }
    }

    // sum wet and dry signals
    buffer.addFrom(0, 0, wetBuffer.getReadPointer(0), numSamples);
    if (numChannels > 1)
        buffer.addFrom(1, 0, wetBuffer.getReadPointer(1), numSamples);

    // prepare FFT buffer and RMS
    auto ch0 = buffer.getReadPointer(0);
    auto ch1 = buffer.getReadPointer(numChannels > 1 ? 1 : 0);
    for (int i = 0; i < numSamples; ++i) {
        eqFFTBuffer[eqFFTWriteIndex++] = 0.5f * (ch0[i] + ch1[i]);
        eqFFTWriteIndex %= eqFFTBuffer.size();
    }
    eqFFTReady.store(true, std::memory_order_release);
    rmsLeft.store(buffer.getMagnitude(0, 0, numSamples));
    rmsRight.store(buffer.getMagnitude(numChannels > 1 ? 1 : 0, 0, numSamples));
}

//==============================================================================
bool QDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* QDelayAudioProcessor::createEditor()
{
    return new QDelayAudioProcessorEditor (*this);
}

//==============================================================================
void QDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = ValueTree("PluginState");
    state.appendChild(params.copyState(), nullptr);
    state.setProperty("version", PROJECT_VERSION, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void QDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement>xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState == nullptr) // Fallback to plain text parsing, used for loading programs
    {
        auto xmlString = juce::String::fromUTF8(static_cast<const char*>(data), sizeInBytes);
        xmlState = juce::parseXML(xmlString);
    }

    if (!xmlState) return;
    auto state = ValueTree::fromXml (*xmlState);
    if (!state.isValid()) return;

    params.replaceState(state.getChild(0));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QDelayAudioProcessor();
}
