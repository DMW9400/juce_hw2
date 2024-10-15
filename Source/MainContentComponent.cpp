#include <JuceHeader.h>
#include "MainContentComponent.h"
#include "gui_record_play.h"

MainContentComponent::MainContentComponent()
{
    addAndMakeVisible(displayAudioWaveForm);
    
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.addListener(this);


    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.addListener(this);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);
    
    addAndMakeVisible(&recordButton);
    recordButton.setButtonText("Record");
    recordButton.addListener(this);
    
    addAndMakeVisible(scrubber);
    scrubber.setEnabled(false);
    scrubber.setRange(0.0, 1.0);
    scrubber.setEnabled(false);
    scrubber.addListener(this);

    setSize(600, 400);

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
        if(state == PLAYING){
            transportSource.stop();
        }
        openFile(false);
        changeState(IDLE, transportSource, playButton, stopButton);
        }
    else if (button == &playButton){
        if (transportSource.isPlaying()) {
            transportSource.stop();  // If it's already playing, stop it
        }
        else {
            transportSource.start();  // Start playing the loaded file
            changeState(PLAYING, transportSource, playButton, stopButton);
            DBG("Playback started");
        }
    }
    else if (button == &stopButton){
        if (transportSource.isPlaying()) {
                    transportSource.stop();  // Stop the playback
                }
        transportSource.setPosition(0.0);  // Reset the playhead to the beginning
        changeState(IDLE, transportSource, playButton, stopButton);
        DBG("Playback stopped and reset");
    }
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
    
    displayAudioWaveForm.setBounds(10, 100, getWidth() - 20, getHeight() - 120);

}

void MainContentComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            state = PLAYING;
        else
            state = IDLE;
    }
}

void MainContentComponent::timerCallback(){
    
}
void MainContentComponent::sliderValueChanged(juce::Slider* slider){}

void MainContentComponent::openFile(bool forOutput)

//forOutput = save to file mode
//!forOutput = open file mode
{
    chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                  juce::File{},
                                                  "*.wav");                     // [7]
    int chooserFlags;
    if (forOutput){
        chooserFlags = juce::FileBrowserComponent::saveMode
        | juce::FileBrowserComponent::canSelectFiles;
    }
    else {
        chooserFlags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;
    }

    chooser->launchAsync(chooserFlags, [this, forOutput](const juce::FileChooser& fc)       // [8]
    {
        auto file = fc.getResult();

        if (file != juce::File{})                                         // [9]
        {
            if (forOutput){
                if (fileWriter.setup(file, 44100, 1))  // Mono, 44.1kHz
                                {
                                    DBG("Recording to file: " << file.getFullPathName());
                                    changeState(RECORDING, transportSource, playButton, stopButton);
                                }
            } 
//            logic for open file mode
            
            else{
                if(loadAudioFile(file)){
                    playButton.setEnabled(true);
                    DBG("Playing File: " << file.getFullPathName());
                }
                else{
                    DBG("Failed to load audio file.");
                }
            }
//            else {
//                
//                auto* reader = formatManager.createReaderFor(file);                 // [10]
//                
//                if (reader != nullptr)
//                {
//                    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
//                    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
//                    playButton.setEnabled(true);                                                      // [13]
//                    readerSource.reset(newSource.release());   
//                    DBG("Playing file: " << file.getFullPathName());// [14]
//                }
//            }
        }
    });
}

bool MainContentComponent::loadAudioFile(const juce::File &file){
    // Create an AudioFormatReader for the file
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        // Load the file data into the waveform display
        juce::AudioBuffer<float> buffer((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
        
        // Push the buffer data to the waveform visualizer
        displayAudioWaveForm.addAudioData(buffer, 0, (int)reader->lengthInSamples);

        return true;  // Successfully loaded the file into the visualizer
    }

    return false;  // Failed to load the file
};
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
