/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace std;

//==============================================================================
_8BitSynthAudioProcessorEditor::_8BitSynthAudioProcessorEditor(_8BitSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), formula_editor(p),
    w_attachment(p.apvts, "w", w_slider),
    x_attachment(p.apvts, "x", x_slider),
    y_attachment(p.apvts, "y", y_slider),
    z_attachment(p.apvts, "z", z_slider)
{
    for (auto slider : getRotarySliders())
        addAndMakeVisible(slider);
    addAndMakeVisible(formula_editor);

    formula_editor.setText(p.getFormula());    // ∂¡»° formula

    setSize (800, 600);
}



_8BitSynthAudioProcessorEditor::~_8BitSynthAudioProcessorEditor()
{
}

//==============================================================================
void _8BitSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

juce::Rectangle<int> CenterBox(juce::Rectangle<int>& area, double border_rate) {
    auto border_width = area.getWidth() * border_rate;
    auto border_height = area.getHeight() * border_rate;

    area.removeFromTop(border_height);
    area.removeFromBottom(border_height);
    area.removeFromLeft(border_width);
    area.removeFromRight(border_width);

    return area;
}

void _8BitSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();

    auto border_width = bounds.getWidth() * 0.05;

    auto formula_editor_area = bounds.removeFromTop(bounds.getHeight() * 0.75);
    formula_editor_area.removeFromLeft(border_width);
    formula_editor_area.removeFromRight(border_width);
    formula_editor.setBounds(formula_editor_area);

    auto rotary_slider_area_width = bounds.getWidth() * 0.25;

    w_slider.setBounds(CenterBox(bounds.removeFromLeft(rotary_slider_area_width), 0.2));
    x_slider.setBounds(CenterBox(bounds.removeFromLeft(rotary_slider_area_width), 0.2));
    y_slider.setBounds(CenterBox(bounds.removeFromLeft(rotary_slider_area_width), 0.2));
    z_slider.setBounds(CenterBox(bounds.removeFromLeft(rotary_slider_area_width), 0.2));
}

std::vector<RotarySlider*> _8BitSynthAudioProcessorEditor::getRotarySliders() {
    return {
        &w_slider,
        &x_slider, 
        &y_slider, 
        &z_slider
    };
}

