#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "theme.h"
#include "components.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class window : public DocumentWindow {
public:
    window(String name = { })
        : DocumentWindow(
            name,
            Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::allButtons
        )
    {
        setLookAndFeel(&_theme);
        setTitleBarHeight(20);
        setUsingNativeTitleBar(false);
        auto main_comp = std::make_unique<comp_main>();
        setContentOwned(main_comp.release(), true);
        setResizable(true, true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
        setResizeLimits(504, 240, 1920 * 2, 1080 * 2);
    }

    ~window() {
        setLookAndFeel(nullptr);
    };
    void closeButtonPressed() override {
        JUCEApplication::getInstance()->systemRequestedQuit();
    };

private:
    theme _theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(window)
};

}
