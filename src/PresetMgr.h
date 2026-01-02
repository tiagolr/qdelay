#pragma once
#include <JuceHeader.h>

class QDelayAudioProcessor;

class PresetMgr
{
public:
	struct Preset
	{
		String name;
		String category;
		const void* data;
    	int size;
	};

	String dir;
	inline static const std::array<Preset, 2> factoryPresets =
    {{
        { "-- Init --" },
        { "Snare" }
    }};

	PresetMgr(QDelayAudioProcessor& proc, String _dir);
	void loadPrev();
	void loadNext();
	void loadInit();
	String exportPreset();

private:
	QDelayAudioProcessor& audioProcessor;
	void load(String name, int offset);
	void loadFactory(int index);
};