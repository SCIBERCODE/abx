#include "windows.h"
#include "../components/main.h"
#include "../components/audio_settings.h"

namespace abx {

window::window(Component* comp_owned, const String& caption) :
    DocumentWindow(
        JUCEApplication::getInstance()->getApplicationName() + " " + caption,
        Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
        DocumentWindow::allButtons)
{
    setLookAndFeel(&_theme);
    setTitleBarHeight(margins::_line);
    setUsingNativeTitleBar(false);
    setContentOwned(comp_owned, true);
    setResizable(true, true);
    centreWithSize(getWidth(), getHeight());
}

window::~window() {
    setLookAndFeel(nullptr);
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/

window_main::window_main() :
    window(std::make_unique<comp_main>().release())
{
    auto area = Desktop::getInstance().getDisplays().getTotalBounds(true);
    setResizeLimits(504, 240, area.getWidth(), area.getHeight());
    setVisible(true);

    if (auto* content = dynamic_cast<comp_main*>(getContentComponent()))
        addKeyListener(content->get_keys()->getKeyMappings());
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/

window_audio_setup::window_audio_setup(AudioDeviceManager& device_manager) :
    window(std::make_unique<comp_audio_settings>(device_manager).release(), "audio settings")
{
    addToDesktop();
}

void window_main       ::closeButtonPressed() { JUCEApplication::getInstance()->systemRequestedQuit(); }
void window_audio_setup::closeButtonPressed() { exitModalState(0); }

}