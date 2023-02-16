 /*
  ==============================================================================

    JUCE Processor of the DopoDelay plugin.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class DopoDelayAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    const juce::String PLUGIN_ID = "DopoDelay";

    const juce::String PARAM_DELAY_SECONDS_ID = "delaySeconds";
    const juce::String PARAM_DELAY_SECONDS_NAME = "Delay Seconds";
    const float PARAM_DELAY_SECONDS_MIN = 0.0f;
    const float PARAM_DELAY_SECONDS_MAX = 2.95f;
    const float PARAM_DELAY_SECONDS_DEFAULT = 0.2f;

    const float GAIN_DEFAULT = 1.0f;

    juce::AudioProcessorValueTreeState treeState;

    //==============================================================================
    DopoDelayAudioProcessor();
    ~DopoDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
 
private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition{ 0 };

    std::atomic<float>* delaySecondsPtr = nullptr;

    void DopoDelayAudioProcessor::fillDelayBuffer(int channel, int bufferSize, int delayBufferSize, float* channelData);
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DopoDelayAudioProcessor)
};
