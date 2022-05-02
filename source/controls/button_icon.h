#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class button_icon : public Button {
public:

    button_icon() : Button("") {
        addAndMakeVisible(&_icon);
        setSize(19, 19);
    }

    ~button_icon() { };

    void paintButton(Graphics& g, bool hover, bool down) override {
        if (down)
            g.setColour(_colors.get(color_ids::button_pressed));
        else if (hover)
            g.setColour(_colors.get(color_ids::button_hover));
        else
            g.setColour(_colors.get(color_ids::button_normal));

        g.fillRect(getLocalBounds());
    }

    void resized() override {
        _icon.setBounds(5, 5, _icon.getWidth(), _icon.getHeight());
    }

    void set_icon(String icon_name) {
        if (icon_name != _icon_name) {
            _icon_name = icon_name;
            _icon.deleteAllChildren();
            _icon.addAndMakeVisible(resources::get_drawable(_icon_name, 9, 9));
        }
    }

private:
    colors            _colors;
    String            _icon_name { };
    DrawableComposite _icon;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(button_icon)
};

}