/*
  ==============================================================================

    JUCE Editor of the DopoDelay plugin.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;


class DopoDelayAudioProcessorEditor  : public juce::AudioProcessorEditor                                      
{
public:
    DopoDelayAudioProcessorEditor (DopoDelayAudioProcessor&);
    ~DopoDelayAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    DopoDelayAudioProcessor& audioProcessor;  // reference to the processor that created the editor

    juce::Label  delayLabel;
    juce::Slider delaySlider;
    std::unique_ptr<SliderAttachment> delaySliderAttachment;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DopoDelayAudioProcessorEditor)
};
