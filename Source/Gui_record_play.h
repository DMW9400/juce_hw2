/*
  ==============================================================================

    gui_record_play.h
    Created: 11 Oct 2024 7:25:49pm
    Author:  David Matthew Welch

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

enum AppState {
    IDLE,
    PLAYING,
    RECORDING
};

extern AppState currentAppState;
void changeState(AppState& state, juce::AudioTransportSource& transportSource, juce::TextButton& playButton, juce::TextButton& stopButton);

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

class DisplayAudioWaveForm : public juce::Component {
public:
    DisplayAudioWaveForm();
    ~DisplayAudioWaveForm() override;
    void addAudioData(const juce::AudioBuffer<float>& buffer,
                      int startSample, int numSamples);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::AudioVisualiserComponent audioVisualiser;
};
