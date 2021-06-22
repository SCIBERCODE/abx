#include "../JuceLibraryCode/JuceHeader.h"
#include "window.h"

class abx_application : public JUCEApplication {
public:
    abx_application () { }
    ~abx_application() { }

    const String getApplicationName()    override { return "abx"; }
    const String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed()    override { return false; }

    void initialise(const String& /*command_line*/) override {
        _window_main = std::make_unique<abx::window>("abx");
    }

    void shutdown() override {
        _window_main = nullptr;
    }

    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const String& /*commandLine*/) override { }

private:
    std::unique_ptr<abx::window>         _window_main;
    SharedResourcePointer<TooltipWindow> _window_tooltip;
};

START_JUCE_APPLICATION(abx_application)
