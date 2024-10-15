#pragma once

#include <JuceHeader.h>
#include "gui_record_play.h"


class MainContentComponent   : public juce::AudioAppComponent,
                               public juce::ChangeListener,
                               public juce::Button::Listener,
                               public juce::Slider::Listener,
                               public juce::Timer

{
public:
    MainContentComponent();
    ~MainContentComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

private:
    AudioToFileWriter fileWriter;
//    AppState state;
    void openFile(bool forOutput);
    bool loadAudioFile(const juce::File &file);
    
    DisplayAudioWaveForm displayAudioWaveForm;
    juce::TextButton openButton, playButton, stopButton, recordButton;
    juce::Slider scrubber;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
