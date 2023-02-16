 /*
  ==============================================================================

    JUCE Processor of the DopoDelay plugin.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


DopoDelayAudioProcessor::DopoDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, juce::Identifier(PLUGIN_ID), createParameters())
#endif
{
    // Initialize the pointer to the delaySeconds parameter
    delaySecondsPtr = treeState.getRawParameterValue(PARAM_DELAY_SECONDS_ID);
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
    // Use this method to do any pre-playback initialization that you need..
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

        // writePosition = current position in the delayBuffer
        // readPosition = where we read from the past in the delayBuffer
        auto readPosition = writePosition - getSampleRate() * *delaySecondsPtr;

        if (readPosition < 0)
            readPosition += delayBufferSize;
        
        if (readPosition + bufferSize < delayBufferSize)
        {
            buffer.copyFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, GAIN_DEFAULT, GAIN_DEFAULT);
        }
        else
        {
            auto numSamplesToEnd = delayBufferSize - readPosition;
            auto numSamplesAtStart = bufferSize - numSamplesToEnd;
            buffer.copyFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, GAIN_DEFAULT, GAIN_DEFAULT);
            buffer.copyFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesAtStart, GAIN_DEFAULT, GAIN_DEFAULT);
        }
    }

    writePosition += bufferSize;   
    writePosition %= delayBufferSize;
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

    auto currentState = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(currentState.createXml());
    copyXmlToBinary(*xml, destData);
}

void DopoDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(treeState.state.getType()))
            treeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DopoDelayAudioProcessor();
}



void DopoDelayAudioProcessor::fillDelayBuffer(int channel, int bufferSize, int delayBufferSize, float* channelData)
{
    // Check to see if main buffer copies to delay buffer without needing to wrap
    if (delayBufferSize > bufferSize + writePosition)
    {
        // Copy the main buffer to the delay buffer
        delayBuffer.copyFromWithRamp(channel, writePosition, channelData, bufferSize, GAIN_DEFAULT, GAIN_DEFAULT);
    }
    else
    {
        // Determine the space left at the end of the delay buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;

        // Copy the starting of the main buffer at the en\d of the delay buffer
        delayBuffer.copyFromWithRamp(channel, writePosition, channelData, numSamplesToEnd, GAIN_DEFAULT, GAIN_DEFAULT);

        // Copy the end of the main buffer at the beginning of the delay buffer
        delayBuffer.copyFromWithRamp(channel, 0, channelData, numSamplesAtStart, GAIN_DEFAULT, GAIN_DEFAULT);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DopoDelayAudioProcessor::createParameters()
{
    // Create a vector of parameters
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Create the delaySeconds parameter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(PARAM_DELAY_SECONDS_ID, PARAM_DELAY_SECONDS_NAME, PARAM_DELAY_SECONDS_MIN, PARAM_DELAY_SECONDS_MAX, PARAM_DELAY_SECONDS_DEFAULT));

    return { params.begin(), params.end() };
}
