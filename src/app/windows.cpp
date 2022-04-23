#include "windows.h"
#include "../components/main.h"
#include "../components/audio_settings.h"

namespace abx {

window::window()
    : DocumentWindow(
        "abx",
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

window::~window() {
    setLookAndFeel(nullptr);
};

void window::closeButtonPressed() {
    JUCEApplication::getInstance()->systemRequestedQuit();
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
window_audio_setup::window_audio_setup(AudioDeviceManager& device_manager)
    : DocumentWindow(
        "abx audio settings",
        Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
        DocumentWindow::allButtons
    )
{
    setLookAndFeel(&_theme);
    setTitleBarHeight(20);
    setUsingNativeTitleBar(false);
    auto main_comp = std::make_unique<comp_audio_settings>(device_manager);
    setContentOwned(main_comp.release(), true);
    setResizable(true, true);
    centreWithSize(getWidth(), getHeight());
    addToDesktop();
}

window_audio_setup::~window_audio_setup() {
    setLookAndFeel(nullptr);
}
void window_audio_setup::closeButtonPressed() {
    exitModalState(0);
}

}