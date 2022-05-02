#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class slider : public Slider {
public:
    slider(double minimum) : Slider(Slider::SliderStyle::LinearBar, Slider::NoTextBox) {
        setSize(100, 18);
        setRange(minimum, 10., 0.);
        setValue(0);
        setDoubleClickReturnValue(true, 0);
        //setSkewFactorFromMidPoint(-20); // todo: [16]
    }

    ~slider() { }

    void resized() override {
        Slider::resized();
    }

    void paint(Graphics& g) override {
        auto bounds       = getLocalBounds();
        auto range        = getRange();
        auto range_length = range.getLength();
        auto ratio        = range_length > 0. ? (getValue() - range.getStart()) / range_length : 0.;

        const auto handle_diameter = 12;
        juce::Rectangle<int> track_rect(handle_diameter / 2, (bounds.getHeight() - 4) / 2, bounds.getWidth() - handle_diameter, 4);

        g.saveState();
        {
            Path clip_region;
            clip_region.addRoundedRectangle(track_rect, 2);
            g.reduceClipRegion(clip_region);
            g.setColour(_colors.get(color_ids::waveform_bg));
            g.fillRect(track_rect);

            juce::Rectangle<int> groove_rect(track_rect);
            groove_rect.setWidth(static_cast<int>(groove_rect.getWidth() * ratio));
            g.setColour(_colors.get(color_ids::outline));
            g.fillRect(groove_rect);
        }
        g.restoreState();

        Path handle_path;
        handle_path.addEllipse(
            track_rect.getX() + static_cast<float>(ratio) * track_rect.getWidth() - handle_diameter / 2.f,
            (bounds.getHeight() - handle_diameter) / 2.f,
            static_cast<float>(handle_diameter),
            static_cast<float>(handle_diameter)
        );
        Colour handle_colour;

        if (isMouseButtonDown())
            handle_colour = _colors.get(color_ids::handle_pressed);
        else if (isMouseOver())
            handle_colour = _colors.get(color_ids::handle_hover);
        else
            handle_colour = _colors.get(color_ids::handle_normal);

        g.setColour(handle_colour);
        g.fillPath(handle_path);
    }

private:
    colors _colors;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(slider)
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class slider_with_label : public Component,
                          public Slider::Listener
{
public:
    slider_with_label(double minimum = -90) :
        _font(resources::get_font(font_ids::file_info)),
        _slider(minimum)
    {
        _label.setText("Vol.", NotificationType::dontSendNotification);
        _label.setFont(_font);
        _label.setColour(Label::textColourId, Colours::black);
        _label.setMinimumHorizontalScale(1.f);
        addAndMakeVisible(_label);

        _label_value.setJustificationType(Justification(Justification::centred));
        _label_value.setFont(_font);
        _label_value.setColour(Label::textColourId, Colours::black);
        _label_value.setMinimumHorizontalScale(1.f);
        addAndMakeVisible(_label_value);

        _slider.addListener(this);
        _slider.setColour(Slider::backgroundColourId, Colours::red);
        addAndMakeVisible(_slider);

        setSize(300, 18);
        update_label_value();
    }

    ~slider_with_label() {
        _slider.removeListener(this);
        setLookAndFeel(nullptr);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto old_height = bounds.getHeight();

        //bounds.setHeight(std::max(bounds.getHeight(), _slider.getHeight()));
        bounds.setHeight(bounds.getHeight() < _slider.getHeight() ? _slider.getHeight() : bounds.getHeight());
        bounds.setY((bounds.getHeight() - old_height) / 2);

        _label.setBounds(bounds.removeFromLeft(34));
        _label_value.setBounds(bounds.removeFromRight(60));
        _slider.setBounds(bounds);
    }

    void set_label(const String& text) {
        _label.setText(text, NotificationType::dontSendNotification);
    }

    void set_on_slider_value_changed(const std::function<void(double)>& callback) {
        _callback = callback;
    }

    double get_value() {
        return _slider.getValue();
    }

    void set_value(double value) {
        _slider.setValue(value);
    }

    void sliderValueChanged(Slider* slider) override {
        if (&_slider == slider) {
            if (_callback) {
                _callback(_slider.getValue());
            }
            update_label_value();
        }
    }

private:
    abx::slider                 _slider;
    Label                       _label,
                                _label_value;
    Font                        _font;
    std::function<void(double)> _callback;

    void update_label_value() {
        _label_value.setText(String(_slider.getValue(), 1) + " dB", NotificationType::dontSendNotification);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(slider_with_label)
};

}