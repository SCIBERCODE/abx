#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../controls/button_toolbar.h"
#include "../controls/slider.h"
#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_toolbar : public Component
{
public:
    enum class button_t {
        rev, a, hz, b, fwd,
        restart, blind,
        pause, play, stop, rewind,
        open
    };

    void connect(bool shoud_be_connected) {
        _connected = shoud_be_connected;
        _button_rev.setEnabled(_connected);
        _button_a  .setEnabled(_connected);
        _button_hz .setEnabled(_connected);
        _button_b  .setEnabled(_connected);
        _button_fwd.setEnabled(_connected);
    }

    comp_toolbar() { // todo: [9]
        setOpaque(true);

        // user input
        _button_rev.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_rev.set_icon(icons_ids::backward);
        _button_rev.set_type(button_toolbar::button_t::ftdi);
        addAndMakeVisible(_button_rev);

        _button_a.set_text("A");
        _button_a.onClick = [this]() {
            std::bitset<8>btn;
            btn.set(btn_a);
            btn.flip();
            _choose_clicked(static_cast<uint8_t>(btn.to_ulong()));
            _choose_clicked(0xff);
        };
        _button_a.set_type(button_toolbar::button_t::ftdi);
        addAndMakeVisible(_button_a);

        _button_hz.set_text("?");
        _button_hz.onClick = [this]() {
            std::bitset<8>btn;
            btn.set(btn_hz);
            btn.flip();
            _choose_clicked(static_cast<uint8_t>(btn.to_ulong()));
            _choose_clicked(0xff);
        };
        _button_hz.set_type(button_toolbar::button_t::ftdi);
        addAndMakeVisible(_button_hz);

        _button_b.set_text("B");
        _button_b.onClick = [this]() {
            std::bitset<8>btn;
            btn.set(btn_b);
            btn.flip();
            _choose_clicked(static_cast<uint8_t>(btn.to_ulong()));
            _choose_clicked(0xff);
        };
        _button_b.set_type(button_toolbar::button_t::ftdi);
        addAndMakeVisible(_button_b);

        _button_fwd.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_fwd.set_icon(icons_ids::forward);
        _button_fwd.set_type(button_toolbar::button_t::ftdi);
        addAndMakeVisible(_button_fwd);

        // settings
        _button_restart.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_restart.set_icon(icons_ids::restart, 16.f);
        _button_restart.setTooltip("Restart Audio");
        _button_restart.setClickingTogglesState(true);
        _button_restart.setToggleState(true, dontSendNotification);
        addAndMakeVisible(_button_restart);

        _button_blind.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_blind.set_icon(icons_ids::blind, 18.f);
        _button_blind.setTooltip("Blind Mode");
        _button_blind.setClickingTogglesState(true);
        _button_blind.setToggleState(true, dontSendNotification);
        addAndMakeVisible(_button_blind);

        // player
        _button_pause.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_pause.set_icon(icons_ids::pause);
        _button_pause.setClickingTogglesState(true);
        addAndMakeVisible(_button_pause);
        _button_pause.setEnabled(false);

        _button_play.set_icon(icons_ids::play);
        _button_play.setClickingTogglesState(true);
        addAndMakeVisible(_button_play);
        _button_play.setEnabled(false);

        _button_stop.set_icon(icons_ids::stop, 12.f);
        addAndMakeVisible(_button_stop);
        _button_stop.setEnabled(false);

        _button_rewind.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_rewind.set_icon(icons_ids::rewind);
        _button_rewind.setTooltip("Go to Start");
        addAndMakeVisible(_button_rewind);
        _button_rewind.setEnabled(false);

        // right
        _button_open.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_open.set_icon(icons_ids::open, 16.f);
        _button_open.setTooltip("Add File");
        addAndMakeVisible(_button_open);

        _button_settings.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_settings.set_icon(icons_ids::settings);
        _button_settings.setTooltip("Audio Device Options");
        addAndMakeVisible(_button_settings);

        auto default_size = get_size();
        setSize(default_size.first, default_size.second);
    }

    ~comp_toolbar() { };

    void paint(Graphics& g) override {
        g.fillAll(_colors.get(color_ids::bg_light));
        /*auto a = _button_a.getBounds();
        auto a_x = a.getX() + (a.getWidth() / 2.f);
        auto a_y = a.getBottom() + 2.f;
        Line<float> line_a(a_x, a_y, a_x + 20.f, a_y + 20.f);
        g.drawLine(line_a);*/
    }

    void resized() override  {
        const auto spacing_small = 2;
        const auto button_size   = _button_a.get_size();
              auto x             = 4;
              auto y             = (getHeight() - button_size) / 2;
              auto resize_button = [&](button_toolbar& button, bool big_spacing = false)
              {
                  button.setBounds(x, y, button_size, button_size);
                  x += button_size + (big_spacing ? spacing_small * 10 : spacing_small);
              };

        resize_button(_button_rev);
        resize_button(_button_a);
        resize_button(_button_hz);
        resize_button(_button_b);
        resize_button(_button_fwd, true);

        resize_button(_button_restart);
        resize_button(_button_blind, true);

        resize_button(_button_pause);
        resize_button(_button_play);
        resize_button(_button_stop);
        resize_button(_button_rewind, true);

        auto bounds = getLocalBounds();
        bounds.reduce(4, 4);
        _button_settings.setBounds(bounds.removeFromRight(button_size));
        bounds.removeFromRight(spacing_small);
        _button_open.setBounds(bounds.removeFromRight(button_size));
    }

    std::pair<int, int> get_size() const {
        return std::make_pair(300, 40);
    }
    void set_on_rewind_clicked(const std::function<void()>& callback) {
        _button_rewind.onClick = callback;
    }
    void set_on_open_clicked(const std::function<void()>& callback) {
        _button_open.onClick = callback;
    }
    void set_on_play_clicked(const std::function<void()>& callback) {
        _button_play.onClick = callback;
    }
    void set_on_pause_clicked(const std::function<void()>& callback) {
        _button_pause.onClick = callback;
    }
    void set_on_stop_clicked(const std::function<void()>& callback) {
        _button_stop.onClick = callback;
    }
    void set_on_blind_clicked(const std::function<void()>& callback) {
        _button_blind.onClick = callback;
    }
    void set_on_choose_clicked(const std::function<void(uint8_t button)>& callback) {
        _choose_clicked = callback;
    }
    void set_on_rev_clicked(const std::function<void()>& callback) {
        _button_rev.onClick = callback;
    }
    void set_on_fwd_clicked(const std::function<void()>& callback) {
        _button_fwd.onClick = callback;
    }
    void set_on_settings_clicked(const std::function<void()>& callback) {
        _button_settings.onClick = callback;
    }

    button_toolbar* get_button(button_t button) {
        switch (button) {
        case button_t::rev:     return &_button_rev;
        case button_t::a:       return &_button_a;
        case button_t::hz:      return &_button_hz;
        case button_t::b:       return &_button_b;
        case button_t::fwd:     return &_button_fwd;
        case button_t::restart: return &_button_restart;
        case button_t::blind:   return &_button_blind;
        case button_t::pause:   return &_button_pause;
        case button_t::play:    return &_button_play;
        case button_t::stop:    return &_button_stop;
        case button_t::rewind:  return &_button_rewind;
        case button_t::open:    return &_button_open;
        }
        return nullptr;
    }

    void hard_press(button_t button, bool should_be_pressed) {
        if (auto p_button = get_button(button)) {
            p_button->hard_pressed = should_be_pressed;
        }
    }
    bool get_state(button_t button) {
        if (auto p_button = get_button(button)) {
            return p_button->getToggleState();
        }
        return false;
    }
    void set_state(button_t button, bool shoud_be_on) {
        if (auto p_button = get_button(button)) {
            p_button->setToggleState(shoud_be_on, dontSendNotification);
        }
    }
    void set_enabled(button_t button, bool should_be_enabled) {
        if (auto p_button = get_button(button)) {
            p_button->setEnabled(should_be_enabled);
        }
    }
    void click(button_t button) {
        if (auto p_button = get_button(button)) {
            p_button->triggerClick();
        }
    }

private:
    // todo: [8]
    button_toolbar _button_rev,
                   _button_a,
                   _button_hz,
                   _button_b,
                   _button_fwd,
                   _button_restart,
                   _button_blind,
                   _button_pause,
                   _button_play,
                   _button_stop,
                   _button_rewind,
                   _button_open,
                   _button_settings;

    std::function<void(uint8_t button)> _choose_clicked;
    bool                                _connected { false };
    colors                              _colors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_toolbar)
};

}