#include "PluginEditor.h"

AutoPresetFollowerEditor::AutoPresetFollowerEditor(AutoPresetFollowerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(520, 480);

    // Input Gain Slider
    inputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(inputGainSlider);
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "input_gain", inputGainSlider);

    // Output Gain Slider
    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(outputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "output_gain", outputGainSlider);

    // Genre / Subgenre Dropdown
    genreMenu.addItemList(juce::StringArray{
        "Heavy Trap / Drill", "Modern Trap", "Boom Bap 90s",
        "Melodic Techno", "Liquid DnB", "Future Bass / Pop", "Lo-Fi Hip Hop"
    }, 1);
    addAndMakeVisible(genreMenu);
    genreAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "genre_select", genreMenu);

    // Prompt Text Box
    promptBox.setText("reese bass for heavy trap");
    promptBox.setFont(juce::Font(13.0f));
    addAndMakeVisible(promptBox);

    // Generate Button
    generateButton.setButtonText("Generate");
    generateButton.addListener(this);
    addAndMakeVisible(generateButton);

    refreshPresetButtons();
}

void AutoPresetFollowerEditor::buttonClicked(juce::Button* b) {
    if (b == &generateButton) {
        audioProcessor.generatePresetsFromPrompt(promptBox.getText(), genreMenu.getSelectedItemIndex());
        refreshPresetButtons();
        repaint();
    } else {
        for (size_t i = 0; i < presetButtons.size(); ++i) {
            if (b == presetButtons[i].get()) {
                audioProcessor.applyPreset(static_cast<int>(i));
                refreshPresetButtons();
                repaint();
                break;
            }
        }
    }
}

void AutoPresetFollowerEditor::refreshPresetButtons() {
    presetButtons.clear();
    const auto& history = audioProcessor.getPresetHistory();

    for (size_t i = 0; i < history.size(); ++i) {
        auto btn = std::make_unique<juce::TextButton>(history[i].title);
        btn->addListener(this);
        if (static_cast<int>(i) == audioProcessor.getActivePresetIndex()) {
            btn->setColour(juce::TextButton::buttonColourId, juce::Colours::orange.withAlpha(0.6f));
        }
        addAndMakeVisible(*btn);
        presetButtons.push_back(std::move(btn));
    }
    resized();
}

void AutoPresetFollowerEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff121214));

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("FLOW AUTO-PRESET FOLLOWER", 20, 15, 300, 24, juce::Justification::left);

    g.setFont(juce::Font(11.0f));
    g.setColour(juce::Colours::grey);
    g.drawText("Detected Slot: Upstream Synth / FX (Auto-Mapped)", 20, 38, 350, 16, juce::Justification::left);

    // Labels for Knobs
    g.setColour(juce::Colours::lightgrey);
    g.drawText("Input Gain", 50, 80, 80, 20, juce::Justification::centred);
    g.drawText("Output Gain", 390, 80, 80, 20, juce::Justification::centred);
    g.drawText("Musical Genre & Subgenre", 160, 80, 200, 20, juce::Justification::centred);

    // Preset Buffer Label
    g.drawText("Preset Buffer (Max 10)", 20, 245, 200, 20, juce::Justification::left);
}

void AutoPresetFollowerEditor::resized() {
    inputGainSlider.setBounds(40, 105, 100, 100);
    genreMenu.setBounds(160, 135, 200, 30);
    outputGainSlider.setBounds(380, 105, 100, 100);

    // Prompt input bar
    promptBox.setBounds(20, 205, 370, 32);
    generateButton.setBounds(400, 205, 100, 32);

    // 10 preset history buttons in 2 columns
    int startY = 270;
    int colWidth = 230;
    int rowHeight = 32;

    for (size_t i = 0; i < presetButtons.size(); ++i) {
        int col = static_cast<int>(i % 2);
        int row = static_cast<int>(i / 2);
        presetButtons[i]->setBounds(20 + col * (colWidth + 20), startY + row * (rowHeight + 6), colWidth, rowHeight);
    }
}