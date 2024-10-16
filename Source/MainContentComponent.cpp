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
    transportSource.stop();
    transportSource.setSource(nullptr);
    shutdownAudio();
}

void MainContentComponent::changeState(AppState newState)
{
    if (state != newState)
    {
        state = newState;

        if (state == IDLE)
        {
            transportSource.stop();             // Ensure transport is stopped
            transportSource.setPosition(0.0);
            stopButton.setEnabled(false);
            playButton.setEnabled(true);
            scrubber.setEnabled(false);
            stopTimer();
        }
        else if (state == PLAYING)
        {
            stopButton.setEnabled(true);
            playButton.setEnabled(false);
            transportSource.start();
            scrubber.setEnabled(true);
            startTimerHz(30);
        }
        else if (state == RECORDING)
        {
            transportSource.stop();             // Ensure transport is stopped
            transportSource.setSource(nullptr); // Clear the source
            stopButton.setEnabled(true);
            playButton.setEnabled(false);
            scrubber.setEnabled(false);
        }
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
        if(state == RECORDING){
            fileWriter.closeFile();
            changeState(IDLE);
        }
        else if (transportSource.isPlaying()) {
            transportSource.stop();  // Stop the playback
            transportSource.setPosition(0.0);  // Reset the playhead to the beginning
            changeState(IDLE);
            DBG("Playback stopped and reset");
        }
    }
    else if (button == &recordButton){
        DBG("Record Button Pressed, opening file for output...");
        openFile(true);
//        changeState(RECORDING);
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
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    if (state == PLAYING)
    {
        transportSource.getNextAudioBlock(bufferToFill);
    }
    else if (state == RECORDING)
    {
        // Capture input audio when recording
        auto* device = deviceManager.getCurrentAudioDevice();
        if (device != nullptr)
        {
            auto activeInputChannels = device->getActiveInputChannels();
            const int maxInputChannels = activeInputChannels.getHighestBit() + 1;

            for (int channel = 0; channel < maxInputChannels; ++channel)
            {
                if (auto* inputChannelData = bufferToFill.buffer->getWritePointer(channel))
                {
                    // Add the input channel data to the waveform display
                    displayAudioWaveForm.addAudioData(*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples);

                    // Write the input audio to file
                    fileWriter.writeOutputToFile(*bufferToFill.buffer);
                }
            }
        }
    }
};

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
{
    chooser = std::make_unique<juce::FileChooser>(
        forOutput ? "Select a file to save recording..." : "Select a Wave file to play...",
        juce::File{},
        "*.wav");

    int chooserFlags = forOutput
        ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
        : juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this, forOutput](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file != juce::File{})
        {
            juce::String filePath = file.getFullPathName();

            if (!filePath.isEmpty())
            {
                // Stop the transport source before changing its source
                transportSource.stop();
                transportSource.setSource(nullptr);

                if (forOutput)  // Recording mode
                {
                    if (fileWriter.setup(file, 44100, 1))  // Adjust sample rate and channels as needed
                    {
                        DBG("Recording to file: " << filePath);
                        changeState(RECORDING);  // Start recording after successful setup
                    }
                    else
                    {
                        DBG("Failed to set up recording.");
                    }
                }
                else  // Playback mode
                {
                    if (loadAudioFile(file))
                    {
                        playButton.setEnabled(true);
                        DBG("Playing File: " << filePath);
                    }
                    else
                    {
                        DBG("Failed to load audio file.");
                    }
                }
            }
            else
            {
                DBG("Invalid file path, operation aborted.");
            }
        }
        else
        {
            DBG("No file selected.");
        }
    });
}

bool MainContentComponent::loadAudioFile(const juce::File &file)
{
    // Stop the transport source before changing its source
    transportSource.stop();
    transportSource.setSource(nullptr);

    // Create an AudioFormatReader for the file
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        // Clear prior sources to prevent issues
        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

        // Set scrubber range and enable
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
