#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

// todo #1
namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class button_icon : public Button {
public:

    button_icon() : Button(String {}) {
        addAndMakeVisible(&_icon);
        setSize(19, 19);
    }

    void paintButton(Graphics& g, bool hover, bool down) override {
        if (down)
            g.setColour(_colours.get(colour_id::button_pressed));
        else if (hover)
            g.setColour(_colours.get(colour_id::button_hover));
        else
            g.setColour(_colours.get(colour_id::button_normal));

        g.fillRect(getLocalBounds());
    }

    void resized() override {
        _icon.setBounds(5, 5, _icon.getWidth(), _icon.getHeight());
    }

    void set_icon(icon_id id) {
        _icon.deleteAllChildren();
        _icon.addAndMakeVisible(resources::get_drawable(id, 9).release());
    }

private:
    colours           _colours;
    DrawableComposite _icon;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(button_icon)
};

}