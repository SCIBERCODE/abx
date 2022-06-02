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
    bit_t button;
    bit_t relay;
    bool  blind;
};

namespace settings_keys {
    static StringRef last_path { "last_path" };
    static StringRef gain_a    { "gain_a"    };
    static StringRef gain_b    { "gain_b"    };
    static StringRef name_a    { "name_a"    };
    static StringRef name_b    { "name_b"    };
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
    void trial_cycle(bit_t button_bt);
    void launch_audio_setup();

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    class timer_long_pressed_button : public Timer
    {
    private:
        bit_t _button_pressed { };
        bool& _fast_reverse;
        bool& _fast_forward;
    public:
        timer_long_pressed_button(bool& fast_reverse, bool& fast_forward) :
            _fast_reverse(fast_reverse),
            _fast_forward(fast_forward) { };
        ~timer_long_pressed_button()    { };

        void pressed(bit_t button_pressed) {
            _button_pressed = button_pressed;
            startTimer(500);
        }

        void timerCallback() override {
            stopTimer();
            if (btn_rev == _button_pressed) _fast_reverse = true;
            if (btn_fwd == _button_pressed) _fast_forward = true;
        }
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(timer_long_pressed_button);
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void()> gain_changed_callback = [this]()
    {
        static bit_t last_relay;
        double gain;
        std::bitset<8> relay_bt(static_cast<uint64_t>(_relay));
        if (relay_bt.test(relay_a)) {
            gain = _master_track.get_gain(0);
            _master_track.get_processor().set_gain(gain, last_relay != relay_a);
            last_relay = relay_a;
        }
        if (relay_bt.test(relay_b)) {
            gain = _master_track.get_gain(1);
            _master_track.get_processor().set_gain(gain, last_relay != relay_b);
            last_relay = relay_b;
        }
        _settings.getUserSettings()->setValue(settings_keys::gain_a, _master_track.get_gain(0));
        _settings.getUserSettings()->setValue(settings_keys::gain_b, _master_track.get_gain(1));
        _settings.saveIfNeeded();
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void(uint8_t new_relay)> relay_change_callback = [this](uint8_t new_relay)
    {
        DBG("relay_change_callback");
        _relay = new_relay;
        auto blind = _toolbar.get_state(comp_toolbar::button_t::blind);
        if (new_relay) {
            gain_changed_callback();
        }
        if (!blind)
        {
            std::bitset<8> relay_bt(new_relay);
            _toolbar.hard_press(comp_toolbar::button_t::a, relay_bt.test(relay_a));
            _toolbar.hard_press(comp_toolbar::button_t::b, relay_bt.test(relay_b));
            _toolbar.repaint();
        }
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void(uint8_t buttons_pressed)> on_button_press = [this](uint8_t buttons_pressed)
    {
        DBG(std::format("on_button_press: {}", buttons_pressed));
        std::bitset<8>btns(buttons_pressed);
        btns.flip();
        bit_t btn = unknown;
        if (btns.test(btn_rev)) btn = btn_rev;
        if (btns.test(btn_a))   btn = btn_a;
        if (btns.test(btn_hz))  btn = btn_hz;
        if (btns.test(btn_b))   btn = btn_b;
        if (btns.test(btn_fwd)) btn = btn_fwd;

        _toolbar.hard_press(comp_toolbar::button_t::rev, btn == btn_rev);
        _toolbar.hard_press(comp_toolbar::button_t::fwd, btn == btn_fwd);
        _toolbar.hard_press(comp_toolbar::button_t::hz,  btn == btn_hz);

        bit_t relay { unknown };
        std::bitset<8> relay_bt(_ftdi.get_relay());

        if (relay_bt.test(relay_a)) relay = relay_a;
        if (relay_bt.test(relay_b)) relay = relay_b;

        auto correct = (btn == btn_a && relay == relay_a) || (btn == btn_b && relay == relay_b);

        _toolbar.hard_press(comp_toolbar::button_t::a, btn == btn_a, !correct);
        _toolbar.hard_press(comp_toolbar::button_t::b, btn == btn_b, !correct);
        _toolbar.repaint();

        if (!_ready) return;

        if (btn == btn_rev || btn == btn_fwd) {
            _timer_long_pressed_button.pressed(btn);
        }
        static auto last_button = unknown;

        if (buttons_pressed == _ftdi.get_buttons_mask()) {
            _timer_long_pressed_button.stopTimer();

            if (_fast_reverse || _fast_forward) {
                _fast_reverse = _fast_forward = false;
            }
            else {
                if (last_button == btn_rev) track_change(_current_track, false);
                if (last_button == btn_fwd) track_change(_current_track, true);
            }
            return;
        }

        if (_tracks.size() == 0) {
            if (btn != unknown) {
                _toolbar.click(comp_toolbar::button_t::open);
            }
            return;
        }
        if (btn == btn_a || btn == btn_hz || btn == btn_b)
        {
            if (btn != unknown) {
                trial_cycle(btn);
            }
        }
        last_button = btn;
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
            case btn_hz:
                btn_text  = "?";
                css_class = "param";
                break;
            case btn_a: btn_text = "A"; break;
            case btn_b: btn_text = "B"; break;
            default:    btn_text = "~";
            }
            switch (trial.relay) {
            case relay_a: relay_text = "A"; break;
            case relay_b: relay_text = "B"; break;
            default:      relay_text = "~";
            }
            if (trial.button != btn_hz) {
                {
                    if ((trial.button == btn_a && trial.relay == relay_a) ||
                        (trial.button == btn_b && trial.relay == relay_b))
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

        auto last_path = _settings.getUserSettings()->getValue(settings_keys::last_path);
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
    AudioTransportSource                  _transport_source;
    state_t                               _state         { state_t::stopped };
    comp_track*                           _current_track { };
    std::vector<trial_t>                  _trials        { };
    double                                _relay         { },
                                          _current_sample_rate,
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