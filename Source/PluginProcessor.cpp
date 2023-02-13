 /*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DopoDelayAudioProcessor::DopoDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DopoDelayAudioProcessor::~DopoDelayAudioProcessor()
{
}

//==============================================================================
const juce::String DopoDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DopoDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DopoDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DopoDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DopoDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DopoDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DopoDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DopoDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DopoDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DopoDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DopoDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    auto delayBufferSize = sampleRate * 3.0;  // 3 seconds buffer
    delayBuffer.setSize(getTotalNumOutputChannels(), (int)delayBufferSize);
}

void DopoDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DopoDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DopoDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        fillDelayBuffer(channel, bufferSize, delayBufferSize, channelData);

        auto gain = 1.0f;
        // writePositin = where is the audio
        // readPosition = where we read from the past
        auto readPosition = writePosition - getSampleRate() * delaySeconds;

        if (readPosition < 0)
            readPosition += delayBufferSize;
        
        if (readPosition + bufferSize < delayBufferSize)
        {
            buffer.copyFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, gain, gain);
        }
        else
        {
            auto numSamplesToEnd = delayBufferSize - readPosition;
            auto numSamplesAtStart = bufferSize - numSamplesToEnd;
            buffer.copyFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, gain, gain);
            buffer.copyFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, gain, gain);
        }

    }

    writePosition += bufferSize;   
    writePosition %= delayBufferSize;
}

void DopoDelayAudioProcessor::fillDelayBuffer(int channel, int bufferSize, int delayBufferSize, float* channelData)
{
    auto gain = 1.0f;

    // Check to see if main buffer copies to delay buffer without needing to wrap
    if (delayBufferSize > bufferSize + writePosition)
    {
        // Copy the main buffer to the delay buffer
        delayBuffer.copyFromWithRamp(channel, writePosition, channelData, bufferSize, gain, gain);
    }
    else
    {
        // Determine the space left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;

        // Copy the starting of the main buffer at the en\d of the delay buffer
        delayBuffer.copyFromWithRamp(channel, writePosition, channelData, numSamplesToEnd, gain, gain);

        // Copy the end of the main buffer at the beginning of the delay buffer
        delayBuffer.copyFromWithRamp(channel, 0, channelData, numSamplesAtStart, gain, gain);
    }
}

//==============================================================================
bool DopoDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DopoDelayAudioProcessor::createEditor()
{
    return new DopoDelayAudioProcessorEditor (*this);
}

//==============================================================================
void DopoDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DopoDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DopoDelayAudioProcessor();
}
