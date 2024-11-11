#pragma once

#include <JuceHeader.h>
#include "FormulaParser.h"
#include <xtensor/xarray.hpp>
#include <xtensor/xview.hpp>
#include <cstdint>


//==============================================================================
// ����ʽ
class FormulaManager {
private:
    fparse::FormulaParser* parser;                          // parser
    std::string formula;                                    // formula
    bool parsed;                                            // ��ǰ formula �Ƿ��ѱ� parse ��
    std::shared_ptr<fparse::Expression> expr;               // ��һ����Ч formula �� parse ���

public:
    FormulaManager(fparse::FormulaParser& formula_parser) {             // ���캯��
        parser = &formula_parser;
        formula = "";
        parsed = false;
        expr = nullptr;
    };

    inline std::string getFormula() {                                   // ��ȡ��ǰ formula
        return formula;
    };

    inline void setFormula(std::string& formula_string) {               // ���õ�ǰ formula
        formula = formula_string;
        parsed = false;                                                 // ��ǰ formula δ�� parse
    };

    inline fparse::ParseResult parse() {                                // parse ��ǰ formula
        fparse::ParseResult result = parser->parse(formula);
        if (result.success) {                                           // ���ִ�гɹ�����ǰ formula �ѱ� parse������ parse �Ľ��
            parsed = true;
            expr = result.expr;
        }
        return result;
    };

    inline bool isFormulaParsed() {                                     // ���ص�ǰ formula �Ƿ��ѱ� parse ��
        return parsed;
    };

    inline std::shared_ptr<fparse::Expression>& getExpr() {             // ������һ�������� expr �����ָ�������
        return expr;
    };
};

//==============================================================================
// Sound ��
class _8BitSynthSound : public juce::SynthesiserSound {
public:
    _8BitSynthSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};


//==============================================================================
// Voice ��
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
        //    // �����������߼�
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

    // �޸� w x y z ��ֵ
    inline void setMacro(const std::string macro_name, int value) {
        vars[macro_name] = { value };
    };

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override {
        if (expr == nullptr)    // ���ʽδ����
            return;
        if (frequency == 0.)    // ����δ����
            return;

        double sample_rate = getSampleRate();

        // ���� t ����
        double end_time = time + (numSamples - 1) * 256.0 * frequency / sample_rate;
        double next_time = end_time + 256.0 * frequency / sample_rate;
        vars["t"] = xt::cast<int32_t>(xt::linspace<double>(time, end_time, numSamples));
        
        double block_bpm = bpm;
        if (block_bpm == -1.) {
            block_bpm = 150.;     // Ĭ��bpm
        }

        // ���� T ����
        double end_standard_time = standard_time + (numSamples - 1) * 256.0 * block_bpm / (sample_rate * 60.);
        double next_standard_time = end_standard_time + 256.0 * block_bpm / (sample_rate * 60.);
        vars["T"] = xt::cast<int32_t>(xt::linspace<double>(standard_time, end_standard_time, numSamples));

        // ���� w x y z ����
        vars["w"][0] = apvts.getRawParameterValue("w")->load();
        vars["x"][0] = apvts.getRawParameterValue("x")->load();
        vars["y"][0] = apvts.getRawParameterValue("y")->load();
        vars["z"][0] = apvts.getRawParameterValue("z")->load();

        // �������
        xt::xarray<float> result = xt::cast<float>((expr->evaluate(vars, numSamples) % 256 + 256) % 256) / 510.0f;
        
        // �����д�� buffer
        for (size_t i = 0; i < numSamples; i++) {
            for (auto channel = outputBuffer.getNumChannels(); --channel >= 0;)
                outputBuffer.addSample(channel, startSample + i, result[i]);
        }
        
        // ����ʱ��
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
// Audio Processor ��
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
    FormulaManager formula_manager;                     // ��ʽ������

private:
    //==============================================================================
    fparse::FormulaParser parser;                       // parser
    juce::Synthesiser synth;                            // synth
    double bpm = 0.;                                         // bpm

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_8BitSynthAudioProcessor)
};

