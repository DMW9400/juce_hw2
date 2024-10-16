/*
  ==============================================================================

    gui_record_play.cpp
    Created: 11 Oct 2024 7:25:39pm
    Author:  David Matthew Welch

  ==============================================================================
*/
#include <JuceHeader.h>
#include "gui_record_play.h"

AppState state = IDLE;

bool AudioToFileWriter::setup(const juce::File& outputFile, int sampleRate, int numChannels)
{
    if (outputFile.existsAsFile())
    {
        outputFile.deleteFile();
    }

    // Create the FileOutputStream as a unique_ptr
    std::unique_ptr<juce::FileOutputStream> stream = outputFile.createOutputStream();

    if (stream != nullptr)
    {
        // Instantiate WavAudioFormat
        juce::WavAudioFormat wavFormat;

        // Create the writer and let it take ownership of the stream
        writer.reset(wavFormat.createWriterFor(stream.release(),  // Transfer ownership
                                               sampleRate,
                                               static_cast<unsigned int>(numChannels),
                                               16,
                                               {},
                                               0));

        if (writer != nullptr)
        {
            DBG("File Write Successful");
            return true;
        }
        else
        {
            DBG("Failed to create AudioFormatWriter.");
        }
    }
    else
    {
        DBG("Failed to create FileOutputStream.");
    }

    DBG("File Write Failed");
    return false;
};

void AudioToFileWriter::writeOutputToFile(const juce::AudioBuffer<float>& buffer)
{
    std::lock_guard<std::mutex> lock(fileMutex);  // Lock the mutex while writing

    if (writer != nullptr)
    {
        int numSamples = buffer.getNumSamples();

        if (numSamples > 0)
        {
            writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);
        }
    }
};

void AudioToFileWriter::closeFile()
{
    std::lock_guard<std::mutex> lock(fileMutex);

    if (writer != nullptr)
    {
        DBG("Closing writer and associated file stream...");
        writer.reset();  // This will close the writer and the owned FileOutputStream
    }
    else
    {
        DBG("Writer was null, nothing to close.");
    }
}

DisplayAudioWaveForm::DisplayAudioWaveForm()
    : audioVisualiser(1)
{
    audioVisualiser.setBufferSize(1024);
    
    
    audioVisualiser.setSamplesPerBlock(256);
    
    audioVisualiser.setColours(juce::Colours::green, juce::Colours::black);
    
    addAndMakeVisible(audioVisualiser);
}

DisplayAudioWaveForm::~DisplayAudioWaveForm()
{
}

void DisplayAudioWaveForm::addAudioData(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples){
//    get the read pointers fo each channel in the buffer
    const float *const  *channelData = buffer.getArrayOfReadPointers();
    audioVisualiser.pushBuffer(channelData, buffer.getNumChannels(), numSamples);
};

void DisplayAudioWaveForm::paint(juce::Graphics &g){
    g.fillAll(juce::Colours::black);
};

void DisplayAudioWaveForm::resized(){
    auto bounds = getLocalBounds().reduced(10);

    audioVisualiser.setBounds(bounds);
};
