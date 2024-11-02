/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

int32_t  shiftRight(int32_t  value, int32_t  count) {
    if (value < 0) {
        return (value >> count) | ((~0) << (32 - count)); // 填充符号位
    }
    else {
        return value >> count; // 正数直接右移
    }
}

int32_t shiftLeft(int32_t value, int32_t count) {
    return value << (count % 16);
}

int32_t div32(int32_t a, int32_t b) {
    if (b == 0) return 0;
    return a / b;
}

int32_t mod32(int32_t a, int32_t b) {
    if (b == 0) return 0;
    return a % b;
}

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
{
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
    sampleRate = currentSampleRate;
    samplesPerBlock = currentSamplesPerBlock;
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
                    //sampleValue = sampleValue + ( ((((t / 2) * (t >> 8)) | ((t % 10) / (t >> t))) | ((t >> 16) >> t)) | (t / 4) ) % 256;
                    sampleValue = sampleValue + ( 
                        (((div32(t, 2) * shiftRight(t, 8)) | div32(mod32(t, 10), shiftRight(t, t))) | shiftRight(t, t)) | (t / 4) 
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
