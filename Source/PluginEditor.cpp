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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 70);

    delaySlider.setRange(0, 2.950, 0.001);
    delaySlider.setTextValueSuffix(" s");
    delaySlider.setValue(0.2);
    delaySlider.addListener(this);
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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void DopoDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto margin = 24;
    auto labelWidth = 36;
    delaySlider.setBounds(margin + labelWidth, margin, getWidth() - 2 * margin - labelWidth, 20);
}

void DopoDelayAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &delaySlider)
    {
        audioProcessor.delaySeconds = delaySlider.getValue();
    }
}
