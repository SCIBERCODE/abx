#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "theme.h"

namespace abx {

class window :
    public DocumentWindow
{
public:
     window(Component* comp_owned, const String& caption = {});
    ~window();

private:
    theme _theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(window)
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class window_main :
    public window
{
public:
    window_main();
    void closeButtonPressed() override;
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class window_audio_setup :
    public window
{
public:
    window_audio_setup(AudioDeviceManager& device_manager);
    void closeButtonPressed() override;
};

}