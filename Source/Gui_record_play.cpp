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

//bool AudioToFileWriter::setup(const juce::File& outputFile, int sampleRate, int numChannels){
//    
////    check if outputFile already exists, then delete if so
//    if (outputFile.existsAsFile())
//       {
//           outputFile.deleteFile();
//       }
////use FileOutputStream's createOutputStream method where audio data can be stored
//    fileStream = outputFile.createOutputStream();
////    ensure filestream is correctly created and not pointing erroneously
//    if (fileStream != nullptr){
////      reset the writer so it is ready for a new fileStream
//        writer.reset();
////        instantiate WavAudioFormat class with wavFormat, so we have access to createWriterFor method
//        juce::WavAudioFormat wavFormat;
//        writer = std::unique_ptr<juce::AudioFormatWriter>(wavFormat.createWriterFor(fileStream.get(),
//                                                                                    sampleRate,
//                                                                                    (unsigned int) numChannels,
//                                                                                    16,
//                                                                                    {},
//                                                                                    0));
//        if (writer != nullptr){
//            DBG("File Write Successful");
//            return true;
//        }
//    }
//    DBG("File Write Failed");
//    return false;
//};

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

//void AudioToFileWriter::writeOutputToFile(const juce::AudioBuffer<float>& buffer){
////    make sure the writer is valid
//    if (writer != nullptr){
////        use buffer class methods to retrieve number of channels and samples associated with buffer
//        int numChannels = buffer.getNumChannels();
//        int numSamples = buffer.getNumSamples();
//        
////        unsure why getNumChannels is suggested in the documentation alongside writeFromAudioSampleBuffer, as the latter method already allows for multi-channel audio writing
//        if (numChannels > 0 && numSamples > 0 ){
//            writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);
//        }
//    }
//};

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
//void AudioToFileWriter::closeFile(){
////    check that writer is valid before closing
//    if(writer != nullptr){
////        release audioFormatWriter to finalize the file
//        DBG("Writer is valid, resetting writer");
//        writer->flush();
//        writer.reset();
//    }
//    else
//    {
//        DBG("Writer was null.");
//    }
////    ensure the file stream is valid before flush is called
//    if(fileStream != nullptr){
//        DBG("Flushing file stream.");
//        fileStream->flush();
////        this is redundant but should ensure the file stream is released before file is closed
//        fileStream.reset();
//    }
//    else{
//        DBG("File stream is already null, skipping flush.");
//    }
//};

//void AudioToFileWriter::closeFile()
//{
//    std::lock_guard<std::mutex> lock(fileMutex);  // Lock the mutex while closing
//
//    if (writer != nullptr)
//    {
//        writer.reset();  // Safely release the writer
//    }
//
//    if (fileStream != nullptr && fileStream->openedOk())  // Ensure fileStream is valid and open
//    {
//        DBG("Flushing file stream...");
//        fileStream->flush();  // Safely flush the stream
//        fileStream.reset();   // Release the file stream
//    }
//    else
//    {
//        DBG("File stream is null or not opened, skipping flush.");
//    }
//}

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
