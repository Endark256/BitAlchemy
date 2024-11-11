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
    _8BitSynthVoice(std::shared_ptr<fparse::Expression>& e, juce::AudioProcessorValueTreeState& s, double& b) 
        : expr(e), apvts(s), bpm(b){
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
        standard_time = 0.;
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
        standard_time = 0.;
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

        // 设置 t 参数
        double end_time = time + (numSamples - 1) * 256.0 * frequency / sample_rate;
        double next_time = end_time + 256.0 * frequency / sample_rate;
        vars["t"] = xt::cast<int32_t>(xt::linspace<double>(time, end_time, numSamples));
        
        double block_bpm = bpm;
        if (block_bpm == -1.) {
            block_bpm = 150.;     // 默认bpm
        }

        // 设置 T 参数
        double end_standard_time = standard_time + (numSamples - 1) * 256.0 * block_bpm / (sample_rate * 60.);
        double next_standard_time = end_standard_time + 256.0 * block_bpm / (sample_rate * 60.);
        vars["T"] = xt::cast<int32_t>(xt::linspace<double>(standard_time, end_standard_time, numSamples));

        // 设置 w x y z 参数
        vars["w"][0] = apvts.getRawParameterValue("w")->load();
        vars["x"][0] = apvts.getRawParameterValue("x")->load();
        vars["y"][0] = apvts.getRawParameterValue("y")->load();
        vars["z"][0] = apvts.getRawParameterValue("z")->load();

        // 计算输出
        xt::xarray<float> result = xt::cast<float>((expr->evaluate(vars, numSamples) % 256 + 256) % 256) / 510.0f;
        
        // 将输出写入 buffer
        for (size_t i = 0; i < numSamples; i++) {
            for (auto channel = outputBuffer.getNumChannels(); --channel >= 0;)
                outputBuffer.addSample(channel, startSample + i, result[i]);
        }
        
        // 更新时间
        time = next_time;
        standard_time = next_standard_time;

    }

private:
    double frequency = 0.;
    double time = 0.;
    double standard_time = 0.;
    double& bpm;

    std::shared_ptr<fparse::Expression>& expr;
    std::unordered_map<std::string, fparse::EvaluationResult> vars;
    juce::AudioProcessorValueTreeState& apvts;
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
    double bpm = 0.;                                         // bpm

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_8BitSynthAudioProcessor)
};

