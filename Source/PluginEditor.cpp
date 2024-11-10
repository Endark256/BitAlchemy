/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace std;

//==============================================================================
FormulaEditor::FormulaEditor(FormulaManager& m) : juce::TextEditor("FormulaEditor")
{
    setMultiLine(true, false);              // ����ģʽ����ǿ�ƶ���
    setScrollbarsShown(true);               // ��ʾ������
    setTabKeyUsedAsCharacter(true);         // �������Tab
    setReturnKeyStartsNewLine(true);        // ������

    manager = &m;                           // audio processor

    updateText();                           // �����ı�����

    onTextChange = [this, &m]() -> void {   // �����ı����ݱ仯ʱ�Ĳ���
        m.setFormula(getText().toStdString());                          // д�� Formula
        setOutlineColourToYellow();
        repaint();
        };

    if (manager->isFormulaParsed())         // ��ʼ��ʱ������ɫ
        setOutlineColourToGreen();
    else
        setOutlineColourToYellow();

    repaint();
};

// �������ⰴ��
bool FormulaEditor::keyPressed(const juce::KeyPress& key) {
    // ���� shift + enter ���߼�
    if (key == juce::KeyPress(juce::KeyPress::returnKey, juce::ModifierKeys::shiftModifier, NULL)) {

        auto result = manager->parse();

        if (result.success) {
            // parse �ɹ�ʱ���߼�
            setOutlineColourToGreen();
        }
        else {
            // parse ʧ��ʱ���߼�
            setOutlineColourToRed();
            // ��ʾ������Ϣ
            // ...
        }
        repaint();
        return true;
    }

    // ���������������߼�
    return juce::TextEditor::keyPressed(key);
};

// �����ı�
inline void FormulaEditor::updateText() {
    juce::TextEditor::setText(manager->getFormula());
};

// ���ñ߿���ɫ
inline void FormulaEditor::setOutlineColourToGreen() {
    setColour(focusedOutlineColourId, juce::Colour(98, 228, 117));
    setColour(outlineColourId, juce::Colour(31, 183, 53));
}

inline void FormulaEditor::setOutlineColourToRed() {
    setColour(focusedOutlineColourId, juce::Colour(228, 98, 98));
    setColour(outlineColourId, juce::Colour(183, 31, 31));
}

inline void FormulaEditor::setOutlineColourToYellow() {
    setColour(focusedOutlineColourId, juce::Colour(231, 228, 98));
    setColour(outlineColourId, juce::Colour(187, 183, 31));
}

inline void FormulaEditor::attachToNewFormulaManager(FormulaManager& m) {
    manager = &m;
}

//==============================================================================
_8BitSynthAudioProcessorEditor::_8BitSynthAudioProcessorEditor(_8BitSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), formula_editor(p.formula_manager),
    w_attachment(p.apvts, "w", w_slider),
    x_attachment(p.apvts, "x", x_slider),
    y_attachment(p.apvts, "y", y_slider),
    z_attachment(p.apvts, "z", z_slider)
{
    for (auto slider : getRotarySliders())
        addAndMakeVisible(slider);
    addAndMakeVisible(formula_editor);

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
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
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