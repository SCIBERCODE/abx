#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../app/ftdi.h"
#include "../app/theme.h"
#include "../app/windows.h"
#include "../app/settings.h"

namespace command_ids
{
    constexpr int add_files = 1;
    constexpr int play      = 2;
    constexpr int fwd       = 3;
    constexpr int rev       = 4;
    constexpr int rewind    = 5;
    constexpr int options   = 6;
};

#include "track.h"
#include "tracks_viewport.h"

#include "toolbar.h"
#include "toolbar_results.h"
#include "toolbar_bottom.h"

namespace abx {

template<typename T>
using pair_ptr = std::pair<std::unique_ptr<T>, std::unique_ptr<T>>; // todo: make it work

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

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class comp_main : public AudioAppComponent,
                  public FileDragAndDropTarget,
                  public DragAndDropContainer,
                  public ChangeListener,
                  public ApplicationCommandTarget
{
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

    auto get_keys() {
        return &_commands;
    }

private:
    comp_track *track_add(const String &file_path, double marker = 0., bool save_settings = true);
    void        track_activate(comp_track*, bool double_click);
    void        track_change(comp_track* _track, bool is_next);
    void        tracks_state_save();
    void        change_state(state_t new_state);
    void        trial_display();
    void        trial_cycle(size_t button_bt);
    void        launch_audio_setup();

    ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }

    void getAllCommands(Array<CommandID>& commands) override
    {
        Array<CommandID> ids {
            command_ids::add_files, command_ids::options,
            command_ids::play,      command_ids::rewind,
            command_ids::fwd,       command_ids::rev
        };
        commands.addArray(ids);
    }

    void getCommandInfo(CommandID cmd_id, ApplicationCommandInfo& result) override
    {
        switch (cmd_id)
        {
        case command_ids::add_files:
            result.setInfo("Add file(s)", "Add file(s)\r\n[" + _filter->getDescription().removeCharacters("*.") + "]\r\n", "Button", 0);
            result.addDefaultKeypress('o', 0);
            result.addDefaultKeypress('o', ModifierKeys::ctrlModifier);
            break;
        case command_ids::play:
            result.setInfo("Play", "Play current track", "Button", 0);
            result.addDefaultKeypress('p', 0);
            break;
        case command_ids::fwd:
            result.setInfo("Forward", "Skip to the next track", "Button", 0);
            result.addDefaultKeypress('f', 0);
            result.addDefaultKeypress('.', ModifierKeys::shiftModifier);
            break;
        case command_ids::rev:
            result.setInfo("Previous", "Skip to the previous track", "Button", 0);
            result.addDefaultKeypress('r', 0);
            result.addDefaultKeypress(',', ModifierKeys::shiftModifier);
            break;
        case command_ids::rewind:
            result.setInfo("Rewind", "Go to start & clear marker", "Button", 0);
            result.addDefaultKeypress('r', ModifierKeys::ctrlModifier);
            break;
        case command_ids::options:
            result.setInfo("Options", "Audio device preferences", "Button", 0);
            result.addDefaultKeypress('p', ModifierKeys::ctrlModifier);
            break;
        }
    }

    bool perform(const InvocationInfo& info) override;

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

        auto gain = _toolbar.gain_get();
        _toolbar.get_processor().set_gain(
            _relay == _A ? gain.first : gain.second, last_relay != _relay
        );
        last_relay = _relay;
        _settings.save(settings_ids::gain, { gain.first, gain.second });
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
    std::function<void(String)> ftdi_status_change_callback = [this](String device)
    {
        _toolbar_bottom.set_device_ftdi(device);
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

        auto last_path = _settings.read_single(settings_ids::path);
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
    comp_track*                           _current_track {};
    std::vector<trial_t>                  _trials        {};
    size_t                                _relay         {};
    double                                _current_sample_rate,
                                          _current_skip_interval;
    std::unique_ptr<WildcardFileFilter>   _filter;
    OwnedArray<comp_track>                _tracks;
    ftdi                                  _ftdi;
    settings                              _settings;
    ApplicationCommandManager             _commands;
    comp_toolbar                          _toolbar;
    comp_toolbar_results                  _toolbar_results;
    comp_toolbar_bottom                   _toolbar_bottom;
    Viewport                              _viewport_tracks;
    std::unique_ptr<comp_tracks_viewport> _viewport_tracks_inside;
    TextButton                            _button_results_header { "trial score" };

    window_audio_setup                    _window_audio_setup;
    header_button_lf                      _theme_header;
    timer_long_pressed_button             _timer_long_pressed_button;
    colours                               _colours;

    bool              _fast_reverse     = false,
                      _fast_forward     = false,
                      _results_expanded = false;
    std::atomic<bool> _user_stopped     = false;
    std::atomic<bool> _ready            = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_main)
};

}