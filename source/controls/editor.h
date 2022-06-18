#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class editor : public TextEditor
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
            setColour(TextEditor::highlightedTextColourId, Colours::black);
            setColour(TextEditor::focusedOutlineColourId,  Colours::black);
            setColour(TextEditor::backgroundColourId,      Colours::transparentWhite);
            setColour(TextEditor::outlineColourId,        _colours.get(colour_id::outline));
        }
        auto get_colour(size_t index) {
            return _colours.get(index);
        }
    private:
        abx::colours _colours;
    };

    listener _listener;
    String   _text_empty;
    laf      _laf;

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

    void paint(Graphics& g) override
    {
        auto focus = hasKeyboardFocus(true) && !isReadOnly();

        g.setColour(focus ? Colours::white : findColour(TextEditor::backgroundColourId));
        g.fillRect(getLocalBounds());

        g.setColour(findColour(focus ? TextEditor::focusedOutlineColourId : TextEditor::outlineColourId));
        g.drawRect(getLocalBounds());
    }

    void mouseDown(const MouseEvent& e) override
    {
        if (getScreenBounds().contains(e.getScreenPosition()))
        {
            TextEditor::mouseDown(e);

            if (_listener.get_text().length() == 0)
                setText(String {}, false);

            setReadOnly(false);
            setMouseCursor(MouseCursor::IBeamCursor);
            setColour(TextEditor::highlightColourId, _laf.get_colour(colour_id::header));
        }
        else {
            if (_listener.get_text().length() == 0)
                setText(_text_empty, false);

            setReadOnly(true);
            setMouseCursor(MouseCursor::NormalCursor);
            setColour(TextEditor::highlightColourId, Colours::transparentWhite);
        }
    }

    void text_set(const String& text)
    {
        if (text.length())
            setText(text, true);
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