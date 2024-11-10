#pragma once

#include <JuceHeader.h>
#include "FormulaParser.h"
#include <xtensor/xarray.hpp>
#include <xtensor/xview.hpp>
#include <cstdint>


//==============================================================================
// 管理公式
class FormulaManager {
private:
    fparse::FormulaParser* parser;                          // parser
    std::string formula;                                    // formula
    bool parsed;                                            // 当前 formula 是否已被 parse 过
    std::shared_ptr<fparse::Expression> expr;               // 上一个有效 formula 的 parse 结果

public:
    FormulaManager(fparse::FormulaParser& formula_parser) {             // 构造函数
        parser = &formula_parser;
        formula = "";
        parsed = false;
        expr = nullptr;
    };

    inline std::string getFormula() {                                   // 获取当前 formula
        return formula;
    };

    inline void setFormula(std::string& formula_string) {               // 设置当前 formula
        formula = formula_string;
        parsed = false;                                                 // 当前 formula 未被 parse
    };

    inline fparse::ParseResult parse() {                                // parse 当前 formula
        fparse::ParseResult result = parser->parse(formula);
        if (result.success) {                                           // 如果执行成功，则当前 formula 已被 parse，储存 parse 的结果
            parsed = true;
            expr = result.expr;
        }
        return result;
    };

    inline bool isFormulaParsed() {                                     // 返回当前 formula 是否已被 parse 过
        return parsed;
    };

    inline std::shared_ptr<fparse::Expression>& getExpr() {             // 返回上一个解析的 expr 对象的指针的引用
        return expr;
    };
};

//==============================================================================
/*
FormulaManager::FormulaManager(fparse::FormulaParser& formula_parser) {
    parser = &formula_parser;
    formula = "";
    parsed = false;
    expr = nullptr;
}

inline std::string FormulaManager::getFormula() {
    return formula;
};

inline void FormulaManager::setFormula(std::string& formula_string) {
    formula = formula_string;
    parsed = false;                 // 当前 formula 未被 parse
};

inline fparse::ParseResult FormulaManager::parse() {
    fparse::ParseResult result = parser->parse(formula);
    if (result.success) {           // 如果执行成功，则当前 formula 已被 parse，储存 parse 的结果
        parsed = true;
        expr = result.expr;
    }
    return result;
};

inline bool FormulaManager::isFormulaParsed() {
    return parsed;
};

inline std::shared_ptr<fparse::Expression> FormulaManager::getExpr() {
    return expr;
}*/


//==============================================================================
// Sound 类
class _8BitSynthSound : public juce::SynthesiserSound {
public:
    _8BitSynthSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};


//==============================================================================
// Voice 类
class _8BitSynthVoice : public juce::SynthesiserVoice {
public:
    _8BitSynthVoice(std::shared_ptr<fparse::Expression>& e) : expr(e) {
        vars["T"] = fparse::EvaluationResult({ 0 });
        vars["t"] = fparse::EvaluationResult({ 0 });
        vars["w"] = fparse::EvaluationResult({ 0 });
        vars["x"] = fparse::EvaluationResult({ 0 });
        vars["y"] = fparse::EvaluationResult({ 0 });
        vars["z"] = fparse::EvaluationResult({ 0 });
    };

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<_8BitSynthSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override {
        frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        time = 0.;
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        //if (allowTailOff)
        //{
        //    // 音符结束的逻辑
        //}
        //else
        //{
        //    
        //}
        clearCurrentNote();
        frequency = 0.;
        time = 0.;
    }

    void pitchWheelMoved(int) override {};
    void controllerMoved(int, int) override {};

    // 修改 w x y z 的值
    inline void setMacro(const std::string macro_name, int value) {
        vars[macro_name] = { value };
    };

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override {
        if (expr == nullptr)    // 表达式未解析
            return;
        if (frequency == 0.)    // 音符未按下
            return;

        double sample_rate = getSampleRate();

        double end_time = time + numSamples * 256.0 * frequency / sample_rate;
        double next_time = end_time + 256.0 * frequency / sample_rate;

        vars["t"] = xt::cast<int32_t>(xt::linspace<double>(time, end_time, numSamples));

        xt::xarray<float> result = xt::cast<float>((expr->evaluate(vars, numSamples) % 256 + 256) % 256) / 510.0f;
        
        for (size_t i = 0; i < numSamples; i++) {
            for (auto channel = outputBuffer.getNumChannels(); --channel >= 0;)
                outputBuffer.addSample(channel, startSample + i, result[i]);
        }

        time = next_time;

    }

private:
    double frequency = 0.;
    double time = 0.;
    std::shared_ptr<fparse::Expression>& expr;
    std::unordered_map<std::string, fparse::EvaluationResult> vars;
};


//==============================================================================
// Audio Processor 类
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
    FormulaManager formula_manager;                     // 公式管理器

private:
    //==============================================================================
    fparse::FormulaParser parser;                       // parser
    juce::Synthesiser synth;                            // synth

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_8BitSynthAudioProcessor)
};

