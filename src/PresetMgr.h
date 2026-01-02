#pragma once
#include <JuceHeader.h>

class QDelayAudioProcessor;

class PresetMgr
{
public:
	struct Preset
	{
		String name;
		const void* data;
    	int size;
	};

	inline static const std::array<Preset, 2> factoryPresets =
    {{
        { "-- Init --" },
        { "Snare" }
    }};

	PresetMgr(QDelayAudioProcessor& proc, String _dir);
	void loadPrev();
	void loadNext();
	void loadInit();

private:
	QDelayAudioProcessor& audioProcessor;
	String dir;
	void load(String name, int offset);
	void loadFactory(int index);
};