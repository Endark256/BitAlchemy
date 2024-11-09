#pragma once

#include <JuceHeader.h>
#include "FormulaParser.h"

//==============================================================================
class _8BitSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    _8BitSynthAudioProcessor();
    ~_8BitSynthAudioProcessor() override;

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

    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    //==============================================================================
    std::string getFormula();                       // 获取当前 formula
    void setFormula(std::string& formula);          // 设置当前 formula
    fparse::ParseResult parse();                    // parse 当前 formula
    bool isFormulaParsed();                         // 返回当前 formula 是否已被 parse 过

private:
    //==============================================================================
    fparse::FormulaParser parser;                   // parser
    std::string formula;                            // formula
    bool parsed;                                    // 当前 formula 是否已被 parse 过
    std::shared_ptr<fparse::Expression> expr;       // 上一个有效 formula 的 parse 结果

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_8BitSynthAudioProcessor)
};
