#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_audio_settings : public Component
{
public:
    comp_audio_settings(AudioDeviceManager& device_manager)
        : _comp_audio_setup(device_manager, 0, 256, 0, 256, false, false, false, false)
    {
        _viewport.setScrollBarThickness(11);
        _viewport.setViewedComponent(&_comp_audio_setup, false);
        _viewport.setScrollBarsShown(true, false);
        addAndMakeVisible(_viewport);
        setSize(520, 400);
    }
    void paint(Graphics& g) override {
        g.fillAll(_colors.get(color_ids::bg_light));
    }
    void resized() override {
        _comp_audio_setup.setBounds(getLocalBounds());
        _viewport.setBounds(getLocalBounds());
    }
private:
    Viewport                     _viewport;
    AudioDeviceSelectorComponent _comp_audio_setup;
    colors                       _colors;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_audio_settings)
};

}