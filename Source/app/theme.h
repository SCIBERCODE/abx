#pragma once

namespace abx {

static Drawable* get_drawable(String icon_name, int width = 16, int height = 16) {
    int bytes;
    if (auto binary_data = BinaryData::getNamedResource((icon_name + "_svg").getCharPointer(), bytes))
    {
        std::unique_ptr<XmlElement> icon_svg_xml(XmlDocument::parse(binary_data));
        auto drawable_svg = Drawable::createFromSVG(*(icon_svg_xml.get()));

        drawable_svg->setTransformToFit(
            juce::Rectangle<float>(0.f, 0.f, static_cast<float>(width), static_cast<float>(height)),
            RectanglePlacement::centred
        );
        return drawable_svg.release();
    }
    return nullptr;
}

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
        button_toogled,
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

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class colors
{
public:
    colors() {
        init_colors();
    }
    Colour get(size_t index) {
        return _colours.count(index) ? _colours.at(index) : Colours::darkgrey;
    }
private:
    std::map<size_t, Colour> _colours;

    void init_colors() {
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
        _colours[button_toogled   ] = {  97, 207, 165 };
        // track
        _colours[waveform         ] = {  50,  50, 200 };
        _colours[waveform_light   ] = { 100, 100, 220 };
        _colours[waveform_disabled] = { 136, 136, 144 };
        _colours[waveform_bg      ] = { 192, 192, 192 };
        _colours[header_focused   ] = { 213, 227, 255 };
        _colours[header_lines     ] = { 128, 128, 128 };
    }
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
    }

    void drawDocumentWindowTitleBar(DocumentWindow& window, Graphics& g,
        int, int height, int x, int,
        const Image*, bool) override
    {
        g.setColour(getCurrentColourScheme().getUIColour(ColourScheme::widgetBackground));
        g.fillAll();

        Font font(height * .6f, Font::bold);
        g.setFont(font);

        auto text_width = font.getStringWidth(window.getName());
        g.setColour(window.findColour(DocumentWindow::textColourId));
        g.drawText(window.getName(), x, 0, text_width, height, Justification::centredLeft, true);
    }

    void drawTextEditorOutline(Graphics&, int /*width*/, int /*height*/, TextEditor&) override
    {
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