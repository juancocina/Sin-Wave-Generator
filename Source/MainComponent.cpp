#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 200);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2); //no input, 2 output
    }
    //adding Sliders
    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(50, 5000.0);
    frequencySlider.setTextValueSuffix("Hz");
    frequencySlider.addListener(this);
    frequencySlider.setSkewFactorFromMidPoint (500.0);
    frequencySlider.setValue (currentFrequency, juce::dontSendNotification); 
    addAndMakeVisible(frequencyLabel);
    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.attachToComponent(&frequencySlider, true);
    
    addAndMakeVisible (durationSlider);
    durationSlider.setRange (1.0 / frequencySlider.getMaximum(),
                             1.0 / frequencySlider.getMinimum());
    durationSlider.setTextValueSuffix (" s");
    durationSlider.addListener (this);
    durationSlider.onValueChange = [this] { frequencySlider.setValue (1.0 / durationSlider.getValue(), juce::dontSendNotification); };

    addAndMakeVisible (durationLabel);
    durationLabel.setText ("Duration", juce::dontSendNotification);
    durationLabel.attachToComponent (&durationSlider, true);

    //levelSlider from white noise volume tutorial;
    addAndMakeVisible(levelSlider);
    levelSlider.setRange(0, 0.25);
    levelSlider.addListener(this);
    
    //setting Values
    frequencySlider.setValue(500);
    levelSlider.setValue(0);
    
    
    //changing slider style
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, frequencySlider.getTextBoxHeight());
    durationSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, durationSlider.getTextBoxHeight());
    //Skewing Slider Values
    //frequencySlider.setSkewFactorFromMidPoint (500);
    durationSlider .setSkewFactorFromMidPoint (0.002);

}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    juce::String message;
    message << "Prepare to play audio...\n";
    message << " Samples per block expected = " << samplesPerBlockExpected << "\n";
    message << " sampleRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);
    
    currentSampleRate = sampleRate;
    updateAngleDelta();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!
    //adding a slider for white noise
    //auto level = (float) levelSlider.getValue();
    //juce::Logger::getCurrentLogger()->writeToLog(std::to_string(level));
    //auto levelScale = level * 2.0f;
    
//    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel){
//        //Get a pointer to start sample in the buffer for this audio output channel
//        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
//
//        //Fill the required number of samples with noise between -0.125 and +0.125
//        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample){
//            buffer[sample] = random.nextFloat() * levelScale - level;
//        }
//    }
    
    //frequency and duration slider output
    auto level2 = (float) levelSlider.getValue();
    auto* leftBuffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    
    auto localTargetFrequency = targetFrequency;
    
    if(localTargetFrequency != currentFrequency){
        auto frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples;
        
        for(auto sample = 0; sample < bufferToFill.numSamples; ++sample){
            auto currentSample = (float) std::sin (currentAngle);
            currentFrequency += frequencyIncrement;
            updateAngleDelta();
            currentAngle += angleDelta;
            leftBuffer[sample] = currentSample * level2;
            rightBuffer[sample] = currentSample * level2;
        }
        currentFrequency = localTargetFrequency;
    }
    else{
        for(auto sample = 0; sample < bufferToFill.numSamples; ++sample){
            auto currentSample = (float) std::sin (currentAngle);
            currentAngle += angleDelta;
            leftBuffer[sample] = currentSample * level2;
            rightBuffer[sample] = currentSample * level2;
        }
    }
    
    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    //bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    juce::Logger::getCurrentLogger()->writeToLog ("Releasing audio resources");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    auto sliderLeft = 100;
    levelSlider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
    frequencySlider.setBounds(sliderLeft, 50, getWidth() - sliderLeft - 10, 20);
    durationSlider.setBounds(sliderLeft, 80, getWidth() - sliderLeft - 10, 20);
}

void MainComponent::sliderValueChanged(juce::Slider* slider) {
    if (slider == &frequencySlider)
        durationSlider.setValue (1.0 / frequencySlider.getValue(), juce::dontSendNotification);
    else if (slider == &durationSlider)
        frequencySlider.setValue (1.0 / durationSlider.getValue(), juce::dontSendNotification);
    if(slider == &levelSlider){
        levelSlider.setValue(levelSlider.getValue(), juce::dontSendNotification);
    }
    
    frequencySlider.onValueChange = [this]
    {
        if(currentSampleRate > 0.0) updateAngleDelta();
    };
}

void MainComponent::updateAngleDelta(){
    auto cyclesPerSample = frequencySlider.getValue() / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}
