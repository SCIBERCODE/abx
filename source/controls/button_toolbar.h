#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class button_toolbar : public Button {
public:
    enum class border_radius_side_t {
        none,
        left,
        right,
        all
    };

    enum class button_t {
        normal,
        ftdi,
        utility
    };

public:
    button_toolbar(const String& text = String()) :
        Button(String()),
        _text(text)
    {
        _icon.setSize(16, 16);
        addAndMakeVisible(_icon);
        setSize(get_size(), get_size());
    }

    void set_icon(icon_id id, float size = margins::_icon_small) {
        _icon.deleteAllChildren();
        _icon.addAndMakeVisible(resources::get_drawable(id, size).release());
        resized();
    }

    void set_border_radius_side(border_radius_side_t side) {
        if (side != _border_radius_side) {
            _border_radius_side = side;
            repaint();
        }
    }

    int get_size() const {
        return 32;
    }

    void hard_press(bool should_be_pressed, bool wrong) {
        _hard_pressed.first  = should_be_pressed;
        _hard_pressed.second = wrong;
    }

    void stress(bool should_be_stressed) {
        _stressed = should_be_stressed;
    }

    void paintButton(Graphics& g, bool highlighted, bool down) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(.5f, .5f);
        const auto corner_size = 6.f;

        if (down)
            g.setColour(_colours.get(colour_id::button_pressed));
        else if (highlighted)
            g.setColour(_colours.get(colour_id::button_hover));
        else
            g.setColour(_colours.get(
                _type == button_t::ftdi ? colour_id::header : colour_id::button_normal));

        if (_hard_pressed.first)
            g.setColour(_colours.get(_hard_pressed.second ? colour_id::button_red : colour_id::button_green));

        Path path;
        path.addRoundedRectangle(bounds.getX(), bounds.getY(),
            bounds.getWidth(), bounds.getHeight(),
            corner_size, corner_size,
            _border_radius_side == border_radius_side_t::left  || _border_radius_side == border_radius_side_t::all,
            _border_radius_side == border_radius_side_t::right || _border_radius_side == border_radius_side_t::all,
            _border_radius_side == border_radius_side_t::left  || _border_radius_side == border_radius_side_t::all,
            _border_radius_side == border_radius_side_t::right || _border_radius_side == border_radius_side_t::all);

        g.fillPath(path);

        if (_type == button_t::utility)
            g.setColour(_colours.get(colour_id::outline_dark).brighter(.2f));
        else {
            auto outline = _colours.get(colour_id::outline);
            g.setColour(isEnabled() ? outline : outline.brighter());
        }
        if (_stressed)
            g.setColour(Colours::black);

        g.strokePath(path, PathStrokeType(_stressed ? 1.5f : 1.f));

        if (getToggleState()) {
            g.setColour(_colours.get(colour_id::button_pressed));
            g.fillPath(path);
            g.setColour(_colours.get(colour_id::button_green));
            g.reduceClipRegion(path);
            g.fillRect(juce::Rectangle<float>(bounds.getX(), bounds.getBottom() - 3.f, bounds.getWidth(), 3.f));
        }

        if (_text.length()) {
            g.setColour(isEnabled() ? Colours::black : _colours.get(colour_id::text_disabled));
            g.setFont(Font(20, Font::FontStyleFlags::plain));
            g.drawText(_text, bounds, Justification::centred);
        }
    }

    void resized() override
    {
        const auto global_bounds = getBounds();
        _icon.setBounds((global_bounds.getWidth() - _icon.getWidth()) / 2,
            (global_bounds.getHeight() - _icon.getHeight()) / 2,
            _icon.getWidth(), _icon.getHeight());
    }

    void enablementChanged() override {
        Button::enablementChanged();
        _icon.setAlpha(isEnabled() ? 1.f : .2f);
    }

    void clicked() override {
        Button::clicked();
        resized();
    }

    void set_type(const button_t type) {
        _type = type;
    }

private:
    colours               _colours;
    DrawableComposite     _icon;
    String                _text               {};
    border_radius_side_t  _border_radius_side { border_radius_side_t::none   };
    button_t              _type               { button_t::normal             };
    std::pair<bool, bool> _hard_pressed       { std::make_pair(false, false) };
    bool                  _stressed           {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(button_toolbar)
};

}