#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::AudioAppComponent,
    private juce::OSCReceiver,
    private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>,
    private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void timerCallback() override;

    // Parametry potwora mapowane na DSP
    float movementSpeed = 0.0f;
    float actorScale = 1.0f;
    float healthPercentage = 1.0f;
    bool isInCombat = false;
    bool isAttacking = false;

    // Zmienne stanu syntezatora
    float currentSampleRate = 44100.0f;
    float mainPhase = 0.0f;
    float fmPhase = 0.0f;
    float lfoPhase = 0.0f;
    float lastFilterOut = 0.0f;
    float currentVolume = 0.0f;
    int samplesWithoutMessage = 0;

    juce::Random randomGenerator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};