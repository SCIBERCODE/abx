#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../controls/button_icon.h"
#include "../controls/slider.h"
#include "../app/play.h"
#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_track : public Component,
                   public ChangeListener,
                   public Timer
{
public:
    comp_track(const String& file_path, AudioTransportSource& transport_source) :
        _thumbnail(512, _processor.getFormatManager(), _thumbnail_cache),
        _font(resources::get_font(font_ids::file_info)),
        _transport_source(transport_source),
        _file_path(file_path)
    {
        _label_name.setFont(_font);
        _label_name.setColour(Label::textColourId, Colours::black);
        _label_name.setMinimumHorizontalScale(1.f);
        addAndMakeVisible(_label_name);

        _label_rate.setFont(_font);
        _label_rate.setColour(Label::textColourId, Colours::grey);
        _label_rate.setMinimumHorizontalScale(1.f);
        addAndMakeVisible(_label_rate);

        _label_format.setFont(_font);
        _label_format.setColour(Label::textColourId, Colours::grey);
        _label_format.setMinimumHorizontalScale(1.f);
        addAndMakeVisible(_label_format);

        _icon.addAndMakeVisible(resources::get_drawable(icons_ids::warning, 14, 14));
        addAndMakeVisible(&_icon);
        _icon.setVisible(false);

        _button_close.set_icon(icons_ids::close);
        _button_close.setTooltip("Close Track");
        _button_close.onClick = [&]()
        {
            _callback_close(this);
        };
        addAndMakeVisible(_button_close);

        // sliders
        _slider_volume.set_label("Vol.");
        _slider_volume.set_on_slider_value_changed([this](double value)
        {
            _processor.set_gain(static_cast<float>(value));
        });
        addAndMakeVisible(_slider_volume);

        _marker_user.setFill(Colours::darkgrey);
        _marker_playing.setFill(Colours::black);

        addAndMakeVisible(_marker_playing);
        addAndMakeVisible(_marker_user);
        _marker_playing.setVisible(false);
        _marker_user.setVisible(false);

        _dull_color.setAlpha(.4f);
        _dull_color.setFill(_colors.get(color_ids::waveform_bg));
        addAndMakeVisible(_dull_color);
        _dull_color.setVisible(false);

        _thumbnail.addChangeListener(this);
        setSize(300, 96);

        _label_name.setText(File(_file_path).getFileNameWithoutExtension(), NotificationType::dontSendNotification);

        auto input_source = new FileInputSource(_file_path);
        _thumbnail.setSource(input_source);

        File file(_file_path);
        if (file.existsAsFile())
        {
            auto reader = _processor.getFormatManager().createReaderFor(file);
            if (reader) {
                _sample_rate = reader->sampleRate;
                _format_name = reader->getFormatName();
                _bps = reader->bitsPerSample;
            }
            delete reader;
        }
    }

    ~comp_track() {
        setLookAndFeel(nullptr);
        _thumbnail.removeChangeListener(this);
    };

    void update_info(double device_sample_rate) {
        if (_sample_rate != 0.)
        {
            auto rate = std::format("{:.0f} Hz", _sample_rate);
            _icon.setVisible(false);
            if (device_sample_rate != _sample_rate)
            {
                rate += std::format("  >>  {:.0f} Hz", device_sample_rate);
                _icon.setVisible(true);
            }
            _label_rate.setText(rate, sendNotificationAsync);
        }

        auto ext = File(_file_path).getFileExtension().substring(1).toUpperCase().toStdString();
        _label_format.setText(std::format("{}{} File",
            _bps ? (std::format("{}-bit ", _bps)) : "", ext), sendNotificationAsync);

        resized();
    }

    const String& get_file_path() const {
        return _file_path;
    }

    track_processor& get_processor() {
        return _processor;
    }

    void set_on_mouse_event(const std::function<void(comp_track*, bool double_click)>& callback) {
        _callback_mouse_event = callback;
    }

    void set_on_close(const std::function<void(comp_track*)>& callback) {
        _callback_close = callback;
    }

    void paint(Graphics& g) override
    {
        g.saveState();
        {
            auto bounds = getLocalBounds();
            bounds.reduce(4, _y);

            Path clip_region;
            clip_region.addRoundedRectangle(bounds, 0);
            g.reduceClipRegion(clip_region);

            // waveform background
            _rect_background = juce::Rectangle<int>(_rect_header.getRight(), _y, bounds.getWidth() - _rect_header.getWidth(), bounds.getHeight());
            g.setColour(_colors.get(color_ids::waveform_bg));
            g.fillRect(_rect_background);
            _rect_background.reduce(0, 4);

            // waveform
            if (_thumbnail.getNumChannels() == 0)
            {
                g.setColour(Colours::red.brighter());
                auto x = 0.f;
                while (x < bounds.getWidth() + bounds.getHeight())
                {
                    juce::Line<float> line(x, static_cast<float>(bounds.getHeight()), bounds.getHeight() + x, 0.f);
                    x += 5.f;
                    g.drawLine(line, 1.f);
                }
            }
            else {
                if (!_active) {
                    g.setColour(_colors.get(color_ids::waveform_disabled));
                    _thumbnail.drawChannels(g, _rect_background, 0, _thumbnail.getTotalLength(), 1.f);
                }
                else {
                    g.setColour(_colors.get(color_ids::waveform));
                    _thumbnail.drawChannels(g, _rect_background, 0, _thumbnail.getTotalLength(), 1.f);
                    g.setColour(_colors.get(color_ids::waveform_light));
                    _thumbnail.drawChannels(g, _rect_background, 0, _thumbnail.getTotalLength(), .3f);
                }
                draw_user_marker();
            }

            g.setColour(_focused ? _colors.get(color_ids::header_focused) : _colors.get(color_ids::bg_light));
            g.fillRect(_rect_header);

            juce::Rectangle<int> colorRect(_rect_header.getX(), _rect_header.getY(), _color_rect_w, _rect_header.getHeight());
            g.setColour(_active ? _colors.get(color_ids::waveform) : _colors.get(color_ids::waveform_disabled));
            g.fillRect(colorRect);
            g.setColour(Colours::black);
            g.drawLine(_rect_header.getRight() - .5f, 0.f, _rect_header.getRight() - .5f, static_cast<float>(_rect_header.getBottom()), 1.f);

            // lines in the header
            g.setColour(_colors.get(color_ids::header_lines));
            g.drawLine(static_cast<float>(_rect_header.getX() + _color_rect_w), _rect_header.getY() + _header_h + .5f, _rect_header.getRight() - 1.0f, _rect_header.getY() + _header_h + .5f); // horizontal
            auto line_1_x = _color_rect_w + _rect_header.getTopLeft().getX() + _button_close.getWidth() + .5f;
            g.drawLine(line_1_x, static_cast<float>(_rect_header.getY()), line_1_x, _rect_header.getY() + _header_h + .5f);

            g.setColour(Colours::black);
            g.drawRect(bounds);
            //g.drawLine(8, bounds.getHeight() + 4, bounds.getWidth() + 1, bounds.getHeight() + 4);
            if (_active) {
                g.setColour(_colors.get(color_ids::waveform));
                g.drawLine(4.f, static_cast<float>(_y), 4.f, static_cast<float>(bounds.getHeight() + _y), 2.f);
            }
        }
        g.restoreState();
    };

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.reduce(4, _y);

        _rect_header = juce::Rectangle<int>(bounds.getX(), bounds.getY(), 250, bounds.getHeight());

        // header
        const auto spacing = 1;

        _rect_header_top = juce::Rectangle<int>(_rect_header);
        _rect_header_top.setHeight(_header_h);
        _rect_header_top.removeFromLeft(_color_rect_w);
        _button_close.setBounds(_rect_header_top.removeFromLeft(_button_close.getWidth()).withTrimmedTop(1));

        _rect_header_top.removeFromRight(spacing);
        _rect_header_top.setHeight(13);
        _rect_header_top.setY(_rect_header_top.getY() + 4);
        _rect_header_top.removeFromLeft(4);
        _label_name.setBounds(_rect_header_top);

        // slider and info
        juce::Rectangle<int> header_bottom_rect(_rect_header);
        const auto padding = 4;
        header_bottom_rect.removeFromTop(24 + spacing + padding);
        header_bottom_rect.removeFromRight(padding + 1);
        header_bottom_rect.removeFromLeft(8 + spacing + padding);
        header_bottom_rect.removeFromBottom(padding);

        auto vol_rect = header_bottom_rect.removeFromTop(_slider_volume.getHeight());
        vol_rect.setHeight(max(vol_rect.getHeight(), _slider_volume.getHeight()));
        _slider_volume.setBounds(vol_rect);

        header_bottom_rect.removeFromBottom(4);
        _label_format.setBounds(header_bottom_rect.removeFromBottom(14));

        auto rate_line = header_bottom_rect.removeFromBottom(14);
        if (_icon.isVisible()) {
            rate_line.removeFromLeft(4);
            _icon.setBounds(rate_line.removeFromLeft(14));
        }
        _label_rate.setBounds(rate_line);
    };

    void changeListenerCallback(ChangeBroadcaster* source) override {
        if (source == &_thumbnail) {
            repaint();
        }
    };

    void timerCallback() override {
        draw_playing_marker();
    };

    void play(bool restart) {
        startTimerHz(40);
        if (!_paused) {
            if (restart) _transport_source.setPosition(_marker);
        }
        draw_playing_marker();
        _marker_playing.setVisible(true);
        _paused = false;
    }

    void pause() {
        _paused = true;
    }

    void stop(bool restart) {
        ignoreUnused(restart);
        stopTimer();
        _marker_playing.setVisible(false);
        /*if (restart)*/ _transport_source.setPosition(_marker);
        draw_playing_marker();
        _paused = false;
    }

    void rewind() {
        update_user_marker(0.0);
        _transport_source.setPosition(_marker);
        draw_playing_marker();
    }

    void focus(bool should_be_focused) {
        if (_focused != should_be_focused)
        {
            _focused = should_be_focused;
            //repaint_header();
            repaint();
        }
    }

    void activate(bool should_be_activated) {
        _active = should_be_activated;
    }

    bool is_active() {
        return _active;
    }

    double get_marker() {
        return _marker;
    }

    void repaint_header() { }

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
private:

    float time2x(const double time) const {
        return roundToInt(_rect_background.getWidth() * static_cast<float>(time / _thumbnail.getTotalLength())) + _rect_background.getX() - 1.f;
    }

    double x2time(const float x) const {
        return ((x - _rect_background.getX()) / _rect_background.getWidth()) * _thumbnail.getTotalLength();
    }

    void mouseDown(const MouseEvent& e) override // todo: [13]
    {
        if (!_active || !_rect_background.contains(e.x, e.y)) {
            _callback_mouse_event(this, false);
            return;
        }
        update_user_marker(x2time(static_cast<float>(e.x)));// +3.f); // todo: [14]
        if (!_paused) {
            _transport_source.setPosition(_marker);
            draw_playing_marker();
        }
    }

    void mouseDrag(const MouseEvent& e) override
    {
        if (!_active) return;
        mouseDown(e);
        if (e.x < _rect_background.getX()) {
            update_user_marker(0.0);
        }
        if (e.x > _rect_background.getRight()) {
            update_user_marker(_rect_background.getRight());
        }
        repaint();
    }

    void mouseDoubleClick(const MouseEvent& e) override
    {
        _callback_mouse_event(this, true);
        mouseMove(e);
    }

    void mouseMove(const MouseEvent& e) override
    {
        if (!_active) return;
        if (_rect_background.contains(e.x, e.y))
            setMouseCursor(MouseCursor::IBeamCursor);
        else
            setMouseCursor(MouseCursor::NormalCursor);
    }

    void mouseUp(const MouseEvent& ) override
    {
        //repaint();
    }

    void update_user_marker(double new_marker)
    {
        _marker = new_marker;
        draw_user_marker();
        _marker_user.setVisible(new_marker);
        _dull_color.setVisible(new_marker);
        _callback_mouse_event(this, false);
    }

    void draw_user_marker() // todo: [15]
    {
        if (_marker) {
            auto rect = juce::Rectangle<float>(time2x(_marker), 1.f, 1.f, static_cast<float>(getHeight() - 2));
            _marker_user.setRectangle(rect);

            auto rect2 = juce::Rectangle<float>(time2x(0), 1.f, time2x(_marker) - time2x(0), static_cast<float>(getHeight() - 2));
            rect2.reduce(1.f, 1.f);
            rect2.setRight(rect2.getRight()+1);
            _dull_color.setRectangle(rect2);
        }
    }

    void draw_playing_marker()
    {
        if (_marker_playing.isVisible()) {
            auto rect = juce::Rectangle<float>(time2x(_transport_source.getCurrentPosition()), 1.f, 1.f, static_cast<float>(getHeight() - 2));
            _marker_playing.setRectangle(rect);
        }
    }

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
private:
    track_processor        _processor;
    AudioTransportSource&  _transport_source;
    String                 _file_path,
                           _format_name;
    double                 _sample_rate {};
    size_t                 _bps         {};

    bool                   _focused     {},
                           _active      {},
                           _paused      {};
    double                 _marker      {};

    colors                 _colors;
    juce::Rectangle<int>   _rect_header,
                           _rect_header_top,
                           _rect_background;

    const int              _y            {  1 },
                           _color_rect_w {  8 },
                           _header_h     { 20 };

    Label                  _label_name,
                           _label_rate,
                           _label_format;

    button_icon            _button_close;
    abx::slider_with_label _slider_volume;
    AudioThumbnailCache    _thumbnail_cache { 1 };
    AudioThumbnail         _thumbnail;

    std::function<void(comp_track*, bool)> _callback_mouse_event;
    std::function<void(comp_track*)>       _callback_close;

    DrawableRectangle      _marker_playing,
                           _marker_user,
                           _dull_color;
    DrawableComposite      _icon;
    Font                   _font;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_track)
};

}