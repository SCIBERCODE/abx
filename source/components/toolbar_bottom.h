#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_toolbar_bottom : public Component {
public:
    comp_toolbar_bottom() {
        setOpaque(true);

        addAndMakeVisible(_labels[0]);
        addAndMakeVisible(_labels[1]);

        _warning = resources::get_drawable(icon_id::warning, margins::_icon_small);
        _ok      = resources::get_drawable(icon_id::ok, margins::_icon_small);

        set_device_audio({});
        set_device_ftdi ({});
    };

    void paint(Graphics& g) override {
        g.fillAll(_colours.get(colour_id::bg_light));
        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawLine(0.f, .5f, static_cast<float>(getBounds().getWidth()), .5f, 1.f);

        auto area = getLocalBounds();
        area.reduce(margins::_medium, 3);

        _icons[0] = area.removeFromLeft(margins::_icon_small).toFloat();
        auto width = _labels[0].getFont().getStringWidth(_labels[0].getText());
        _labels[0].setBounds(area.removeFromLeft(width + margins::_small));

        _icons[1] = area.removeFromLeft(margins::_icon_small).toFloat();
        _labels[1].setBounds(area);

        for (int k = 0; k < 2; k++) {
            Drawable* icon = _devices[k].length() ? _ok.get() : _warning.get();
            icon->drawAt(g, _icons[k].getX(), _icons[k].getY(), 1.f);
        }
    };

    void set_device_audio(const String& audio) {
        _labels[1].setText(audio.length() ? audio : "No audio device", sendNotificationAsync);
        _devices[1] = audio;
        repaint();
    }

    void set_device_ftdi(const String& ftdi) {
        _labels[0].setText(ftdi.length() ? ftdi : "No remote control", sendNotificationAsync);
        _devices[0] = ftdi;
        repaint();
    }

private:
    String                    _devices[2];
    Label                     _labels[2];
    std::unique_ptr<Drawable> _warning,
                              _ok;
    juce::Rectangle<float>    _icons[2];
    colours                   _colours;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_toolbar_bottom)
};

}
