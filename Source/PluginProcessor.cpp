#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
_8BitSynthAudioProcessor::_8BitSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
, formula_manager(parser){
    
}

_8BitSynthAudioProcessor::~_8BitSynthAudioProcessor()
{
    
}

//==============================================================================
const juce::String _8BitSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool _8BitSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool _8BitSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool _8BitSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double _8BitSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int _8BitSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int _8BitSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void _8BitSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String _8BitSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void _8BitSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void _8BitSynthAudioProcessor::prepareToPlay (double currentSampleRate, int currentSamplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void _8BitSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool _8BitSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void _8BitSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    /*
    auto numChannels = buffer.getNumChannels(); // block通道数
    auto numSamples = buffer.getNumSamples(); // block大小

    // 清空缓冲区
    buffer.clear();

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            int noteNumber = message.getNoteNumber();
            isNoteActive[noteNumber] = true; // 记录音符激活状态
        }
        else if (message.isNoteOff())
        {
            int noteNumber = message.getNoteNumber();
            isNoteActive[noteNumber] = false; // 重置音符激活状态
            noteTimes[noteNumber] = 0; // 音符结束，时间重置
        }
    }

    // 填充缓冲区
    for (auto channel = 0; channel < numChannels; ++channel)
    {
        auto* writePointer = buffer.getWritePointer(channel);
        for (int32_t sample = 0; sample < numSamples; ++sample)
        {
            int32_t sampleValue = 0;

            // 计算所有激活音符的输出
            for (int note = 0; note < 128; ++note)
            {
                if (isNoteActive[note])
                {
                    double frequency = 440.0 * std::pow(2.0, (note - 69) / 12.0);
                    int32_t t = (static_cast<int32_t>(std::floor((noteTimes[note] + sample) * frequency * 256.0 / sampleRate)));
                    
                    sampleValue = sampleValue + ( 
                        shiftRight(t, 6) | (shiftRight((t & 174) * (shiftRight(t, 7) & 209), 1) ^ (t & 14))
                        ) % 256;
                }
            }

            writePointer[sample] = sampleValue / (255.0f * 2.0f); // 写入缓冲区
        }
    }


    // 更新音符时间
    for (int i = 0; i < 128; ++i)
    {
        if (isNoteActive[i])
        {
            noteTimes[i] += numSamples; // 为每个激活音符增加时间
        }
        else {
            noteTimes[i] = 0;
        }
    }
    */
}

//==============================================================================
bool _8BitSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* _8BitSynthAudioProcessor::createEditor()
{
    return new _8BitSynthAudioProcessorEditor (*this);
}

//==============================================================================
void _8BitSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void _8BitSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new _8BitSynthAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout _8BitSynthAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterInt>("w", "w", 0, 255, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("x", "x", 0, 255, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("y", "y", 0, 255, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("z", "z", 0, 255, 0));
    
    return layout;
};


//==============================================================================

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
}