#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/ftdi.h"
#include "../app/theme.h"
#include "../app/windows.h"

#include "track.h"
#include "track_master.h"
#include "tracks_viewport.h"

#include "toolbar.h"
#include "toolbar_results.h"

namespace abx {

enum class state_t
{
    stopped,
    starting,
    playing,
    pausing,
    paused,
    stopping
};

const std::map<state_t, std::string> state_names = {
    {state_t::stopped,  "stopped"  },
    {state_t::starting, "starting" },
    {state_t::playing,  "playing"  },
    {state_t::pausing,  "pausing"  },
    {state_t::paused,   "paused"   },
    {state_t::stopping, "stopping" }
};

struct trial_t
{
    size_t button;
    size_t relay;
    bool   blind;
};

namespace settings_keys {
    static StringRef last_path { "last_path" };
    static StringRef gain      { "gain"      };
    static StringRef name      { "name"      };
    static StringRef tracks    { "tracks"    };
    static StringRef track     { "track"     };
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_main : public AudioAppComponent,
                  public FileDragAndDropTarget,
                  public DragAndDropContainer,
                  public ChangeListener {
public:
     comp_main();
    ~comp_main();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) noexcept override;
    void releaseResources() override;
    void paint(Graphics& g) override;
    void resized() override;
    bool isInterestedInFileDrag(const StringArray &files) override;
    void filesDropped(const StringArray &files, int x, int y) override;
    void changeListenerCallback(ChangeBroadcaster* source) override;

private:
    void track_add(const String &file_path);
    void track_activate(comp_track*, bool double_click);
    void track_change(comp_track* _track, bool is_next);
    void change_state(state_t new_state);
    void trial_cycle(size_t button_bt);
    void launch_audio_setup();

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    class timer_long_pressed_button : public Timer
    {
    private:
        size_t _button {};
        bool&  _fast_reverse;
        bool&  _fast_forward;
    public:
        timer_long_pressed_button(bool& fast_reverse, bool& fast_forward) :
            _fast_reverse(fast_reverse),
            _fast_forward(fast_forward) {};
        ~timer_long_pressed_button()    {};

        void pressed(size_t button) {
            _button = button;
            startTimer(500);
        }

        void timerCallback() override {
            stopTimer();
            if (_button == _REV) _fast_reverse = true;
            if (_button == _FWD) _fast_forward = true;
        }
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(timer_long_pressed_button);
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void()> gain_changed_callback = [this]()
    {
        static size_t last_relay {};

        auto gain = _master_track.gain_get();
        _master_track.get_processor().set_gain(
            _relay == _A ? gain.first : gain.second, last_relay != _relay
        );
        last_relay = _relay;
        settings_save(settings_keys::gain, std::format("{}\n{}", gain.first, gain.second));
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void(size_t)> relay_change_callback = [this](size_t relay)
    {
        DBG("relay_change_callback");

        _relay = relay;
        auto blind = _toolbar.get_state(comp_toolbar::button_t::blind);
        if (relay) {
            gain_changed_callback();
        }
        if (!blind)
        {
            _toolbar.hard_press(comp_toolbar::button_t::a, relay == _A);
            _toolbar.hard_press(comp_toolbar::button_t::b, relay == _B);
            _toolbar.repaint();
        }
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void(size_t)> on_button_press = [this](size_t button)
    {
        DBG(std::format("on_button_press: button = {}", button));

        _toolbar.hard_press(comp_toolbar::button_t::rev, button == _REV);
        _toolbar.hard_press(comp_toolbar::button_t::fwd, button == _FWD);
        _toolbar.hard_press(comp_toolbar::button_t::hz,  button == _HZ);

        auto correct = button == _ftdi.get_relay();
        _toolbar.hard_press(comp_toolbar::button_t::a, button == _A, !correct);
        _toolbar.hard_press(comp_toolbar::button_t::b, button == _B, !correct);
        _toolbar.repaint();

        if (!_ready) return;

        if (button == _REV || button == _FWD) {
            _timer_long_pressed_button.pressed(button);
        }
        static size_t last_button {};

        if (!button) { //! check
            _timer_long_pressed_button.stopTimer();

            if (_fast_reverse || _fast_forward) {
                _fast_reverse = _fast_forward = false;
            }
            else {
                if (last_button == _REV) track_change(_current_track, false);
                if (last_button == _FWD) track_change(_current_track, true);
            }
            return;
        }

        if (_tracks.size() == 0) {
            if (button) {
                _toolbar.click(comp_toolbar::button_t::open);
            }
            return;
        }
        if (button == _A || button == _B || button == _HZ) {
            trial_cycle(button);
        }
        last_button = button;
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void()> trial_save = [this]() {
        std::string html { resources::get_html_header() };
        const char* css_class;
        const char* btn_text;
        const char* relay_text;

        size_t count_all = 0, count_correct = 0;
        for (const auto& trial : _trials)
        {
            if (!trial.blind) continue;

            switch (trial.button) {
            case _HZ:
                btn_text  = "?";
                css_class = "param";
                break;
            case _A: btn_text = "A"; break;
            case _B: btn_text = "B"; break;
            default: btn_text = "~";
            }
            switch (trial.relay) {
            case _A: relay_text = "A"; break;
            case _B: relay_text = "B"; break;
            default: relay_text = "~";
            }
            if (trial.button != _HZ) {
                {
                    if (trial.button == trial.relay)
                    {
                        count_correct++;
                        css_class = "ok";
                    }
                    else {
                        css_class = "nope";
                    }
                }
                count_all++;
            }
            auto pval = abs(((count_all / 2.) - count_correct) / sqrt(count_all / 4.));
            html += std::format("<span class = 'static'>{:.4f}</span><span class = '{}'>    BUTTON_{} / RELAY_{}    </span>\r\n",
                pval, css_class, btn_text, relay_text);
        }
        html += "</pre>\r\n</body>\r\n</html>";

        auto last_path = settings_read(settings_keys::last_path).getFirst().toString();
        FileChooser chooser("Save Trial Log to...", last_path, "*.html", true, false, this);
        if (chooser.browseForFileToSave(true))
        {
            File html_file(chooser.getResult());
            html_file.replaceWithText(html);
        }
    };

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
private:
    Array<var> settings_read(const StringRef key) {
        Array<var> result;
        auto value = _settings.getUserSettings()->getValue(key);
        if (value.containsChar('\n'))
        {
            StringArray tokens;
            tokens.addTokens(value, "\n", "");
            for (auto const & token : tokens) {
                result.add(token);
            }
        }
        else {
            result.add(value);
        }
        return result;
    }

    void settings_save(const StringRef key, const String& value) {
        _settings.getUserSettings()->setValue(key, value);
        _settings.saveIfNeeded();
    }


    AudioTransportSource                  _transport_source;
    state_t                               _state         { state_t::stopped };
    comp_track*                           _current_track {};
    std::vector<trial_t>                  _trials        {};
    size_t                                _relay         {};
    double                                _current_sample_rate,
                                          _current_skip_interval;
    std::unique_ptr<WildcardFileFilter>   _filter;
    OwnedArray<comp_track>                _tracks;
    ApplicationProperties                 _settings;
    ftdi                                  _ftdi;

    comp_track_master                     _master_track;
    comp_toolbar                          _toolbar;
    comp_toolbar_results                  _toolbar_results;
    Viewport                              _viewport_tracks;
    std::unique_ptr<comp_tracks_viewport> _viewport_tracks_inside;
    TextButton                            _button_results_header { "trial score" };

    window_audio_setup                    _window_audio_setup;
    header_button_lf                      _theme_header;
    timer_long_pressed_button             _timer_long_pressed_button;
    colors                                _colors;

    bool              _fast_reverse     = false,
                      _fast_forward     = false,
                      _results_expanded = false;
    std::atomic<bool> _user_stopped     = false;
    std::atomic<bool> _ready            = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_main)
};

}