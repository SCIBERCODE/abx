#include "../JuceLibraryCode/JuceHeader.h"

#include "app/windows.h"

class abx_application : public JUCEApplication {
public:
     abx_application() { }
    ~abx_application() { }

    const String getApplicationName()              override { return "abx"; }
    const String getApplicationVersion()           override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed()              override { return false; }
    void initialise(const String& command_line)    override { ignoreUnused(command_line); }
    void shutdown()                                override { }
    void systemRequestedQuit()                     override { quit(); }
    void anotherInstanceStarted(const String& cmd) override { ignoreUnused(cmd); }

private:
    abx::window_main                     _window_main;
    SharedResourcePointer<TooltipWindow> _window_tooltip;
};

START_JUCE_APPLICATION(abx_application)