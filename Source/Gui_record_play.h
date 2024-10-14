/*
  ==============================================================================

    gui_record_play.h
    Created: 11 Oct 2024 7:25:49pm
    Author:  David Matthew Welch

  ==============================================================================
*/

#pragma once

enum AppState {
    IDLE,
    PLAYING,
    RECORDING
};

extern AppState currentAppState;

class AudioToFileWriter {
public:
    AudioToFileWriter(){}
    bool setup(const juce::File& outputFile, int sampleRate, int numChannels);
    void writeOutputToFile(const juce::AudioBuffer<float>& buffer);
    void closeFile();
private:
    std::unique_ptr<juce::FileOutputStream> fileStream;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    
};

