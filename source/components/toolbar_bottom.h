#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/theme.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_toolbar_bottom : public Component {
public:
    comp_toolbar_bottom() {
        setOpaque(true);

        auto size = get_size();
        setSize(size.first, size.second);
    };

    void paint(Graphics& g) override {
        g.fillAll(_colours.get(colour_id::bg_light));
        g.setColour(Colours::darkgrey.darker(0.7f));
        g.drawLine(0.f, .5f, static_cast<float>(getBounds().getWidth()), .5f, 1.f);
    };

    void resized() override {
    };

    std::pair<int, int> get_size() const {
        return std::make_pair(300, 56);
    };

private:
    colours _colours;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_toolbar_bottom)
};

}
