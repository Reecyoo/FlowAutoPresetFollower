#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class AutoPresetFollowerEditor : public juce::AudioProcessorEditor,
                                 public juce::Button::Listener {
public:
    explicit AutoPresetFollowerEditor(AutoPresetFollowerAudioProcessor&);
    ~AutoPresetFollowerEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button*) override;

private:
    AutoPresetFollowerAudioProcessor& audioProcessor;

    // Strictly requested user controls
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::ComboBox genreMenu;

    // Text box + Generate button
    juce::TextEditor promptBox;
    juce::TextButton generateButton;

    // 10-Preset Buffer Buttons
    std::vector<std::unique_ptr<juce::TextButton>> presetButtons;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> genreAttachment;

    void refreshPresetButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoPresetFollowerEditor)
};