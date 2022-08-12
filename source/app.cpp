#include "../JuceLibraryCode/JuceHeader.h"

#include "app/windows.h"

class abx_application : public JUCEApplication {
public:

    void initialise(const String& command_line) override
    {
        ignoreUnused(command_line);
        //_window_tooltip->setMillisecondsBeforeTipAppears(1500);
        _window_tooltip->setLookAndFeel(&_theme);
    }

    void systemRequestedQuit() override
    {
        _window_tooltip->setLookAndFeel(nullptr);
        quit();
    }

    const String getApplicationName()              override { return "abx"; }
    const String getApplicationVersion()           override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed()              override { return false; }
    void shutdown()                                override { }
    void anotherInstanceStarted(const String& cmd) override { ignoreUnused(cmd); }

private:
    abx::window_main                     _window_main;
    SharedResourcePointer<TooltipWindow> _window_tooltip;
    abx::theme _theme;
};

START_JUCE_APPLICATION(abx_application)