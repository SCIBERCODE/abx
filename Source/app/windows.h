#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class window : public DocumentWindow {
public:
    window();
    ~window();
    void closeButtonPressed() override;
private:
    theme _theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(window)
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class window_audio_setup : public DocumentWindow {
public:
    window_audio_setup(AudioDeviceManager& device_manager);
    ~window_audio_setup();
    void closeButtonPressed() override;
private:
    theme _theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(window_audio_setup)
};


}