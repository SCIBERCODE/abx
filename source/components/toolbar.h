#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../controls/button_toolbar.h"
#include "../controls/slider.h"
#include "../controls/editor.h"
#include "../app/theme.h"

#include "../app/play.h"

namespace abx {

enum class button_t {
    rev, a, hz, b, fwd,
    restart, blind,
    pause, play, stop, rewind,
    open
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_toolbar : public Component,
                     public ChangeListener
{
private:
    using p_edit = std::unique_ptr<editor>;

public:
    comp_toolbar(ApplicationCommandManager& commands_) :
        _commands(commands_)
    { // todo: [9]
        setOpaque(true);

        // user input
        _button_rev.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_rev.set_icon(icon_id::backward);
        _button_rev.set_type(button_toolbar::button_type_t::ftdi);
        _button_rev.setCommandToTrigger(&_commands, commands::rev, true);
        addAndMakeVisible(_button_rev);

        _button_a .set_type(button_toolbar::button_type_t::ftdi);
        _button_b .set_type(button_toolbar::button_type_t::ftdi);
        _button_hz.set_type(button_toolbar::button_type_t::ftdi);

        _button_a .setCommandToTrigger(&_commands, commands::a,  true);
        _button_b .setCommandToTrigger(&_commands, commands::b,  true);
        _button_hz.setCommandToTrigger(&_commands, commands::hz, true);

        addAndMakeVisible(_button_a);
        addAndMakeVisible(_button_b);
        addAndMakeVisible(_button_hz);

        _button_fwd.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_fwd.set_icon(icon_id::forward);
        _button_fwd.set_type(button_toolbar::button_type_t::ftdi);
        _button_fwd.setCommandToTrigger(&_commands, commands::fwd, true);
        addAndMakeVisible(_button_fwd);

        // settings
        _button_restart.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_restart.set_icon(icon_id::restart, 16.f);
        _button_restart.setClickingTogglesState(true);
        _button_restart.setToggleState(true, dontSendNotification);
        addAndMakeVisible(_button_restart);

        _button_blind.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_blind.set_icon(icon_id::blind, 18.f);
        _button_blind.setClickingTogglesState(true);
        _button_blind.setToggleState(true, dontSendNotification);
        addAndMakeVisible(_button_blind);

        // player
        _button_pause.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_pause.set_icon(icon_id::pause);
        _button_pause.setClickingTogglesState(true);
        _button_pause.setEnabled(false);
        addAndMakeVisible(_button_pause);

        _button_play.set_icon(icon_id::play);
        _button_play.setClickingTogglesState(true);
        _button_play.setEnabled(false);
        addAndMakeVisible(_button_play);

        _button_stop.set_icon(icon_id::stop, 12.f);
        _button_stop.setCommandToTrigger(&_commands, commands::stop, true);
        _button_stop.setEnabled(false);
        addAndMakeVisible(_button_stop);

        _button_rewind.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_rewind.set_icon(icon_id::rewind);
        _button_rewind.setCommandToTrigger(&_commands, commands::rewind, true);
        _button_rewind.setEnabled(false);
        addAndMakeVisible(_button_rewind);

        // right
        _button_open.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_open.set_icon(icon_id::open, 16.f);
        _button_open.setCommandToTrigger(&_commands, commands::add_files, true);
        addAndMakeVisible(_button_open);

        _button_settings.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_settings.set_icon(icon_id::settings);
        _button_settings.setCommandToTrigger(&_commands, commands::options, true);
        addAndMakeVisible(_button_settings);

        // names
        auto init_edit = [&](p_edit& edit)
        {
            edit = std::make_unique<editor>("click to enter device name");
            edit->addChangeListener(this);
            edit->onTextChange = [this]()
            {
                _callback_name_changed();
            };
            addAndMakeVisible(*edit);
        };

        init_edit(_edits.first);
        init_edit(_edits.second);

        setSize(margins::_width, get_height());

        _edit_areas.first  = draw_reference_line(nullptr, _button_a, 31.f);
        _edit_areas.second = draw_reference_line(nullptr, _button_b, 10.f);

        // gain
        _sliders.first  = std::make_unique<slider_with_label>(-40);
        _sliders.second = std::make_unique<slider_with_label>(-40);

        _sliders.first->set_on_slider_value_changed([this](double value) {
            _gain.first = value;
            _gain.second = _sliders.second->get_value();
            _callback_gain_changed();
        });
        _sliders.second->set_on_slider_value_changed([this](double value) {
            _gain.first = _sliders.first->get_value();
            _gain.second = value;
            _callback_gain_changed();
        });

        addAndMakeVisible(_sliders.first.get());
        addAndMakeVisible(_sliders.second.get());

        resized();
    }

    ~comp_toolbar() {
        _edits.first ->removeChangeListener(this);
        _edits.second->removeChangeListener(this);
    }

    void changeListenerCallback(ChangeBroadcaster* source) override {
        ignoreUnused(source);
        _button_a.stress(_edits.first ->is_active());
        _button_b.stress(_edits.second->is_active());
        repaint();
    };

    void set_on_name_changed(const std::function<void()>& callback) {
        _callback_name_changed = callback;
    }

    void names_set(std::pair<String, String> names) const {
        _edits.first ->text_set(names.first);
        _edits.second->text_set(names.second);
    };

    auto names_get() const {
        return std::make_pair(
            _edits.first ->text_get(),
            _edits.second->text_get()
        );
    };

    juce::Rectangle<int> draw_reference_line(Graphics* g, const button_toolbar& button, float y_boottom, bool active = false)
    {
        auto area   = button.getBounds();
        auto edit_x = _button_restart.getX();
        auto x      = area.getX() + (area.getWidth() / 2.f);
        auto y      = button.getBottom() + 2.5f;

        Line<float> line(x, y, x + y_boottom, y + y_boottom);
        if (g) {
            g->setColour(active ? Colours::black : _colours.get(colour_id::outline));
            g->drawLine(line);
            g->fillRect(line.getEndX(), line.getEndY() - .5f, edit_x - line.getEndX() - 2.f, 1.f);
        }
        return juce::Rectangle<int>(
            edit_x, static_cast<int>(line.getEndY()) - 8, _button_rewind.getRight() - edit_x, 17);
    };

    void paint(Graphics& g) override {
        g.fillAll(_colours.get(colour_id::bg_light));
        draw_reference_line(&g, _button_a, 31.f, _edits.first ->is_active());
        draw_reference_line(&g, _button_b, 10.f, _edits.second->is_active());
    }

    void resized() override  {
        const auto button_size = _button_a.get_size();

        int x = margins::_medium;
        auto resize_button = [&](button_toolbar& button, bool big_spacing = false)
        {
            button.setBounds(x, margins::_medium, button_size, button_size);
            x += button_size + (big_spacing ? margins::_small * 10 : margins::_small);
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
        bounds.reduce(margins::_medium, margins::_medium);
        bounds.setHeight(button_size);
        _button_settings.setBounds(bounds.removeFromRight(button_size));
        bounds.removeFromRight(margins::_small);
        _button_open.setBounds(bounds.removeFromRight(button_size));

        _edits.first->setBounds(_edit_areas.first);
        _edits.second->setBounds(_edit_areas.second);

        auto slider_rect = [&](p_edit& parent)
        {
            int x = parent->getRight() + margins::_small;
            int w = getLocalBounds().getRight() - x - margins::_small;
            return juce::Rectangle<int>(x, parent->getY(), w, parent->getHeight());
        };

        if (_edits.first->getRight()) {
            _sliders.first->setBounds(slider_rect(_edits.first));
            _sliders.second->setBounds(slider_rect(_edits.second));
        }
    }

    int get_height() const {
        return 82;
    }
    auto get_settings_pos() const {
        return localPointToGlobal(Point<int>(
            _edit_areas.first.getX(),
            _edit_areas.first.getBottom() + margins::_medium
        ));
    }
    void set_on_play_clicked(const std::function<void()>& callback) {
        _button_play.onClick = callback;
    }
    void set_on_pause_clicked(const std::function<void()>& callback) {
        _button_pause.onClick = callback;
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

    void hard_press(button_t button, bool should_be_pressed, bool wrong = false) {
        if (auto p_button = get_button(button)) {
            p_button->hard_press(should_be_pressed, wrong);
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

    void set_on_gain_changed(const std::function<void()>& callback) {
        _callback_gain_changed = callback;
    }

    auto gain_get() {
        return _gain;
    }

    void gain_set(std::pair<double, double> volumes) const {
        _sliders.first ->set_value(volumes.first);
        _sliders.second->set_value(volumes.second);
    };

    track_processor& get_processor() {
        return _processor;
    }

private:
    track_processor            _processor;
    ApplicationCommandManager& _commands;

    // todo: [8]
    button_toolbar  _button_rev,
                    _button_a  { "A" },
                    _button_hz { "?" },
                    _button_b  { "B" },
                    _button_fwd,
                    _button_restart,
                    _button_blind,
                    _button_pause,
                    _button_play,
                    _button_stop,
                    _button_rewind,
                    _button_open,
                    _button_settings;
    std::pair<juce::Rectangle<int>,
              juce::Rectangle<int>>
                    _edit_areas;
    std::pair<p_edit, p_edit>
                    _edits;
    std::pair<double, double>
                    _gain;
    std::pair<std::unique_ptr<slider_with_label>,
              std::unique_ptr<slider_with_label>>
                    _sliders;

    std::function<void()> _callback_name_changed;
    std::function<void()> _callback_gain_changed;
    colours               _colours;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_toolbar)
};

}