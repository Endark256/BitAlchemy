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

    bool keyPressed(const juce::KeyPress& key);                 // 处理按键事件

    inline void updateText();                                   // 从 Formula Manager 获取文本并更新

    inline void attachToNewFormulaManager(FormulaManager& m);   // 绑定到新的 FormulaManager 上

    inline void setOutlineColourToGreen();                      // 设置边框颜色
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
        z_slider;    // wxyz 旋钮

    FormulaEditor formula_editor;                           // 框
    
    juce::AudioProcessorValueTreeState::SliderAttachment 
        w_attachment, 
        x_attachment, 
        y_attachment, 
        z_attachment;    // 绑定参数

    std::vector<RotarySlider*> getRotarySliders();          // 便于获取 4 个旋钮的方法

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_8BitSynthAudioProcessorEditor)
};


