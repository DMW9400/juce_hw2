#pragma once

#include <JuceHeader.h>

class MainContentComponent   : public juce::AudioAppComponent,
                               public juce::ChangeListener,
                               public juce::Button::Listener
{
public:
    MainContentComponent();
    ~MainContentComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
//    void buttonClicked(juce::Button* button) override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState(TransportState newState);
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
