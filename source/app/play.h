#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class track_processor
{
public:
    track_processor() {
        _format_manager.registerBasicFormats();
    };

    void process(AudioBuffer<float>& buffer) noexcept {
        for (auto ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            if (_gain.get() != _gain_previous) {
                buffer.applyGainRamp(ch, 0, buffer.getNumSamples(), static_cast<float>(_gain_previous), static_cast<float>(_gain.get()));
                _gain_previous = _gain.get();
            }
            else {
                buffer.applyGain(ch, 0, buffer.getNumSamples(), static_cast<float>(_gain.get()));
            }
        }
    };

    void gain_set(double db, bool reset = false) {
        _gain = std::pow(10., db / 20.);
        if (reset) {
            _gain_previous = 0.;
        }
    }

    auto gain_get() {
        return _gain.get();
    }

    void load_file_to_transport(File file, AudioTransportSource& transport_source)
    {
        if (file.existsAsFile())
        {
            auto reader = _format_manager.createReaderFor(file);
            if (reader == nullptr) return;

            auto reader_source = std::make_unique<AudioFormatReaderSource>(reader, true);
            if (!reader_source) return;

            transport_source.setSource(nullptr);
            _current_file_source.reset(reader_source.release());
            if (!_current_file_source) return;

            transport_source.setSource(_current_file_source.get(), 0, nullptr, reader->sampleRate);
        }
        else {
            _current_file_source.reset(nullptr);
            transport_source.setSource(nullptr);
        }
    };

    AudioFormatManager& getFormatManager() {
        return _format_manager;
    }

private:
    AudioFormatManager                       _format_manager;
    std::unique_ptr<AudioFormatReaderSource> _current_file_source;
    Atomic<double>                           _gain          { 1.f };
    double                                   _gain_previous { 1.f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(track_processor)
};

}