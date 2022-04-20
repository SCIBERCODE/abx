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

    button_toolbar() : Button("") {
        _icon.setSize(16, 16);
        addAndMakeVisible(_icon);
        const auto default_size = get_size();
        setSize(default_size.first, default_size.second);
    }
    ~button_toolbar() { };

    void set_icon(String icon_name, int width = 14, int height = 14) {
        if (icon_name != _icon_name) {
            _icon_name = icon_name;
            _icon.deleteAllChildren();
            _icon.addAndMakeVisible(get_drawable(_icon_name, width, height));
            resized();
        }
    }

    void set_border_radius_side(border_radius_side_t side) {
        if (side != _border_radius_side) {
            _border_radius_side = side;
            repaint();
        }
    }

    std::pair<int, int> get_size() const {
        return std::make_pair(32, 32);
    }

    void paintButton(Graphics& g, bool highlighted, bool down) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(.5f, .5f);
        const auto corner_size = 6.f;

        if (down)
            g.setColour(_colors.get(color_ids::button_pressed));
        else if (highlighted)
            g.setColour(_colors.get(color_ids::button_hover));
        else
            g.setColour(_colors.get(_type == button_t::ftdi ? color_ids::header : color_ids::button_normal));

        if (hard_pressed) {
            g.setColour(_colors.get(color_ids::button_toogled));
        }

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
            g.setColour(_colors.get(color_ids::outline_dark).brighter(.2f));
        else {
            auto outline = _colors.get(color_ids::outline);
            g.setColour(isEnabled() ? outline : outline.brighter());
        }
        g.strokePath(path, PathStrokeType(1.f));

        if (getToggleState()) {
            g.setColour(_colors.get(color_ids::button_pressed));
            g.fillPath(path);
            g.setColour(_colors.get(color_ids::button_toogled));
            g.reduceClipRegion(path);
            g.fillRect(juce::Rectangle<float>(bounds.getX(), bounds.getBottom() - 3.f, bounds.getWidth(), 3.f));
        }

        if (_text.length()) {
            g.setColour(isEnabled() ? Colours::black : _colors.get(color_ids::text_disabled));
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

    void set_text(const String& new_text) {
        _text = new_text;
    }

    void set_type(const button_t type) {
        _type = type;
    }

    bool hard_pressed{ false };

private:
    colors               _colors;
    DrawableComposite    _icon;
    String               _icon_name          { },
                         _text               { };
    border_radius_side_t _border_radius_side { border_radius_side_t::none };
    button_t             _type               { button_t::normal           };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(button_toolbar)
};

}