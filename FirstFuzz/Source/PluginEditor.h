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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FirstFuzzAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstFuzzAudioProcessorEditor)
};
