#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_toolbar_results : public Component {
public:
    comp_toolbar_results(const std::function<void()>& export_callback) :
        _font(resources::get_font(font_ids::result, 65))
    {
        _label_result.setFont(_font.boldened().withHorizontalScale(1.15f));
        _label_result.setColour(Label::textColourId, Colours::white);
        _label_result.setJustificationType(Justification::centred);
        addAndMakeVisible(_label_result);

        _button_share.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_clear.set_border_radius_side(button_toolbar::border_radius_side_t::right);
        _button_share.set_icon(icons_ids::share);
        _button_clear.set_icon(icons_ids::clear);
        _button_share.set_type(button_toolbar::button_t::utility);
        _button_clear.set_type(button_toolbar::button_t::utility);
        addAndMakeVisible(_button_share);
        addAndMakeVisible(_button_clear);

        _share_menu.addItem("Copy to Clipboard", [=]() {
            SystemClipboard::copyTextToClipboard(_label_result.getText());
        });
        _share_menu.addItem("Save as HTML", export_callback);

        _button_share.onClick = [&] {
            _share_menu.showAt(&_button_share);
        };

        set_result();
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
        auto button_size = _button_clear.get_size();
        _button_clear.setBounds(area.removeFromRight(button_size).withSizeKeepingCentre(button_size, button_size));
        area.removeFromRight(2);
        _button_share.setBounds(area.removeFromRight(button_size).withSizeKeepingCentre(button_size, button_size));
    };

    void set_result(String text = {}) {
        _button_clear.setEnabled(text.length());
        _button_share.setEnabled(text.length());
        if (text.length() == 0) {
            text = "0 / 0, -.--";
        }
        _label_result.setText(text, sendNotificationAsync);
    }
private:
    Label          _label_result;
    button_toolbar _button_clear, // todo: cancel last trial
                   _button_share;
    PopupMenu      _share_menu;
    colors         _colors;
    Font           _font;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_toolbar_results)
};

}