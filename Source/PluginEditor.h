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
        setMultiLine(true, false);              // 多行模式，不强制断行
        setScrollbarsShown(true);               // 显示滚动条
        setTabKeyUsedAsCharacter(true);         // 允许键入Tab
        setReturnKeyStartsNewLine(true);        // 允许换行

        audio_processor = &p;                   // audio processor

        onTextChange = [this, &p]() -> void {   // 文本内容变化
                p.setFormula(getText().toStdString());                  // 写入 Formula
                setColour(focusedOutlineColourId, juce::Colour(231, 228, 98));  // 修改边框为黄色
                setColour(outlineColourId, juce::Colour(187, 183, 31));  
                repaint();
            };
    };


    bool keyPressed(const juce::KeyPress& key) {
        // 按下 shift + enter 的逻辑
        if (key == juce::KeyPress(juce::KeyPress::returnKey, juce::ModifierKeys::shiftModifier, NULL)) {
    
            auto result = audio_processor->parse();
    
            if (result.success) {
                // parse 成功时的逻辑
                setColour(focusedOutlineColourId, juce::Colour(98, 228, 117)); // 修改边框为绿色
                setColour(outlineColourId, juce::Colour(31, 183, 53));
            }
            else {
                // parse 失败时的逻辑
                setColour(focusedOutlineColourId, juce::Colour(228, 98, 98));  // 修改边框为红色
                setColour(outlineColourId, juce::Colour(183, 31, 31));
                // 显示错误信息
                // ...
            }
            repaint();
            return true;
        }
    
        // 按下其它按键的逻辑
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


