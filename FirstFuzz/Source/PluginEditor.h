/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FirstFuzzAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FirstFuzzAudioProcessorEditor (FirstFuzzAudioProcessor&);
    ~FirstFuzzAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    juce::ComboBox inputChannelCombo;
    juce::Label inputChannelLabel;
    std::unique_ptr<ComboBoxAttachment> inputChannelAttachment;

    juce::Slider gainSlider;
    juce::Slider gateSlider;
    juce::Slider toneSlider;
    juce::Slider volumeSlider;

    juce::Label gainLabel;
    juce::Label gateLabel;
    juce::Label toneLabel;
    juce::Label volumeLabel;

    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> gateAttachment;
    std::unique_ptr<SliderAttachment> toneAttachment;
    std::unique_ptr<SliderAttachment> volumeAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FirstFuzzAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstFuzzAudioProcessorEditor)
};
