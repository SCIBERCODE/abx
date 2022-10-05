#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_toolbar_results : public Component {
public:
    comp_toolbar_results(ApplicationCommandManager& commands) :
        _font(resources::get_font(font_id::result, 65)),
        _commands(commands)
    {
        _label_result.setFont(_font.boldened().withHorizontalScale(1.15f));
        _label_result.setColour(Label::textColourId, Colours::white);
        _label_result.setJustificationType(Justification::centred);
        addAndMakeVisible(_label_result);

        _button_undo .set_border_radius_side(button_toolbar::border_radius_side_t::all);
        _button_share.set_border_radius_side(button_toolbar::border_radius_side_t::left);
        _button_clear.set_border_radius_side(button_toolbar::border_radius_side_t::right);

        _button_undo .set_icon(icon_id::undo);
        _button_share.set_icon(icon_id::share);
        _button_clear.set_icon(icon_id::clear);

        _button_undo .set_type(button_toolbar::button_type_t::utility);
        _button_share.set_type(button_toolbar::button_type_t::utility);
        _button_clear.set_type(button_toolbar::button_type_t::utility);

        _button_undo .setCommandToTrigger(&_commands, commands::undo,  true);
        _button_clear.setCommandToTrigger(&_commands, commands::clear, true);

        addAndMakeVisible(_button_undo);
        addAndMakeVisible(_button_share);
        addAndMakeVisible(_button_clear);

        _share_menu.setLookAndFeel(&_theme);
        _share_menu.addCommandItem(&_commands, commands::share_clipboard);
        _share_menu.addCommandItem(&_commands, commands::share_html);
        _button_share.onClick = [&] {
            _share_menu.showAt(&_button_share);
        };

        result_set();
    }

    ~comp_toolbar_results() {
        _share_menu.setLookAndFeel(nullptr);
    }

    void paint(Graphics& g) override {
        g.fillAll  (_colours.get(colour_id::bg_dark).brighter(.3f));
        g.setColour(_colours.get(colour_id::outline_dark));
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
        _button_undo.setBounds(area.removeFromLeft(button_size).withSizeKeepingCentre(button_size, button_size));
        _button_clear.setBounds(area.removeFromRight(button_size).withSizeKeepingCentre(button_size, button_size));
        area.removeFromRight(2);
        _button_share.setBounds(area.removeFromRight(button_size).withSizeKeepingCentre(button_size, button_size));
    };

    void result_set(String text = {}) {
        auto enable = text.length();

        _button_undo .enable(enable);
        _button_clear.enable(enable);
        _button_share.enable(enable);

        if (!enable) {
            text = "0 / 0, -.--";
        }
        _label_result.setText(text, sendNotificationAsync);
    }

    auto result_get() {
        return _label_result.getText();
    }

private:
    ApplicationCommandManager& _commands;
    Label                      _label_result;
    button_toolbar             _button_undo,
                               _button_clear,
                               _button_share;
    PopupMenu                  _share_menu;
    colours                    _colours;
    Font                       _font;
    theme                      _theme;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_toolbar_results)
};

}