/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DopoDelayAudioProcessorEditor::DopoDelayAudioProcessorEditor (DopoDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize(400, 70);

    // Attach the slider to the state's delaySeconds parameter
    delaySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "delaySeconds", delaySlider);
    delaySlider.setTextValueSuffix(" s");
    addAndMakeVisible(delaySlider);

    delayLabel.setText("Delay", juce::dontSendNotification);
    delayLabel.attachToComponent(&delaySlider, true);
    addAndMakeVisible(delayLabel);  
}

DopoDelayAudioProcessorEditor::~DopoDelayAudioProcessorEditor()
{
}

//==============================================================================
void DopoDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Our component is opaque, so we must completely fill the background with a solid colour
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void DopoDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any subcomponents in your editor..
    auto margin = 24;
    auto labelWidth = 36;
    delaySlider.setBounds(margin + labelWidth, margin, getWidth() - 2 * margin - labelWidth, 20);
}
