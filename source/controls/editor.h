#pragma once

#pragma warning(disable : 4706)

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class editor : public TextEditor,
               public ChangeBroadcaster
{
private:
    class listener : public TextEditor::Listener
    {
    public:
        void textEditorTextChanged(TextEditor& editor) override {
            _text = editor.getText();
        }
        auto get_text() {
            return _text;
        }
    private:
        String _text;
    };

    class laf : public juce::LookAndFeel_V4
    {
    public:
        laf() {
            set_enabled(false);
            setColour(TextEditor::focusedOutlineColourId,  Colours::black);
            setColour(TextEditor::backgroundColourId,      Colours::transparentWhite);
            setColour(TextEditor::outlineColourId,        _colours.get(colour_id::outline));
        }
        auto get_colour(size_t index) {
            return _colours.get(index);
        }
        void set_enabled(bool enabled) {
            setColour(TextEditor::highlightedTextColourId, enabled ? Colours::black : Colours::grey);
            setColour(TextEditor::textColourId,            enabled ? Colours::black : Colours::grey);
        }
        void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& editor) override
        {
            auto focus = editor.hasKeyboardFocus(true) && !editor.isReadOnly();
            g.setColour(findColour(focus ? TextEditor::focusedOutlineColourId : TextEditor::outlineColourId));
            g.drawRect(0.f, 0.f, static_cast<float>(width), static_cast<float>(height), focus ? 1.3f : 1.f);
        }
        void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& editor) override
        {
            auto focus = editor.hasKeyboardFocus(true) && !editor.isReadOnly();
            g.setColour(focus ? Colours::white : findColour(TextEditor::backgroundColourId));
            g.fillRect(0, 0, width, height);
        }
    private:
        abx::colours _colours;
    };

    listener _listener;
    String   _text_empty;
    laf      _laf;
    bool     _active = false;

public:

    editor(String text_empty) : _text_empty(text_empty)
    {
        setLookAndFeel(&_laf);
        setIndents(3, 0);
        setText(text_empty, false);
        setMouseCursor(MouseCursor::NormalCursor);
        addListener(&_listener);
    }

    ~editor() {
        setLookAndFeel(nullptr);
    }

    void parentHierarchyChanged() override
    {
        if (getParentComponent())
            Desktop::getInstance().addGlobalMouseListener(this);
        else
            Desktop::getInstance().removeGlobalMouseListener(this);
    }

    void mouseDown(const MouseEvent& e) override
    {
        if (_active = getScreenBounds().contains(e.getScreenPosition()))
        {
            TextEditor::mouseDown(e);

            if (_listener.get_text().length() == 0) {
                _laf.set_enabled(true);
                setText(String{}, false);
            }

            setReadOnly(false);
            setMouseCursor(MouseCursor::IBeamCursor);
            setColour(TextEditor::highlightColourId, _laf.get_colour(colour_id::header));
        }
        else {
            if (_listener.get_text().length() == 0) {
                _laf.set_enabled(false);
                setText(_text_empty, false);
            }

            setReadOnly(true);
            setMouseCursor(MouseCursor::NormalCursor);
            setColour(TextEditor::highlightColourId, Colours::transparentWhite);
        }
        sendChangeMessage();
    }

    auto is_active() {
        return _active;
    }

    void text_set(const String& text)
    {
        if (text.length()) {
            _laf.set_enabled(true);
            setText(text, true);
        }
        else
            setText(_text_empty, true);
    }

    auto text_get()
    {
        return _listener.get_text();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(editor)
};

}