#include "PresetMgr.h"
#include "PluginProcessor.h"


PresetMgr::PresetMgr(QDelayAudioProcessor& proc, String _dir)
	: audioProcessor(proc)
	, dir(_dir)
{
	if (!File(dir).exists())
    	File(dir).createDirectory();
}

void PresetMgr::loadNext()
{
	load("", 1);
}

void PresetMgr::loadPrev()
{
	load("", -1);
}

void PresetMgr::load(String name, int offset)
{
	File parent = File(dir);
	juce::Array<juce::File> userPresets;
	parent.findChildFiles(userPresets, File::findFiles, false, "*.xml");
	if (name.isEmpty())
		name = audioProcessor.presetName;

	for (int i = 0; i < userPresets.size(); ++i)
	{
		if (userPresets[i].getFileNameWithoutExtension() == name)
		{
			i += offset;
			if (i >= (int)userPresets.size()) i = 0;
			if (i < 0) i = (int)(userPresets.size() - 1);
			// Load user preset
			std::unique_ptr<juce::XmlElement> xml;
    		xml = juce::XmlDocument::parse(userPresets[i]); // load preset file

    		if (xml == nullptr) {
    		    loadInit();
    		    return;
    		}
			try {
    		    juce::MemoryBlock state;
    		    audioProcessor.copyXmlToBinary(*xml, state);
    		    audioProcessor.setStateInformation(state.getData(), (int)state.getSize());
    		}
    		catch (...) {
    		    loadInit();
    		    return;
    		}
			return;
		}
	}

	// user preset not found, load factory preset instead
	for (int i = 0; i < factoryPresets.size(); ++i) {
		if (factoryPresets[i].name == name)
		{
			i += offset;
			if (i >= (int)factoryPresets.size()) i = 0;
			if (i < 0) i = (int)(factoryPresets.size() - 1);
			loadFactory(i);
			return;
		}
	}

	loadInit();
}

void PresetMgr::loadInit()
{
	loadFactory(0);
}

void PresetMgr::loadFactory(int index)
{
	auto& preset = factoryPresets[index];
	audioProcessor.setStateInformation(preset.data, preset.size);
}

String PresetMgr::exportPreset()
{
	juce::MemoryBlock state;
	audioProcessor.getStateInformation(state);
	std::unique_ptr<juce::XmlElement> xml(
		audioProcessor.getXmlFromBinary(state.getData(), (int)state.getSize())
	);
	if (xml == nullptr) {
		return "";
	}
	return xml->toString();
}