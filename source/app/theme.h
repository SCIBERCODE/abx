#pragma once

#pragma warning(disable : 26812)

namespace abx {

namespace margins {
    constexpr size_t _small      = 2;
    constexpr size_t _medium     = _small * 2;
    constexpr size_t _line       = _small * 10;
    constexpr size_t _icon_small = 14;
    constexpr size_t _width      = 845;
}

namespace colour_id {
    enum window_ids : size_t {
        header,
        bg_light,
        bg_dark,
        outline,
        outline_dark,
        text_disabled,
        __last_window_ids
    };
    enum slider_ids : size_t {
        handle_normal = __last_window_ids,
        handle_hover,
        handle_pressed,
        __last_slider_ids
    };
    enum button_ids : size_t {
        button_normal = __last_slider_ids,
        button_hover,
        button_pressed,
        button_green,
        button_red,
        __last_button_ids
    };
    enum track_ids : size_t {
        waveform = __last_button_ids,
        waveform_light,
        waveform_disabled,
        waveform_bg,
        header_focused,
        header_lines
    };
}

enum class font_id {
    result, file_info
};

enum class icon_id {
    // abx
    backward,
    forward,
    // abx settings
    restart,
    blind,
    // player
    pause,
    play,
    stop,
    rewind,
    // devices names
    edit,
    // toolbar right sided
    open,
    language,
    settings,
    // result
    undo,
    clear,
    share,
    // tracks
    close,
    // common
    ok,
    warning
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class resources
{
private:
    static inline std::map<icon_id, const char*> icons_svg {
        { icon_id::backward, BinaryData::backward_svg },
        { icon_id::forward,  BinaryData::forward_svg  },
        { icon_id::restart,  BinaryData::restart_svg  },
        { icon_id::blind,    BinaryData::blind_svg    },
        { icon_id::pause,    BinaryData::pause_svg    },
        { icon_id::play,     BinaryData::play_svg     },
        { icon_id::stop,     BinaryData::stop_svg     },
        { icon_id::rewind,   BinaryData::rewind_svg   },
        { icon_id::edit,     BinaryData::edit_svg     },
        { icon_id::open,     BinaryData::open_svg     },
        { icon_id::language, BinaryData::language_svg },
        { icon_id::settings, BinaryData::settings_svg },
        { icon_id::undo,     BinaryData::undo_svg     },
        { icon_id::clear,    BinaryData::clear_svg    },
        { icon_id::share,    BinaryData::share_svg    },
        { icon_id::close,    BinaryData::close_svg    },
        { icon_id::ok,       BinaryData::ok_svg       },
        { icon_id::warning,  BinaryData::warning_svg  }
    };
public:
    static std::unique_ptr<Drawable> get_drawable(icon_id id, float size = 16.f) {
        if (icons_svg.count(id) == 0) return nullptr;

        std::unique_ptr<XmlElement> icon_svg_xml(XmlDocument::parse(icons_svg.at(id)));
        auto drawable_svg = Drawable::createFromSVG(*(icon_svg_xml.get()));

        drawable_svg->setTransformToFit(
            juce::Rectangle<float>(0.f, 0.f, size, size),
            RectanglePlacement::centred
        );

        return drawable_svg;
    }

    static auto get_font(font_id font_id, float height = 13.f)
    {
        String font_name {};
        switch (font_id) {
        case font_id::result:    font_name = "upcdl"; break;
        case font_id::file_info: font_name = "sfpro"; break;
        }
        int binary_data_size;
        auto binary_data = BinaryData::getNamedResource((font_name + "_ttf").getCharPointer(), binary_data_size);
        return Font(Typeface::createSystemTypefaceFor(binary_data, binary_data_size)).withHeight(height);
    }

    static auto get_html_header()
    {
        return "\
            <!DOCTYPE html>\
            <html><head>\
            <style>\
                .delim  { border-top: dotted 1px black;         }\
                .nope   { background-color: rgb(251, 202, 204); }\
                .param  { background-color: rgb(225, 225, 225); }\
                .ok     { background-color: rgb(209, 236, 193); }\
                .static {\
                    min-width:     10rem;\
                    display:       inline-block;\
                    padding-right: 0.8rem;\
                    margin-bottom: 0.3rem;\
                    text-align:    right;\
                }\
            </style>\
            </head>\
            <body style = 'background-color: rgb(240, 240, 240);'>\
            <pre>\
            <span class='static' style='text-align: left;'>abx log</span>\
            <div class='delim'></div>\
            <span class='static' style='margin-bottom: 0.8rem'>score</span><span>button / relay</span>\
        ";
    }
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class colours
{
public:
    colours() {
        using namespace colour_id;
        _colours[header           ] = { 205, 217, 241 };
        _colours[bg_light         ] = { 247, 249, 255 };
        _colours[bg_dark          ] = {  98, 103, 133 };
        _colours[outline          ] = { 134, 138, 161 };
        _colours[outline_dark     ] = {  50,  62,  68 };
        _colours[text_disabled    ] = { 198, 200, 205 };
        // slider
        _colours[handle_normal    ] = {  77,  86, 236 };
        _colours[handle_hover     ] = _colours[handle_normal].brighter(.4f);
        _colours[handle_pressed   ] = _colours[handle_normal].brighter(.8f);
        // button
        _colours[button_normal    ] = Colours::transparentWhite;
        _colours[button_hover     ] = { 185, 194, 227 };
        _colours[button_pressed   ] = _colours[button_hover].darker(.07f);
        _colours[button_green     ] = {  97, 207, 165 };
        _colours[button_red       ] = { 217, 152, 104 };
        // track
        _colours[waveform         ] = {  50,  50, 200 };
        _colours[waveform_light   ] = { 100, 100, 220 };
        _colours[waveform_disabled] = { 136, 136, 144 };
        _colours[waveform_bg      ] = { 192, 192, 192 };
        _colours[header_focused   ] = { 213, 227, 255 };
        _colours[header_lines     ] = { 128, 128, 128 };
    }

    Colour get(size_t index) {
        return _colours.count(index) ? _colours.at(index) : Colours::darkgrey;
    }
private:
    std::map<size_t, Colour> _colours;
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class theme : public LookAndFeel_V4 {
private:
    abx::colours _colours;
    const std::map<int, Colour> _colours_sys =
    {
        { TextButton   ::textColourOnId,     Colours::black            },
        { TextButton   ::buttonColourId,     Colours::transparentWhite },

        { ComboBox     ::backgroundColourId, Colours::white            },
        { ListBox      ::backgroundColourId, Colours::white            },

        { TooltipWindow::backgroundColourId, Colours::lightyellow      },
        { TooltipWindow::outlineColourId,    Colours::black            },
        { TooltipWindow::textColourId,       Colours::black            },

        { PopupMenu    ::highlightedTextColourId,        Colours::black                   },
        { PopupMenu    ::backgroundColourId,            _colours.get(colour_id::bg_light) },
        { PopupMenu    ::highlightedBackgroundColourId, _colours.get(colour_id::header)   },

        { AlertWindow  ::backgroundColourId,            _colours.get(colour_id::bg_light) },
        { AlertWindow  ::outlineColourId,               Colours::black                    }
    };

public:
    theme() {
        auto scheme = getLightColourScheme();
        scheme.setUIColour(ColourScheme::outline,          _colours.get(colour_id::outline));
        scheme.setUIColour(ColourScheme::widgetBackground, _colours.get(colour_id::header)); // for the window title bar and buttons
        setColourScheme(scheme);

        for (const auto [id, colour] : _colours_sys)
            setColour(id, colour);
    }

    void drawDocumentWindowTitleBar(DocumentWindow& window, Graphics& g,
        int width, int height, int title_x, int title_width,
        const Image* icon, bool text_on_left) override
    {
        ignoreUnused(icon);
        ignoreUnused(width);
        ignoreUnused(title_width);
        ignoreUnused(text_on_left);

        g.setColour(getCurrentColourScheme().getUIColour(ColourScheme::widgetBackground));
        g.fillAll();

        Font font(height * .6f, Font::bold);
        g.setFont(font);

        auto text_width = font.getStringWidth(window.getName());
        g.setColour(window.findColour(DocumentWindow::textColourId));
        g.drawText(window.getName(), title_x, 0, text_width, height, Justification::centredLeft, true);
    }

    auto layout_tooltip(const String& text) noexcept
    {
        const auto split = "[";

        String info, keys;
        if (text.indexOf(0, split) == -1)
            info = text;
        else {
            info = text.upToFirstOccurrenceOf(split, false, false).trim();
            keys = text.fromFirstOccurrenceOf(split, true, false).toLowerCase();
        }

        AttributedString as;
        as.setJustification(Justification::centredLeft);

        if (info.length()) {
            info << "\r\n";
            as.append(info, Font(13.f, Font::bold), Colours::black);
        }
        as.append(keys, Font(13.f), Colours::darkgrey);

        TextLayout tl;
        tl.createLayout(as, 400.f);
        return tl;
    }

    juce::Rectangle<int> getTooltipBounds(const String& text, Point<int> pos, juce::Rectangle<int> parent) override
    {
        const TextLayout tl(layout_tooltip(text));

        auto w = static_cast<int>(tl.getWidth ()) + 14;
        auto h = static_cast<int>(tl.getHeight()) + 8;

        return juce::Rectangle<int>(
            pos.x > parent.getCentreX() ? pos.x - (w + 12) : pos.x + 24,
            pos.y > parent.getCentreY() ? pos.y - (h + 6) : pos.y + 6,
            w, h)
            .constrainedWithin(parent);
    }

    void drawTooltip(Graphics& g, const String& text, int w, int h) override
    {
        juce::Rectangle<int> bounds(w, h);

        g.setColour(findColour(TooltipWindow::backgroundColourId));
        g.fillRoundedRectangle(bounds.toFloat(), 0.f);

        g.setColour(findColour(TooltipWindow::outlineColourId));
        g.drawRoundedRectangle(bounds.toFloat().reduced(.5f, .5f), 0.f, 1.f);

        g.setOrigin(7, 0);
        layout_tooltip(text)
            .draw(g, { static_cast<float>(w), static_cast<float>(h) });
    }
   
    void drawPopupMenuItem(Graphics& g, const juce::Rectangle<int>& area,
        const bool separator, const bool active,
        const bool highlighted, const bool ticked,
        const bool has_submenu, const String& text,
        const String& text_key,
        const Drawable* icon, const Colour* const text_colour_to_use) override
    {
        ignoreUnused(ticked);
        ignoreUnused(icon);
        if (separator)
        {
            auto r = area.reduced(5, 0);
            r.removeFromTop(roundToInt((static_cast<float>(r.getHeight()) * .5f) - .5f));
            g.setColour(findColour(PopupMenu::textColourId).withAlpha(.3f));
            g.fillRect(r.removeFromTop(1));
        }
        else
        {
            auto text_colour = (text_colour_to_use == nullptr ? findColour(PopupMenu::textColourId) : *text_colour_to_use);

            auto r = area.reduced(1);

            if (highlighted && active)
            {
                g.setColour(findColour(PopupMenu::highlightedBackgroundColourId));
                g.fillRect(r);
                g.setColour(findColour(PopupMenu::highlightedTextColourId));
            }
            else
            {
                g.setColour(text_colour.withMultipliedAlpha(active ? 1.f : .5f));
            }

            r.reduce(jmin(5, area.getWidth() / 20), 0);

            auto font = getPopupMenuFont();

            auto max_font_height = static_cast<float>(r.getHeight()) / 1.3f;

            if (font.getHeight() > max_font_height)
                font.setHeight(max_font_height);

            g.setFont(Font("Meiryo UI", 14.f, 0));

            r.removeFromLeft(roundToInt(max_font_height)).toFloat();

            if (has_submenu)
            {
                auto arrowH = .6f * getPopupMenuFont().getAscent();

                auto x = static_cast<float>(r.removeFromRight(static_cast<int>(arrowH)).getX());
                auto halfH = static_cast<float>(r.getCentreY());

                Path path;
                path.startNewSubPath(x, halfH - arrowH * .5f);
                path.lineTo(x + arrowH * .6f, halfH);
                path.lineTo(x, halfH + arrowH * .5f);

                g.strokePath(path, PathStrokeType(2.f));
            }

            r.removeFromRight(3);
            g.drawFittedText(text, r, Justification::centredLeft, 1);

            /*AttributedString as;
            as.setJustification(Justification::centredLeft);
            as.append(text, Font(13.f));
            TextLayout tl;
            tl.createLayout(as, 400.f);
            tl.draw(g, area.toFloat());*/

            if (text_key.isNotEmpty())
            {
                g.setColour(Colours::darkgrey);
                g.drawText(text_key.toLowerCase(), r, Justification::centredRight, true);
            }
        }
    }

};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class header_button_lf : public theme
{
public:
    void drawButtonBackground(Graphics& g, Button& button, const Colour& bg_colour, bool hover, bool pressed) override
    {
        ignoreUnused(bg_colour);
        auto bounds = button.getLocalBounds().toFloat();
        bounds.removeFromBottom(.5f);

        if (pressed)
            g.setColour(_colours.get(colour_id::button_pressed));
        else if (hover)
            g.setColour(_colours.get(colour_id::button_hover));
        else
            g.setColour(_colours.get(colour_id::bg_light));

        g.fillRect(bounds);
        g.setColour(_colours.get(colour_id::outline_dark));

        if (button.isConnectedOnBottom())
            g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getWidth(), bounds.getBottom());

        g.setColour(_colours.get(colour_id::outline));
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getY());
    }

    void drawButtonText(Graphics& g, TextButton& button, bool highlighted, bool down)
    {
        ignoreUnused(highlighted);
        ignoreUnused(down);

        Font font(jmin(16.f, button.getHeight() * .6f));
        font.setBold(true);
        g.setFont(font);
        g.setColour(_colours.get(colour_id::outline));

        const int y           = jmin(4, button.proportionOfHeight(.3f));
        const int corner_size = jmin(button.getHeight(), button.getWidth()) / 2;
        const int font_height = roundToInt(font.getHeight() * .6f);
        const int x           = jmin(font_height, 2 + corner_size / (button.isConnectedOnLeft()  ? 4 : 2));
        const int right       = jmin(font_height, 2 + corner_size / (button.isConnectedOnRight() ? 4 : 2));
        const int text_width  = button.getWidth() - x - right;
        auto button_size      = button.getHeight() * .75f;
        auto button_indent    = (button.getHeight() - button_size) * .5f;
        bool close            = !button.getToggleState();

        if (button.getClickingTogglesState())
        {
            juce::Rectangle<float> area(font.getStringWidthFloat(button.getButtonText()) + 10, button_indent, button_size, button_size);
            Path path;
            path.addTriangle(1.f, 0.f, 0.f, close ? 0.f : .5f, close ? .5f : 1.f, 1.f);
            g.fillPath(path, path.getTransformToScaleToFit(area.reduced(3.f, 3.f), true));
        }

        g.setColour(Colours::black);
        if (text_width > 0)
            g.drawFittedText(button.getButtonText(), x, y, text_width, button.getHeight() - y * 2, Justification::centredLeft, 2);
    }

private:
    abx::colours _colours;

};

}