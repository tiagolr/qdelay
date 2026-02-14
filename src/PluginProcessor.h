/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include "Globals.h"
#include "dsp/Delay.h"
#include "dsp/SVF.h"
#include "dsp/Follower.h"
#include "dsp/Distortion.h"
#include "dsp/Crusher.h"
#include "dsp/Diffusor.h"
#include "dsp/Pitcher.h"
#include "dsp/Flutter.h"
#include "dsp/Wow.h"
#include "dsp/DelayLine.h"
#include "dsp/Phaser.h"
#include "PresetMgr.h"

using namespace globals;

//==============================================================================
/**
*/
class QDelayAudioProcessor
    : public AudioProcessor
    , public AudioProcessorParameter::Listener
    , public ChangeBroadcaster
{
public:
    enum RightTab
    {
		EQ,
		Saturation,
		Tape,
		LoFi,
    };

    std::unique_ptr<Delay> delay;
    std::unique_ptr<Distortion> distPre;
    std::unique_ptr<Crusher> crushPre;
    std::unique_ptr<Phaser> phaser;
    int distPrePath = 0; // Pre or feedback distortion/saturation
    std::unique_ptr<Distortion> distPost;
    std::unique_ptr<Crusher> crushPost;
    std::unique_ptr<PresetMgr> presetmgr;
    float dist_pre = 0.f;
    float dist_post = 0.f;
    std::unique_ptr<juce::dsp::Oversampling<float>> distPreOversampler;
    std::unique_ptr<juce::dsp::Oversampling<float>> distPostOversampler;
    std::unique_ptr<Diffusor> diffusor;
    int diffPath = 0; // pre or post delay diffusion
    std::unique_ptr<Pitcher> pitcher;
    float pitcherSpeed = 0.f;
    int pitcherPath = 0; // feedback or post delay signal
    int phaserPath = 0; // feedback or post delay signal
    AudioBuffer<float> wetBuffer;
    AudioBuffer<float> distPreBuffer;
    AudioBuffer<float> distPostBuffer;
    Follower follower;
    int eqPath = 0; // Input or output eq
    std::vector<SVF::EQBand> eqBands;
    std::vector<SVF> eqL;
    std::vector<SVF> eqR;

    // wow and flutter
    float tapeAmt = 0.f;
    int tapeFadeSamps = 0;
    int tapeFadeSize = 500;
    bool tapeFadeIn = false;
    std::unique_ptr<Flutter> flutter;
    std::unique_ptr<Wow> wow;
    std::unique_ptr<DelayLine> wowflut_l; // wow + flutter delay
    std::unique_ptr<DelayLine> wowflut_r;


    // Plugin settings
    float scale = 1.0f; // UI scale factor

    // PlayHead state
    bool playing = false;
    double ppqPosition = 0.0;
    double beatsPerSample = 0.00005;
    double beatsPerSecond = 1.0;
    int samplesPerBeat = 44100;
    double secondsPerBeat = 0.1;
    double srate = 44100.0;
    double secondsPerBar = 1.0;
    double timeInSeconds = 0.f;

    // UI State
    String presetName = "-- Init --";
    int delayTab = 0; // mix, pan, pattern
    int eqTab = 0; // input, feedback
    RightTab rightTab = RightTab::EQ;
    size_t eqFFTWriteIndex = 0;
    std::array<float, (1 << EQ_FFT_ORDER) * 2> eqFFTBuffer;
    std::atomic<bool> eqFFTReady = false;
    std::atomic<float> rmsLeft = 0.f;
    std::atomic<float> rmsRight = 0.f;
    std::atomic<float> duckEnv = 0.f;
    bool drawWaveform = true;
    bool isLoadingState = false;

    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================
    QDelayAudioProcessor();
    ~QDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
    bool supportsDoublePrecisionProcessing() const override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    //==============================================================================
    void loadSettings();
    void saveSettings();
    void setScale(float scl);
    void onSlider ();
    void clearAll();
    std::vector<SVF::EQBand> getEqualizer(SVF::EQType type) const;

    //==============================================================================
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    void loadProgram(int index);
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //=========================================================

    AudioProcessorValueTreeState params;
    UndoManager undoManager;

private:
    bool paramChanged = false; // flag that triggers on any param change
    ApplicationProperties settings;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QDelayAudioProcessor)
};
