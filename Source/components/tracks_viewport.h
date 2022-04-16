#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_tracks_viewport : public Component {
public:
    comp_tracks_viewport(OwnedArray<comp_track>& tracks) : _tracks(tracks) { }
    ~comp_tracks_viewport() { };
public:
    void resized() override {
        auto area = getLocalBounds();
        area.removeFromTop(3);
        for (auto track : _tracks) {
            auto track_height = track->getHeight();
            track->setBounds(area.removeFromTop(track_height));
            area.removeFromTop(2);
        }
    };
private:
    OwnedArray<comp_track>& _tracks;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_tracks_viewport)
};

}