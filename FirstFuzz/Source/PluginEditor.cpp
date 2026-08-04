/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FirstFuzzAudioProcessorEditor::FirstFuzzAudioProcessorEditor (FirstFuzzAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    auto setupSlider = [this] (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 20);
        addAndMakeVisible (slider);
    };

    auto setupLabel = [this] (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    setupSlider (gainSlider);
    setupSlider (gateSlider);
    setupSlider (toneSlider);
    setupSlider (volumeSlider);

    setupLabel (gainLabel, "Gain");
    setupLabel (gateLabel, "Gate");
    setupLabel (toneLabel, "Tone");
    setupLabel (volumeLabel, "Volume");

    auto& apvts = audioProcessor.getValueTreeState();
    gainAttachment = std::make_unique<SliderAttachment> (apvts, "gain", gainSlider);
    gateAttachment = std::make_unique<SliderAttachment> (apvts, "gate", gateSlider);
    toneAttachment = std::make_unique<SliderAttachment> (apvts, "tone", toneSlider);
    volumeAttachment = std::make_unique<SliderAttachment> (apvts, "volume", volumeSlider);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (480, 240);
}

FirstFuzzAudioProcessorEditor::~FirstFuzzAudioProcessorEditor()
{
}

//==============================================================================
void FirstFuzzAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void FirstFuzzAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto row = area.removeFromTop (area.getHeight());

    const int columnWidth = row.getWidth() / 4;
    auto layoutKnob = [&] (juce::Slider& slider, juce::Label& label, int index)
    {
        auto column = row.withTrimmedLeft (index * columnWidth).removeFromLeft (columnWidth).reduced (6);
        label.setBounds (column.removeFromTop (20));
        slider.setBounds (column);
    };

    layoutKnob (gainSlider, gainLabel, 0);
    layoutKnob (gateSlider, gateLabel, 1);
    layoutKnob (toneSlider, toneLabel, 2);
    layoutKnob (volumeSlider, volumeLabel, 3);
}
