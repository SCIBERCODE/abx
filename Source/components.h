#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

#include "ftdi.h"
#include "theme.h"
#include "track.h"
#include "track_master.h"
#include "toolbar.h"

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

struct trial_t {
    bit_t button;
    bit_t relay;
    bool  blind;
};

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
    void track_activate(track*, bool double_click);
    void track_change(track* _track, bool is_next);
    void change_state(state_t new_state);
    void trial_cycle(bit_t button_bt);
    void trial_save();
    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    class comp_tracks : public Component {
    public:
        comp_tracks(OwnedArray<track>& tracks) : _tracks(tracks) { }
        ~comp_tracks() { };
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
        OwnedArray<track>& _tracks;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_tracks)
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    class timer_long_pressed_button : public Timer
    {
    private:
        bit_t _button_pressed;
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
        _settings.getUserSettings()->setValue("gain_a", _master_track.get_gain(0));
        _settings.getUserSettings()->setValue("gain_b", _master_track.get_gain(1));
        _settings.saveIfNeeded();
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    std::function<void(uint8_t buttons_pressed)> on_button_press = [this](uint8_t buttons_pressed)
    {
        std::bitset<8>btns(buttons_pressed);
        btns.flip();
        bit_t btn = unknown;
        if (btns.test(btn_rev)) btn = btn_rev;
        if (btns.test(btn_a))   btn = btn_a;
        if (btns.test(btn_hz))  btn = btn_hz;
        if (btns.test(btn_b))   btn = btn_b;
        if (btns.test(btn_fwd)) btn = btn_fwd;

        _toolbar.hard_press(toolbar::button_t::rev, btn == btn_rev);
        _toolbar.hard_press(toolbar::button_t::fwd, btn == btn_fwd);
        _toolbar.repaint();

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
                _toolbar.click(toolbar::button_t::open);
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
    class comp_results_toolbar : public Component {
    public:
        comp_results_toolbar() {
            _label_result.setFont(Font("DilleniaUPC", 65, Font::FontStyleFlags::bold).withHorizontalScale(1.15f));//Gisha
            _label_result.setColour(Label::textColourId, Colours::white);
            _label_result.setText("0 / 0, -.--", dontSendNotification);
            _label_result.setJustificationType(Justification::centred);
            addAndMakeVisible(_label_result);

            _button_clear.set_border_radius_side(button_toolbar::border_radius_side_t::all);
            _button_clear.set_icon("clear");
            _button_clear.set_type(button_toolbar::button_t::utility);
            addAndMakeVisible(_button_clear);
            //button_clear.setEnabled(false);
        }
        ~comp_results_toolbar() {};

        void set_on_clear(const std::function<void()>& callback) {
            _button_clear.onClick = callback;
        }

        void paint(Graphics& g) override {
            g.fillAll(_colors.get(color_ids::bg_dark).brighter(.3f));
            g.setColour(_colors.get(color_ids::outline_dark));
            auto bounds = getLocalBounds();
            g.drawLine(
                static_cast<float>(bounds.getX()),
                static_cast<float>(bounds.getBottom()),
                static_cast<float>(bounds.getWidth()),
                static_cast<float>(bounds.getBottom()));
        };

        void resized() override {
            auto area = getLocalBounds();
            _label_result.setBounds(area.withTrimmedBottom(2));
            area.reduce(4, 4);
            auto button_size = _button_clear.get_size().first;
            _button_clear.setBounds(area.removeFromRight(button_size).withSizeKeepingCentre(button_size, button_size));
        };

        void set_result(String text) {
            _label_result.setText(text, sendNotificationAsync);
        }
    private:
        Label          _label_result;
        button_toolbar _button_clear;
        colors         _colors;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_results_toolbar)
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    class comp_audio_setup : public Component
    {
    public:
        comp_audio_setup(AudioDeviceManager& device_manager)
            : _comp_audio_setup(device_manager, 0, 256, 0, 256, false, false, false, false)
        {
            addAndMakeVisible(_comp_audio_setup);
            setSize(520, 450);
        }
        void paint(Graphics& g) override {
            g.fillAll(_colors.get(color_ids::bg_light));
        }
        void resized() override {
            _comp_audio_setup.setBounds(getLocalBounds()); // todo: [5]
        }
    private:
        AudioDeviceSelectorComponent _comp_audio_setup;
        colors                       _colors;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_audio_setup)
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    class window_audio_setup : public DocumentWindow
    {
    public:
        window_audio_setup(AudioDeviceManager& device_manager)
            : DocumentWindow(
                "abx audio settings",
                Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons
            )
        {
            setLookAndFeel(&_theme);
            setTitleBarHeight(20);
            setUsingNativeTitleBar(false);
            auto main_comp = std::make_unique<comp_audio_setup>(device_manager);
            setContentOwned(main_comp.release(), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            addToDesktop();
        }
        ~window_audio_setup() {
            setLookAndFeel(nullptr);
        }
        void closeButtonPressed() {
            exitModalState(0);
        }
    private:
        theme _theme;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(window_audio_setup)
    };

    void launch_audio_setup() {
        auto audio_setup = std::make_unique<window_audio_setup>(deviceManager).release();
        audio_setup->enterModalState(true, nullptr, true);
    }

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
private:
    AudioTransportSource                _transport_source;
    state_t                             _state         { state_t::stopped };
    track*                              _current_track { };
    std::vector<trial_t>                _trials        { };
    double                              _relay         { },
                                        _current_sample_rate,
                                        _current_skip_interval;
    std::unique_ptr<WildcardFileFilter> _filter;
    OwnedArray<track>                   _tracks;
    ApplicationProperties               _settings;
    ftdi                                _ftdi;

    track_master                 _master_track;
    toolbar                      _toolbar;
    Viewport                     _viewport_tracks;
    std::unique_ptr<comp_tracks> _viewport_tracks_inside;
    TextButton                   _button_results_header { "trial score" };
    comp_results_toolbar         _results_toolbar;
    window_audio_setup           _window_audio_setup;
    header_button_lf             _theme_header;
    timer_long_pressed_button    _timer_long_pressed_button;
    colors                       _colors;

    bool _user_stopped     { },
         _fast_reverse     { },
         _fast_forward     { },
         _results_expanded { };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(comp_main)
};

}
