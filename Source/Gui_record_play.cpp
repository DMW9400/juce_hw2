/*
  ==============================================================================

    gui_record_play.cpp
    Created: 11 Oct 2024 7:25:39pm
    Author:  David Matthew Welch

  ==============================================================================
*/
#include <JuceHeader.h>
#include "gui_record_play.h"

bool AudioToFileWriter::setup(const juce::File& outputFile, int sampleRate, int numChannels){
    
//    check if outputFile already exists, then delete if so
    if (outputFile.existsAsFile())
       {
           outputFile.deleteFile();
       }
//use FileOutputStream's createOutputStream method where audio data can be stored
    fileStream = outputFile.createOutputStream();
//    ensure filestream is correctly created and not pointing erroneously
    if (fileStream != nullptr){
//      reset the writer so it is ready for a new fileStream
        writer.reset();
        
        juce::WavAudioFormat wavFormat;
        writer = std::unique_ptr<juce::AudioFormatWriter>(wavFormat.createWriterFor(fileStream.get(),
                                                                                    sampleRate,
                                                                                    (unsigned int) numChannels,
                                                                                    16,
                                                                                    {},
                                                                                    0));
        
    }
    
    return false;
};
