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
    /*class comp_viewport : public Component {
    public:
         comp_viewport(AudioDeviceManager& device_manager) : _settings(device_manager) { };
        ~comp_viewport() { };
    public:
        void resized() override {
            //auto area = getLocalBounds();
            _settings.setBounds(getLocalBounds());
        };
    private:
        comp_audio_settings _settings;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_viewport)
    };

    Viewport                       _viewport;
    std::unique_ptr<comp_viewport> _viewport_inside;*/

    window_audio_setup(AudioDeviceManager& device_manager);
    ~window_audio_setup();
    void closeButtonPressed() override;
private:
    theme _theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(window_audio_setup)
};


}