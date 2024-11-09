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
    FormulaEditor(_8BitSynthAudioProcessor& p) : 
        juce::TextEditor("FormulaEditor")
    {
        setMultiLine(true, false);              // ����ģʽ����ǿ�ƶ���
        setScrollbarsShown(true);               // ��ʾ������
        setTabKeyUsedAsCharacter(true);         // �������Tab
        setReturnKeyStartsNewLine(true);        // ������

        audio_processor = &p;                   // audio processor

        onTextChange = [this, &p]() -> void {   // �ı����ݱ仯
                p.setFormula(getText().toStdString());                  // д�� Formula
                setColour(focusedOutlineColourId, juce::Colour(231, 228, 98));  // �޸ı߿�Ϊ��ɫ
                setColour(outlineColourId, juce::Colour(187, 183, 31));  
                repaint();
            };
    };


    bool keyPressed(const juce::KeyPress& key) {
        // ���� shift + enter ���߼�
        if (key == juce::KeyPress(juce::KeyPress::returnKey, juce::ModifierKeys::shiftModifier, NULL)) {
    
            auto result = audio_processor->parse();
    
            if (result.success) {
                // parse �ɹ�ʱ���߼�
                setColour(focusedOutlineColourId, juce::Colour(98, 228, 117)); // �޸ı߿�Ϊ��ɫ
                setColour(outlineColourId, juce::Colour(31, 183, 53));
            }
            else {
                // parse ʧ��ʱ���߼�
                setColour(focusedOutlineColourId, juce::Colour(228, 98, 98));  // �޸ı߿�Ϊ��ɫ
                setColour(outlineColourId, juce::Colour(183, 31, 31));
                // ��ʾ������Ϣ
                // ...
            }
            repaint();
            return true;
        }
    
        // ���������������߼�
        return juce::TextEditor::keyPressed(key);
    };


private:
    _8BitSynthAudioProcessor* audio_processor;
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


