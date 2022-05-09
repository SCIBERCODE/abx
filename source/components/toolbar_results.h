#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_toolbar_results : public Component {
public:
    comp_toolbar_results() :
        _font(resources::get_font(font_ids::result, 65))
    {
        _label_result.setFont(_font.boldened().withHorizontalScale(1.15f));
        _label_result.setColour(Label::textColourId, Colours::white);
        _label_result.setText("0 / 0, -.--", dontSendNotification);
        _label_result.setJustificationType(Justification::centred);
        addAndMakeVisible(_label_result);

        _button_save .set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_clear.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_clear.set_icon(icons_ids::clear);
        _button_clear.set_type(button_toolbar::button_t::utility);
        addAndMakeVisible(_button_clear);
        //button_clear.setEnabled(false);
    }
    ~comp_toolbar_results() {};

    void set_on_clear(const std::function<void()>& callback) {
        _button_clear.onClick = callback;
    }

    void paint(Graphics& g) override {
        g.fillAll(_colors.get(color_ids::bg_dark).brighter(.3f));
        g.setColour(_colors.get(color_ids::outline_dark));
        auto bounds = getLocalBounds();
        g.drawLine(
            static_cast<float>(bounds.getX()),
            static_cast<float>(bounds.getBottom()),
            static_cast<float>(bounds.getWidth()),
            static_cast<float>(bounds.getBottom()));
    };

    void resized() override {
        auto area = getLocalBounds();
        _label_result.setBounds(area.withTrimmedBottom(2));
        area.reduce(4, 4);
        auto button_size = _button_clear.get_size().first;
        _button_clear.setBounds(area.removeFromRight(button_size).withSizeKeepingCentre(button_size, button_size));
    };

    void set_result(String text) {
        _label_result.setText(text, sendNotificationAsync);
    }
private:
    Label          _label_result;
    button_toolbar _button_clear,
                   _button_save;
    colors         _colors;
    Font           _font;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_toolbar_results)
};

}