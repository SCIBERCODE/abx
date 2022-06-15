#pragma once

namespace abx {

namespace color_ids {
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

enum class font_ids {
    result, file_info
};

enum class icons_ids {
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
    // toolbar right sided
    open,
    settings,
    // result
    clear,
    share,
    // tracks
    close,
    warning,
    // bottom toolbar
    edit
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class resources
{
private:
    static inline std::map<icons_ids, const char*> icons_svg {
        { icons_ids::backward, BinaryData::backward_svg },
        { icons_ids::forward,  BinaryData::forward_svg  },
        { icons_ids::restart,  BinaryData::restart_svg  },
        { icons_ids::blind,    BinaryData::blind_svg    },
        { icons_ids::pause,    BinaryData::pause_svg    },
        { icons_ids::play,     BinaryData::play_svg     },
        { icons_ids::stop,     BinaryData::stop_svg     },
        { icons_ids::rewind,   BinaryData::rewind_svg   },
        { icons_ids::open,     BinaryData::open_svg     },
        { icons_ids::settings, BinaryData::settings_svg },
        { icons_ids::clear,    BinaryData::clear_svg    },
        { icons_ids::share,    BinaryData::share_svg    },
        { icons_ids::close,    BinaryData::close_svg    },
        { icons_ids::warning,  BinaryData::warning_svg  },
        { icons_ids::edit,     BinaryData::edit_svg     }
    };
public:
     resources() { }
    ~resources() { }

    static Drawable* get_drawable(icons_ids icon_id, float width = 16.f, float height = 16.f) {
        if (icons_svg.count(icon_id) == 0) return nullptr;

        std::unique_ptr<XmlElement> icon_svg_xml(XmlDocument::parse(icons_svg.at(icon_id)));
        auto drawable_svg = Drawable::createFromSVG(*(icon_svg_xml.get()));

        drawable_svg->setTransformToFit(
            juce::Rectangle<float>(0.f, 0.f, width, height),
            RectanglePlacement::centred
        );
        return drawable_svg.release();
    }

    static auto get_font(font_ids font_id, float height = 13.f)
    {
        String font_name {};
        switch (font_id) {
        case font_ids::result:    font_name = "upcdl"; break;
        case font_ids::file_info: font_name = "sfpro"; break;
        }
        int binary_data_size;
        auto binary_data = BinaryData::getNamedResource((font_name + "_ttf").getCharPointer(), binary_data_size);
        return Font(Typeface::createSystemTypefaceFor(binary_data, binary_data_size)).withHeight(height);
    }

    static auto get_html_header()
    {
        return BinaryData::trial_log_header_html;
    }
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class colors
{
public:
    colors() {
        using namespace color_ids;
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
    ~colors() { }

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
public:
    theme() {
        auto scheme = getLightColourScheme();
        scheme.setUIColour(ColourScheme::outline, _colors.get(color_ids::outline));
        scheme.setUIColour(ColourScheme::widgetBackground, _colors.get(color_ids::header)); // for the window title bar and buttons
        setColourScheme(scheme);

        // restoring white background after init with ColourScheme::widgetBackground
        setColour(TextEditor::backgroundColourId, Colours::white);
        setColour(TextButton::buttonColourId,     Colours::white);
        setColour(ComboBox::backgroundColourId,   Colours::white);
        setColour(ListBox::backgroundColourId,    Colours::white);

        Desktop::getInstance().getDefaultLookAndFeel().setColour(TextButton::textColourOffId, Colours::black);
        Desktop::getInstance().getDefaultLookAndFeel().setColour(TextEditor::textColourId,    Colours::black);

        setColour(TextButton::textColourOnId,        Colours::black);
        setColour(TooltipWindow::backgroundColourId, Colours::lightyellow);
        setColour(TooltipWindow::outlineColourId,    Colours::black);
        setColour(TooltipWindow::textColourId,       Colours::black);

        setColour(TextEditor::outlineColourId,       Colours::black);
    }
    ~theme() { }

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

private:
    colors _colors;
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class header_button_lf : public theme
{
public:
     header_button_lf() { }
    ~header_button_lf() { }

    void drawButtonBackground(Graphics& g,
        Button& button,
        const Colour&,
        bool hover,
        bool pressed) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        bounds.removeFromBottom(.5f);

        if (pressed)
            g.setColour(_colors.get(color_ids::button_pressed));
        else if (hover)
            g.setColour(_colors.get(color_ids::button_hover));
        else
            g.setColour(_colors.get(color_ids::bg_light));

        g.fillRect(bounds);
        g.setColour(_colors.get(color_ids::outline_dark));

        if (button.isConnectedOnBottom())
            g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getWidth(), bounds.getBottom());

        g.setColour(_colors.get(color_ids::outline));
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getY());
    }

    void drawButtonText(Graphics& g, TextButton& button, bool, bool)
    {
        Font font(jmin(16.f, button.getHeight() * .6f));
        font.setBold(true);
        g.setFont(font);
        g.setColour(_colors.get(color_ids::outline));

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
    colors _colors;
};

}