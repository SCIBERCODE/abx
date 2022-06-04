#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../controls/slider.h"
#include "../controls/button_icon.h"
#include "../app/play.h"
#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_track_master : public Component {
public:
    comp_track_master() {

        setOpaque(true);
        auto init_controls = [&](std::unique_ptr<TextEditor>& edit, std::unique_ptr<slider_with_label>& slider)
        {
            edit   = std::make_unique<TextEditor>();
            slider = std::make_unique<slider_with_label>(-40);
            slider->set_label("Vol.");

            edit->setTextToShowWhenEmpty("(click to enter device name)", Colours::grey);
            edit->setIndents(0, 2);
            edit->setColour(TextEditor::ColourIds::backgroundColourId, Colours::transparentWhite);

            edit->onFocusLost = [&]() {
                auto text = edit->getText();
                if (text.length())
                    edit->setText("(" + text.removeCharacters("()") + ")");
            };
            edit->onTextChange = [this]() {
                _callback_name_changed();
            };
            addAndMakeVisible(*edit);
            addAndMakeVisible(*slider);
        };
        init_controls(_edits.first,  _sliders.first);
        init_controls(_edits.second, _sliders.second);

        addAndMakeVisible(_labels.first);
        addAndMakeVisible(_labels.second);

        _labels.first.setText ("A", dontSendNotification);
        _labels.second.setText("B", dontSendNotification);

        _sliders.first->set_on_slider_value_changed([this](double value) {
            _gain.first  = value;
            _gain.second = _sliders.first->get_value();
            _callback_gain_changed();
        });
        _sliders.second->set_on_slider_value_changed([this](double value) {
            _gain.first  = _sliders.second->get_value();
            _gain.second = value;
            _callback_gain_changed();
        });

        auto size = get_size();
        setSize(size.first, size.second);
    };

    ~comp_track_master() { };

    void paint(Graphics& g) override {
        g.fillAll(_colors.get(color_ids::bg_light));
        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawLine(0.f, .5f, static_cast<float>(getBounds().getWidth()), .5f, 1.f);
    };

    void resized() override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(7);

        auto set_controls = [&](Component* label, Component* edit, Component* slider) {
            auto line = bounds.removeFromTop(20);
            line.removeFromLeft(10);
            label->setBounds(line.removeFromLeft(20));
            edit->setBounds(line.removeFromLeft(215));
            line.removeFromLeft(10);
            slider->setBounds(line);
            bounds.removeFromTop(2);
        };
        set_controls(&_labels.first,  _edits.first.get(),  _sliders.first.get());
        set_controls(&_labels.second, _edits.second.get(), _sliders.second.get());
    };

    std::pair<int, int> get_size() const {
        return std::make_pair(300, 56);
    };

    track_processor& get_processor() {
        return _processor;
    };

    void set_on_gain_changed(const std::function<void()>& callback) {
        _callback_gain_changed = callback;
    }

    void set_on_name_changed(const std::function<void()>& callback) {
        _callback_name_changed = callback;
    }

    std::pair<String, String> names_get() const {
        return std::make_pair(
            _edits.first ->getText(),
            _edits.second->getText()
        );
    };

    void names_set(std::pair<String, String> names) const {
        _edits.first ->setText(names.first);
        _edits.second->setText(names.second);
    };

    auto gain_get() {
        return _gain;
    }

    void gain_set(std::pair<double, double> volumes) const {
        _sliders.first ->set_value(volumes.first);
        _sliders.second->set_value(volumes.second);
    };

private:
    track_processor           _processor;
    std::pair<double, double> _gain;
    std::pair<Label, Label>   _labels;
    std::pair<std::unique_ptr<slider_with_label>,
              std::unique_ptr<slider_with_label>>
                              _sliders;
    std::pair<std::unique_ptr<TextEditor>,
              std::unique_ptr<TextEditor>>
                              _edits;

    std::function<void()>     _callback_gain_changed,
                              _callback_name_changed;

    colors                    _colors;

    //button_icon button_edit_name_a; // todo
    //button_icon button_edit_name_b;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_track_master)
};

}
