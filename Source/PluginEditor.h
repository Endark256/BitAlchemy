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

class RotarySlider : public juce::Slider {
public:
    RotarySlider() : juce::Slider(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::TextBoxAbove
    ) {};
};

class FormulaEditor : public juce::TextEditor {
public:
    FormulaEditor(FormulaManager& m);

    bool keyPressed(const juce::KeyPress& key);                 // �������¼�

    inline void updateText();                                   // �� Formula Manager ��ȡ�ı�������

    inline void attachToNewFormulaManager(FormulaManager& m);   // �󶨵��µ� FormulaManager ��

    inline void setOutlineColourToGreen();                      // ���ñ߿���ɫ
    inline void setOutlineColourToRed();
    inline void setOutlineColourToYellow();

private:
    FormulaManager* manager;
};


class _8BitSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    _8BitSynthAudioProcessorEditor (_8BitSynthAudioProcessor&);
    ~_8BitSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    _8BitSynthAudioProcessor& audioProcessor;

    RotarySlider 
        w_slider, 
        x_slider, 
        y_slider, 
        z_slider;    // wxyz ��ť

    FormulaEditor formula_editor;                           // ��
    
    juce::AudioProcessorValueTreeState::SliderAttachment 
        w_attachment, 
        x_attachment, 
        y_attachment, 
        z_attachment;    // �󶨲���

    std::vector<RotarySlider*> getRotarySliders();          // ���ڻ�ȡ 4 ����ť�ķ���

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_8BitSynthAudioProcessorEditor)
};


