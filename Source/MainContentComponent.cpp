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
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    stopButton.setEnabled(false);
    
    addAndMakeVisible(&recordButton);
    recordButton.setButtonText("Record");
    recordButton.addListener(this);
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
//    recordButton.setEnabled()
    
    addAndMakeVisible(scrubber);
    scrubber.setEnabled(false);
    scrubber.setRange(0.0, 1.0);
    scrubber.addListener(this);

    setSize(600, 400);

    formatManager.registerBasicFormats();       // [1]
    transportSource.addChangeListener(this);    // [2]

    setAudioChannels(1, 2);
}

MainContentComponent::~MainContentComponent()
{
    shutdownAudio();
    transportSource.setSource(nullptr);
}

void MainContentComponent::changeState(AppState newState)
{
    state = newState;

    if (state == IDLE)
    {
        stopButton.setEnabled(false);
        playButton.setEnabled(true);
//        reset playhead
        transportSource.setPosition(0.0);
        scrubber.setEnabled(false);
        stopTimer();
    }
    else if (state == PLAYING)
    {
        stopButton.setEnabled(true);
        playButton.setEnabled(false);
        transportSource.start();
        scrubber.setEnabled(true);
//        set timer to update 30 times per sec
        startTimerHz(30);
    }
    else if (state == RECORDING)
    {
        stopButton.setEnabled(true);
        playButton.setEnabled(false);
//        transportSource.stop();
        scrubber.setEnabled(false);
//        stopTimer();
    }
}

void MainContentComponent::buttonClicked(juce::Button* button){
    if (button == &openButton){
        if(state == PLAYING){
            transportSource.stop();
        }
        openFile(false);
        changeState(IDLE);
        }
    else if (button == &playButton){
        if (transportSource.isPlaying()) {
            transportSource.stop();  // If it's already playing, stop it
        }
        else {
            transportSource.start();  // Start playing the loaded file
            changeState(PLAYING);
            DBG("Playback started");
        }
    }
    else if (button == &stopButton){
        if (transportSource.isPlaying()) {
                    transportSource.stop();  // Stop the playback
                }
        transportSource.setPosition(0.0);  // Reset the playhead to the beginning
        changeState(IDLE);
        DBG("Playback stopped and reset");
    }
    else if (button == &recordButton){
        openFile(true);
        changeState(RECORDING);
    }
}

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainContentComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (state == IDLE)
    {
        // Clear the buffer if we're not recording or playing
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    if (state == PLAYING)
    {
        // If we're playing, fill the buffer with data from the transport source
        transportSource.getNextAudioBlock(bufferToFill);
    }
    else if (state == RECORDING)
    {
        // Capture input audio when recording
        auto* device = deviceManager.getCurrentAudioDevice();
        if (device != nullptr)
        {
            auto activeInputChannels = device->getActiveInputChannels();
            auto activeOutputChannels = device->getActiveOutputChannels();
            
            const int maxInputChannels = activeInputChannels.getHighestBit() + 1;
            const int maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

            for (int channel = 0; channel < maxOutputChannels; ++channel)
            {
                if (!activeOutputChannels[channel])
                {
                    bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
                }
            }

            for (int channel = 0; channel < maxInputChannels; ++channel)
            {
                if (auto* inputChannelData = bufferToFill.buffer->getWritePointer(channel))
                {
                    // Add the input channel data to the waveform display
                    displayAudioWaveForm.addAudioData(*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples);
                    
                    // Write the input audio to file (optional)
                    fileWriter.writeOutputToFile(*bufferToFill.buffer);
                }
            }
        }
    }
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
    recordButton.setBounds(10, 100, getWidth() - 20, 20);
    
    scrubber.setBounds(10, 130, getWidth() - 20, 20);
    
    displayAudioWaveForm.setBounds(10, 160, getWidth() - 20, getHeight() - 160);

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
    if (state == PLAYING){
        scrubber.setValue(transportSource.getCurrentPosition(), juce::dontSendNotification);
    }
}
void MainContentComponent::sliderValueChanged(juce::Slider* slider){
    if (slider == &scrubber && (state == PLAYING || state == IDLE)){
        transportSource.setPosition(scrubber.getValue());
    }
}

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
//          clean transportSource in the event a previous file was loaded
            transportSource.stop();
            transportSource.setSource(nullptr);
            if (forOutput){
                if (fileWriter.setup(file, 44100, 1))  // Mono, 44.1kHz
                                {
                                    DBG("Recording to file: " << file.getFullPathName());
//                                    changeState(RECORDING, transportSource, playButton, stopButton);
                                    changeState(RECORDING);
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
        }
    });
}

bool MainContentComponent::loadAudioFile(const juce::File &file){
    // Create an AudioFormatReader for the file
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
//        clear prior sources to prevent issues
        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
        
//        correctly set scrubber range and enable
        scrubber.setRange(0.0, transportSource.getLengthInSeconds());
        scrubber.setEnabled(true);
        // Load the file data into the waveform display
        juce::AudioBuffer<float> buffer((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);
        
        // Push the buffer data to the waveform visualizer
        displayAudioWaveForm.addAudioData(buffer, 0, (int)reader->lengthInSamples);

        return true;  // Successfully loaded the file into the visualizer
    }

    return false;  // Failed to load the file
};

