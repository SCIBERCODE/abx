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
        addAndMakeVisible(_comp_audio_setup);
        setSize(520, 450);
    }
    void paint(Graphics& g) override {
        g.fillAll(_colors.get(color_ids::bg_light));
    }
    void resized() override {
        _comp_audio_setup.setBounds(getLocalBounds()); // todo: [5]
    }
private:
    AudioDeviceSelectorComponent _comp_audio_setup;
    colors                       _colors;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_audio_settings)
};

}