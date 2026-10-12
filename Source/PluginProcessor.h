#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <string>
#include <map>

struct PresetData {
    juce::String title;
    juce::String prompt;
    double bpm = 120.0;
    std::map<juce::String, float> normalizedParams; // paramID -> 0.0 to 1.0
};

class AutoPresetFollowerAudioProcessor : public juce::AudioProcessor {
public:
    AutoPresetFollowerAudioProcessor();
    ~AutoPresetFollowerAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Flow Auto-Preset Follower"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Preset & Host Control Logic
    void generatePresetsFromPrompt(const juce::String& prompt, int genreIdx);
    void applyPreset(int index);
    const std::vector<PresetData>& getPresetHistory() const { return presetHistory; }
    int getActivePresetIndex() const { return activePresetIndex; }
    double getCurrentBpm() const { return currentBpm; }
    
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::SmoothedValue<float> smoothedInputGain;
    juce::SmoothedValue<float> smoothedOutputGain;

    std::vector<PresetData> presetHistory;
    int activePresetIndex = -1;
    double currentBpm = 120.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoPresetFollowerAudioProcessor)
};