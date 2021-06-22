#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "controls/slider.h"
#include "controls/button_icon.h"
#include "track_processor.h"
#include "theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class track_master : public Component {
public:
    track_master() {

        setOpaque(true);

        for (size_t k = 0; k < 2; k++) {

            _edit_name[k] = std::make_unique<TextEditor>();
            _slider_vol[k] = std::make_unique<slider_with_label>(-40);
            _slider_vol[k]->set_label("Vol.");

            _edit_name[k]->setTextToShowWhenEmpty("(click to enter device name)", Colours::grey);
            _edit_name[k]->setIndents(0, 2);
            _edit_name[k]->setColour(TextEditor::ColourIds::backgroundColourId, Colours::transparentWhite);

            _edit_name[k]->onFocusLost = [=]() {
                auto text = _edit_name[k]->getText();
                if (text.length())
                    _edit_name[k]->setText("(" + text.removeCharacters("()") + ")");
            };
            _edit_name[k]->onTextChange = [this]() {
                _callback_name_changed();
            };

            addAndMakeVisible(_label_name[k]);
            addAndMakeVisible(*_edit_name[k]);
            addAndMakeVisible(*_slider_vol[k]);
        }

        _slider_vol[0]->set_on_slider_value_changed([this](double value) {
            _gain[0] = value;
            _gain[1] = _slider_vol[1]->get_value();
            _callback_gain_changed();
        });
        _slider_vol[1]->set_on_slider_value_changed([this](double value) {
            _gain[0] = _slider_vol[0]->get_value();
            _gain[1] = value;
            _callback_gain_changed();
        });

        auto size = get_size();
        setSize(size.first, size.second);
    };

    ~track_master() { };

    void paint(Graphics& g) override {
        g.fillAll(_colors.get(color_ids::bg_light));
        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawLine(0.f, .5f, static_cast<float>(getBounds().getWidth()), .5f, 1.f);
    };

    void resized() override {
        auto bounds = getLocalBounds();

        bounds.removeFromTop(7);
        for (size_t k = 0; k < 2; k++)
        {
            auto line = bounds.removeFromTop(20);
            line.removeFromLeft(10);
            _label_name[k].setBounds(line.removeFromLeft(20));
            _edit_name[k]->setBounds(line.removeFromLeft(215));
            line.removeFromLeft(10);
            _slider_vol[k]->setBounds(line);

            bounds.removeFromTop(2);
        }
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

    double get_gain(size_t index) {
        return _gain[index];
    }

    std::pair<String, String> get_names() const {
        return std::make_pair(
            _edit_name[0]->getText(),
            _edit_name[1]->getText()
        );
    };

    void set_names(std::pair<String, String> names) const {
        _edit_name[0]->setText(names.first);
        _edit_name[1]->setText(names.second);
    };

    void set_volumes(std::pair<double, double> volumes) const {
        _slider_vol[0]->set_value(volumes.first);
        _slider_vol[1]->set_value(volumes.second);
    };

private:
    track_processor _processor;
    double          _gain[2];

    Label                              _label_name[2] { { { }, "A" }, { { }, "B" } };
    std::unique_ptr<slider_with_label> _slider_vol[2];
    std::unique_ptr<TextEditor>        _edit_name[2];

    std::function<void()> _callback_gain_changed;
    std::function<void()> _callback_name_changed;

    colors _colors;

    //button_icon button_edit_name_a; // todo
    //button_icon button_edit_name_b;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(track_master)
};

}
