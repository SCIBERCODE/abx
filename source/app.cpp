#include "../JuceLibraryCode/JuceHeader.h"

#include "app/windows.h"

class abx_application : public JUCEApplication {
public:

    void initialise(const String& command_line) override
    {
        ignoreUnused(command_line);

        int lang_size;
        auto lang = BinaryData::getNamedResource("russian_lng", lang_size);

        _lang = std::make_unique<LocalisedStrings>(CharPointer_UTF8(lang), false);
        //juce::LocalisedStrings::setCurrentMappings(_lang.release());

        _window_main = std::make_unique<abx::window_main>();

        _window_tooltip->setLookAndFeel(&_theme);
        LookAndFeel::setDefaultLookAndFeel(&_theme);
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
    std::unique_ptr<abx::window_main>    _window_main;
    std::unique_ptr <LocalisedStrings>   _lang;
    abx::theme                           _theme;
    SharedResourcePointer<TooltipWindow> _window_tooltip;
};

START_JUCE_APPLICATION(abx_application)