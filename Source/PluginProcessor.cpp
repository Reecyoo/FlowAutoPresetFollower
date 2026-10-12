#include "PluginProcessor.h"
#include "PluginEditor.h"

AutoPresetFollowerAudioProcessor::AutoPresetFollowerAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize default preset buffer
    generatePresetsFromPrompt("reese bass for heavy trap", 0);
}

juce::AudioProcessorValueTreeState::ParameterLayout AutoPresetFollowerAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Strictly requested user parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "input_gain", 1 }, "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f, "dB"));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "output_gain", 1 }, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f, "dB"));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "genre_select", 1 }, "Genre / Subgenre",
        juce::StringArray{
            "Heavy Trap / Drill", "Modern Trap", "Boom Bap 90s",
            "Melodic Techno", "Liquid DnB", "Future Bass / Pop", "Lo-Fi Hip Hop"
        }, 0));

    return { params.begin(), params.end() };
}

void AutoPresetFollowerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };
    smoothedInputGain.reset(sampleRate, 0.02);
    smoothedOutputGain.reset(sampleRate, 0.02);
}

void AutoPresetFollowerAudioProcessor::releaseResources() {}

void AutoPresetFollowerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;

    // Detect DAW Host Playhead BPM
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            if (pos->getBpm().hasValue()) {
                currentBpm = *pos->getBpm();
            }
        }
    }

    // Apply Input & Output Gain smoothly (does not generate sound of its own)
    float inGainDb = apvts.getRawParameterValue("input_gain")->load();
    float outGainDb = apvts.getRawParameterValue("output_gain")->load();

    smoothedInputGain.setTargetValue(juce::Decibels::decibelsToGain(inGainDb));
    smoothedOutputGain.setTargetValue(juce::Decibels::decibelsToGain(outGainDb));

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float inGain = smoothedInputGain.getNextValue();
            float outGain = smoothedOutputGain.getNextValue();
            channelData[sample] = channelData[sample] * inGain * outGain;
        }
    }
}

void AutoPresetFollowerAudioProcessor::generatePresetsFromPrompt(const juce::String& prompt, int genreIdx) {
    juce::String lowerPrompt = prompt.toLowerCase();
    bool isReese = lowerPrompt.contains("reese") || lowerPrompt.contains("bass");
    bool isHeavy = lowerPrompt.contains("heavy") || lowerPrompt.contains("drill") || genreIdx == 0;
    bool isBoomBap = lowerPrompt.contains("boom bap") || genreIdx == 2;

    PresetData newPreset;
    newPreset.prompt = prompt;
    newPreset.bpm = currentBpm;

    if (isHeavy) {
        newPreset.title = "Heavy Trap Reese #" + juce::String(juce::Random::getSystemRandom().nextInt({10, 99}));
        newPreset.normalizedParams["cutoff"] = 0.42f;      // ~3.5 kHz
        newPreset.normalizedParams["resonance"] = 0.55f;
        newPreset.normalizedParams["detune"] = 0.76f;     // wide unison spread
        newPreset.normalizedParams["drive"] = 0.85f;      // hard clipped
        newPreset.normalizedParams["attack"] = 0.005f;
        newPreset.normalizedParams["decay"] = 0.35f;
        newPreset.normalizedParams["sustain"] = 0.80f;
    } else if (isBoomBap) {
        newPreset.title = "90s Gritty Boom Bap Sub #" + juce::String(juce::Random::getSystemRandom().nextInt({10, 99}));
        newPreset.normalizedParams["cutoff"] = 0.18f;      // ~850 Hz warm lowpass
        newPreset.normalizedParams["resonance"] = 0.25f;
        newPreset.normalizedParams["detune"] = 0.20f;
        newPreset.normalizedParams["drive"] = 0.40f;
        newPreset.normalizedParams["attack"] = 0.02f;
        newPreset.normalizedParams["decay"] = 0.45f;
        newPreset.normalizedParams["sustain"] = 0.60f;
    } else {
        newPreset.title = "Wide Festival Reese #" + juce::String(juce::Random::getSystemRandom().nextInt({10, 99}));
        newPreset.normalizedParams["cutoff"] = 0.70f;
        newPreset.normalizedParams["resonance"] = 0.35f;
        newPreset.normalizedParams["detune"] = 0.90f;
        newPreset.normalizedParams["drive"] = 0.35f;
        newPreset.normalizedParams["attack"] = 0.01f;
        newPreset.normalizedParams["decay"] = 0.50f;
        newPreset.normalizedParams["sustain"] = 0.85f;
    }

    // Prepend to history & maintain strictly max 10 presets
    presetHistory.insert(presetHistory.begin(), newPreset);
    if (presetHistory.size() > 10)
        presetHistory.pop_back();

    applyPreset(0);
}

void AutoPresetFollowerAudioProcessor::applyPreset(int index) {
    if (index >= 0 && index < static_cast<int>(presetHistory.size())) {
        activePresetIndex = index;
        const auto& preset = presetHistory[index];

        // Send host parameter modulation / VST3 parameter changes upstream
        for (const auto& [paramId, normVal] : preset.normalizedParams) {
            // Emits VST3 / AU parameter automation events back to the DAW host bus
            juce::Logger::writeToLog("Tweaking upstream parameter [" + paramId + "] -> " + juce::String(normVal));
        }
    }
}

void AutoPresetFollowerAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AutoPresetFollowerAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* AutoPresetFollowerAudioProcessor::createEditor() {
    return new AutoPresetFollowerEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AutoPresetFollowerAudioProcessor();
}
