/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    static constexpr auto gainParamId = "gain";
    static constexpr auto gateParamId = "gate";
    static constexpr auto toneParamId = "tone";
    static constexpr auto volumeParamId = "volume";
    static constexpr auto inputChannelParamId = "inputChannel";
}

juce::AudioProcessorValueTreeState::ParameterLayout FirstFuzzAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    {
        auto range = juce::NormalisableRange<float> (1.0f, 40.0f, 0.01f, 0.4f);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { gainParamId, 1 }, "Gain", range, 8.0f));
    }

    {
        auto range = juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f, 1.0f);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { gateParamId, 1 }, "Gate", range, 0.08f));
    }

    {
        auto range = juce::NormalisableRange<float> (200.0f, 12000.0f, 1.0f, 0.5f);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { toneParamId, 1 }, "Tone", range, 4000.0f));
    }

    {
        auto range = juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f, 1.0f);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { volumeParamId, 1 }, "Volume", range, 0.8f));
    }

    {
        juce::StringArray choices { "Left", "Right", "Both" };
        params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { inputChannelParamId, 1 }, "Input Channel", choices, 2));
    }

    return { params.begin(), params.end() };
}

//==============================================================================
FirstFuzzAudioProcessor::FirstFuzzAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
#else
     : parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    gainParameter = parameters.getRawParameterValue (gainParamId);
    gateParameter = parameters.getRawParameterValue (gateParamId);
    toneParameter = parameters.getRawParameterValue (toneParamId);
    volumeParameter = parameters.getRawParameterValue (volumeParamId);
    inputChannelParameter = dynamic_cast<juce::AudioParameterChoice*> (parameters.getParameter (inputChannelParamId));
}

FirstFuzzAudioProcessor::~FirstFuzzAudioProcessor()
{
}

//==============================================================================
const juce::String FirstFuzzAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FirstFuzzAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FirstFuzzAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FirstFuzzAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FirstFuzzAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FirstFuzzAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FirstFuzzAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FirstFuzzAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FirstFuzzAudioProcessor::getProgramName (int index)
{
    return {};
}

void FirstFuzzAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FirstFuzzAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (samplesPerBlock);

    currentSampleRate = sampleRate;
    toneFilterState = { 0.0f, 0.0f };

    setLatencySamples (0);
}

void FirstFuzzAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FirstFuzzAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FirstFuzzAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    const auto gain = gainParameter != nullptr ? gainParameter->load() : 8.0f;
    const auto gate = gateParameter != nullptr ? gateParameter->load() : 0.08f;
    const auto tone = toneParameter != nullptr ? toneParameter->load() : 4000.0f;
    const auto volume = volumeParameter != nullptr ? volumeParameter->load() : 0.8f;
    const auto gateThreshold = juce::jmap (gate, 0.0f, 1.0f, 0.001f, 0.2f);
    const auto sampleRate = static_cast<float> (juce::jmax (1.0, currentSampleRate));
    const auto alpha = std::exp (-2.0f * juce::MathConstants<float>::pi * tone / sampleRate);
    const auto inputMode = inputChannelParameter != nullptr ? inputChannelParameter->getIndex() : 2;

    auto processChannel = [&] (int chIndex)
    {
        auto* channelData = buffer.getWritePointer (chIndex);
        auto& state = toneFilterState[static_cast<size_t> (juce::jmin (chIndex, 1))];

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto fuzzed = std::tanh (channelData[sample] * gain);

            if (std::abs (fuzzed) < gateThreshold)
                fuzzed = 0.0f;

            state = ((1.0f - alpha) * fuzzed) + (alpha * state);
            const auto toned = state;
            channelData[sample] = toned * volume;
        }
    };

    if (inputMode == 0)
    {
        // Left only
        processChannel (0);
        if (totalNumOutputChannels > 1)
            buffer.copyFrom (1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    else if (inputMode == 1)
    {
        // Right only
        if (totalNumInputChannels > 1)
        {
            processChannel (1);
            buffer.copyFrom (0, 0, buffer, 1, 0, buffer.getNumSamples());
        }
        else
        {
            processChannel (0);
            if (totalNumOutputChannels > 1)
                buffer.copyFrom (1, 0, buffer, 0, 0, buffer.getNumSamples());
        }
    }
    else
    {
        // Both channels
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
            processChannel (channel);
    }
}

//==============================================================================
bool FirstFuzzAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FirstFuzzAudioProcessor::createEditor()
{
    return new FirstFuzzAudioProcessorEditor (*this);
}

//==============================================================================
void FirstFuzzAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = parameters.copyState().createXml())
        copyXmlToBinary (*state, destData);
}

void FirstFuzzAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto state = getXmlFromBinary (data, sizeInBytes))
        parameters.replaceState (juce::ValueTree::fromXml (*state));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FirstFuzzAudioProcessor();
}
