#include <JuceHeader.h>
#include "MainContentComponent.h"

MainContentComponent::MainContentComponent()
    : state(Stopped)
{
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
//    openButton.onClick = [this] { openButtonClicked(); };

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
//    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
//    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);

    setSize(300, 200);

    formatManager.registerBasicFormats();       // [1]
    transportSource.addChangeListener(this);    // [2]

    setAudioChannels(1, 2);
}

MainContentComponent::~MainContentComponent()
{
    shutdownAudio();
}

void MainContentComponent::buttonClicked(juce::Button* button){
    if (button == &openButton){
        }
    else if (button == &playButton){}
    else if (button == &stopButton){}
}

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainContentComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);
}

void MainContentComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainContentComponent::resized()
{
    openButton.setBounds(10, 10, getWidth() - 20, 20);
    playButton.setBounds(10, 40, getWidth() - 20, 20);
    stopButton.setBounds(10, 70, getWidth() - 20, 20);
}

void MainContentComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState(Playing);
        else
            changeState(Stopped);
    }
}

void MainContentComponent::changeState(MainContentComponent::TransportState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
            case Stopped:                           // [3]
                stopButton.setEnabled(false);
                playButton.setEnabled(true);
                transportSource.setPosition(0.0);
                break;

            case Starting:                          // [4]
                playButton.setEnabled(false);
                transportSource.start();
                break;

            case Playing:                           // [5]
                stopButton.setEnabled(true);
                break;

            case Stopping:                          // [6]
                transportSource.stop();
                break;
        }
    }
}

void MainContentComponent::timerCallBack(){
    
}

//void MainContentComponent::openButtonClicked()
//{
//    chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
//                                                  juce::File{},
//                                                  "*.wav");                     // [7]
//    auto chooserFlags = juce::FileBrowserComponent::openMode
//                      | juce::FileBrowserComponent::canSelectFiles;
//
//    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)       // [8]
//    {
//        auto file = fc.getResult();
//
//        if (file != juce::File{})                                               // [9]
//        {
//            auto* reader = formatManager.createReaderFor(file);                 // [10]
//
//            if (reader != nullptr)
//            {
//                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
//                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
//                playButton.setEnabled(true);                                                      // [13]
//                readerSource.reset(newSource.release());                                          // [14]
//            }
//        }
//    });
//}

//void MainContentComponent::playButtonClicked()
//{
//    changeState(Starting);
//}
//
//void MainContentComponent::stopButtonClicked()
//{
//    changeState(Stopping);
//}
