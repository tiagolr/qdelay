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
	inline static const std::array<Preset, 26> factoryPresets =
    {{
        { "-- Init --", "", BinaryData::init_xml, BinaryData::init_xmlSize },
        { "16 Zone", "Basic", BinaryData::basic16zone_xml, BinaryData::basic16zone_xmlSize },
        { "Dual Darker", "Basic", BinaryData::basicdualdarker_xml, BinaryData::basicdualdarker_xmlSize },
        { "Dub Like", "Basic", BinaryData::basicdublike_xml, BinaryData::basicdublike_xmlSize },
        { "Ping-Mid-Pong", "Basic", BinaryData::basicpingmidpong_xml, BinaryData::basicpingmidpong_xmlSize},
        { "Quarters", "Basic", BinaryData::basicquarters_xml, BinaryData::basicquarters_xmlSize },
        { "Smeared", "Basic", BinaryData::basicsmeared_xml, BinaryData::basicsmeared_xmlSize },
        { "Smooth Vibes", "Basic", BinaryData::basicsmoothvibes_xml, BinaryData::basicsmoothvibes_xmlSize },
        { "Swing Quarters", "Basic", BinaryData::basicswingquarters_xml, BinaryData::basicswingquarters_xmlSize },
        { "Taped", "Basic", BinaryData::basictaped_xml, BinaryData::basictaped_xmlSize },
        { "Tape Loop", "Basic", BinaryData::basictapeloop_xml, BinaryData::basictapeloop_xmlSize },
        { "Wider", "Basic", BinaryData::basicwider_xml, BinaryData::basicwider_xmlSize },
        { "Slight Groove", "Drums", BinaryData::drumsslightgroove_xml, BinaryData::drumsslightgroove_xmlSize },
        { "More Groove", "Drums", BinaryData::drumsmoregroove_xml, BinaryData::drumsmoregroove_xmlSize },
        { "Phatter", "Drums", BinaryData::drumsphatter_xml, BinaryData::drumsphatter_xmlSize },
        { "Punchy Echo", "Drums", BinaryData::drumspunchyecho_xml, BinaryData::drumspunchyecho_xmlSize },
        { "Shimmer Echo", "Drums", BinaryData::drumsshimmerecho_xml, BinaryData::drumsshimmerecho_xmlSize },
        { "Rush Beat", "Drums", BinaryData::drumsrushbeat_xml, BinaryData::drumsrushbeat_xmlSize },
        { "Snare Dub", "Drums", BinaryData::drumssnaredub_xml, BinaryData::drumssnaredub_xmlSize },
        { "Harmonizer", "FX", BinaryData::fxharmonizer_xml, BinaryData::fxharmonizer_xmlSize },
        { "Deharmonizer", "FX", BinaryData::fxdeharmonizer_xml, BinaryData::fxdeharmonizer_xmlSize },
        { "Diffused", "FX", BinaryData::fxdiffused_xml, BinaryData::fxdiffused_xmlSize },
        { "Duct Tape", "FX", BinaryData::fxducttape_xml, BinaryData::fxducttape_xmlSize },
        { "Metallizer", "FX", BinaryData::fxmetallizer_xml, BinaryData::fxmetallizer_xmlSize },
        { "Short Verb", "FX", BinaryData::fxshortreverb_xml, BinaryData::fxshortreverb_xmlSize },
        { "Octaver", "FX", BinaryData::fxoctaver_xml, BinaryData::fxoctaver_xmlSize }
    }};

	PresetMgr(QDelayAudioProcessor& proc, String _dir);
	void loadPrev();
	void loadNext();
	void loadInit();
	void loadFactory(int index);
	String exportPreset();
	void load(String name, int offset);

private:
	QDelayAudioProcessor& audioProcessor;
};