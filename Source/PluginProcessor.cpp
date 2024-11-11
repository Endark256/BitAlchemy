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
, formula_manager(parser) {
    std::shared_ptr<fparse::Expression>& expr = formula_manager.getExpr();
    for (auto i = 0; i < 16; ++i)
        synth.addVoice(new _8BitSynthVoice(expr, apvts, bpm));

    synth.addSound(new _8BitSynthSound());
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
    uint8_t oversampling_factor = apvts.getRawParameterValue("oversampling_factor")->load();

    synth.setCurrentPlaybackSampleRate(currentSampleRate * oversampling_factor);

    constexpr auto filterType = juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR;

    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(2, oversampling_factor, filterType);
    oversampler->initProcessing(currentSamplesPerBlock);
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
    buffer.clear();

    // 获取播放头
    auto position = getPlayHead()->getPosition();


    // 获取 bpm
    bpm = -1.;

    if (position.hasValue()) {
        auto bpm_info = position->getBpm();
        if (bpm_info.hasValue())
            if (*bpm_info != 0.)
                bpm = *bpm_info;
    };


    // 重新设置滤波器
    auto currentSampleRate = getSampleRate();
    auto currentSamplesPerBlock = buffer.getNumSamples();

    uint8_t oversampling_factor = apvts.getRawParameterValue("oversampling_factor")->load();


    // 过采样
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> osBlock = oversampler->processSamplesUp(block);
    float* p[] = {osBlock.getChannelPointer(0), osBlock.getChannelPointer(1)};
    juce::AudioBuffer<float> osBuffer(p, 2, static_cast<int> (osBlock.getNumSamples()));
    synth.setCurrentPlaybackSampleRate(currentSampleRate * oversampling_factor);
    synth.renderNextBlock(osBuffer, midiMessages, 0, osBlock.getNumSamples());
    oversampler->processSamplesDown(block);

    osBlock.clear();
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

    layout.add(std::make_unique<juce::AudioParameterInt>("oversampling_factor", "oversampling_factor", 1, 16, 2));
    
    return layout;
};