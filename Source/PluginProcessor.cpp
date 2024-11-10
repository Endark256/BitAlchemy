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
    for (auto i = 0; i < 4; ++i)
        synth.addVoice(new _8BitSynthVoice(expr));

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
    synth.setCurrentPlaybackSampleRate(currentSampleRate);
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

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
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