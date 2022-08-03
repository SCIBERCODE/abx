#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

namespace abx {

namespace settings_ids {
    static Identifier tracks { "tracks" };
    static Identifier track  { "track"  };
    static Identifier gain   { "gain"   };
    static Identifier path   { "path"   };
    static Identifier name   { "name"   };
}

struct track_options {
    track_options() = default;
    track_options(const String& sparams)
    {
        auto params = StringArray::fromTokens(sparams, "|", "");
        if (params.size() == 3) {
            active = params[0].getIntValue() != 0;
            marker = params[1].getDoubleValue();
            path   = params[2];
        }
    }

    String to_string() const {
        StringArray params;
        params.add(active ? "1" : "0");
        params.add(String(marker));
        params.add(path);
        return params.joinIntoString("|");
    }

    String path   = "";
    double marker = 0.;
    bool   active = false;
};

class settings :
    public ValueTree::Listener {
public:

    settings() {
        if (_settings_file.existsAsFile())
        {
            if (auto xml = parseXML(_settings_file)) {
                _settings = ValueTree::fromXml(*xml);
            }
        }
        _settings.addListener(this);
    }

    void save(const Identifier id, const Array<var> values) {
        StringArray strings;
        for (const auto& value : values) {
            strings.add(value.toString());
        }
        _settings
            .getOrCreateChildWithName(id, nullptr)
            .setProperty("value", strings.joinIntoString("|"), nullptr);
    }

    void save_tracks(Array<track_options> tracks) {
        auto node = _settings.getOrCreateChildWithName(settings_ids::tracks, nullptr);
        node.removeAllChildren(nullptr);
        for (auto const& track : tracks)
        {
            ValueTree new_node(settings_ids::track);
            new_node.setProperty("value", track.to_string(), nullptr);
            node.appendChild(new_node, nullptr);
        }
    }

    Array<track_options> load_tracks() {
        auto tracks_node = _settings.getChildWithName(settings_ids::tracks);
        if (!tracks_node.isValid()) return {};

        Array<track_options> tracks;
        for (int k = 0; k < tracks_node.getNumChildren(); k++)
        {
            track_options options(tracks_node.getChild(k).getProperty("value"));
            tracks.add(options);
        }
        return tracks;
    }

    StringArray read(const Identifier id) {
        auto child = _settings.getChildWithName(id);
        if (child.isValid()) {
            return StringArray::fromTokens(child["value"].toString(), "|", "");
        }
        return {};
    }

    String read_single(const Identifier id) {
        auto values = read(id);
        if (values.isEmpty()) return {};
        return values[0];
    }

private:

    ValueTree _settings      { ProjectInfo::projectName };
    File      _settings_file { File::getCurrentWorkingDirectory().getChildFile(String(ProjectInfo::projectName) + ".xml").getFullPathName() };


    void _save() {
        _settings.createXml()->writeTo(_settings_file);
    }

    void valueTreePropertyChanged(ValueTree&, const Identifier&) override { _save(); }
    void valueTreeChildRemoved(ValueTree&, ValueTree&, int)      override { _save(); }
    void valueTreeChildOrderChanged(ValueTree&, int, int)        override { _save(); }
    void valueTreeChildAdded(ValueTree&, ValueTree&)             override { _save(); }
    void valueTreeParentChanged(ValueTree&)                      override { _save(); }
};

}