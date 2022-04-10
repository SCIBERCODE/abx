#include "components.h"

namespace abx {

comp_main::comp_main() :
    _window_audio_setup(deviceManager),
    _timer_long_pressed_button(_fast_reverse, _fast_forward)
{
    setOpaque(true);

    PropertiesFile::Options params;
    params.applicationName = L"abx";
    params.filenameSuffix  = L".xml";
    params.folderName      = File::getCurrentWorkingDirectory().getFullPathName();
    params.storageFormat   = PropertiesFile::storeAsXML;
    _settings.setStorageParameters(params);

    _viewport_tracks.setScrollBarThickness(11);
    _viewport_tracks_inside = std::make_unique<comp_tracks>(_tracks);
    _viewport_tracks.setViewedComponent(_viewport_tracks_inside.get(), false);
    addAndMakeVisible(_viewport_tracks);

    _toolbar.set_on_fwd_clicked([&]() {
        track_change(_current_track, true);
    });
    _toolbar.set_on_rev_clicked([&]() {
        track_change(_current_track, false);
    });

    _toolbar.set_on_rewind_clicked([&](){
        _current_track->rewind();
        if (_state == state_t::paused) {
            _user_stopped = true;
            change_state(state_t::stopped);
        }
        if (_state == state_t::playing) {
            //change_state(state_t::stopping);
        }
    });
    _toolbar.set_on_play_clicked([&](){
        trial_cycle(btn_hz);
    });
    _toolbar.set_on_pause_clicked([&]() {
        if (_state == state_t::playing)
            change_state(state_t::pausing);
        else
            change_state(state_t::starting);
    });
    _toolbar.set_on_stop_clicked([&](){
        if (_state == state_t::paused)
            change_state(state_t::stopped);
        else {
            _user_stopped = true;
            change_state(state_t::stopping);
        }
    });
    _toolbar.set_on_open_clicked([&](){
        static bool already_opened = false;
        if (!already_opened) {
            already_opened = true;
            auto last_path = _settings.getUserSettings()->getValue("last_path");
            FileChooser chooser({ }, last_path, _filter->getDescription(), true, false, this);
            if (chooser.browseForMultipleFilesToOpen()) {
                for (auto const& file : chooser.getResults()) {
                    track_add(file.getFullPathName());
                }
            }
            already_opened = false;
        }
    });
    _toolbar.set_on_choose_clicked(on_button_press);

    _toolbar.set_on_settings_clicked([&]() {
        _window_audio_setup.setVisible(true);
     });

    _toolbar.setAlwaysOnTop(true);
    addAndMakeVisible(_toolbar);

    _master_track.setAlwaysOnTop(true);
    _master_track.set_on_gain_changed(gain_changed_callback);
    _master_track.set_on_name_changed([&]() {
        auto names = _master_track.get_names();
        _settings.getUserSettings()->setValue("a", names.first);
        _settings.getUserSettings()->setValue("b", names.second);
        _settings.saveIfNeeded();
    });
    _master_track.set_names(std::make_pair(
        _settings.getUserSettings()->getValue("a"),
        _settings.getUserSettings()->getValue("b")
    ));
    _master_track.set_volumes(std::make_pair(
        _settings.getUserSettings()->getDoubleValue("gain_a"),
        _settings.getUserSettings()->getDoubleValue("gain_b")
    ));

    addAndMakeVisible(_master_track);

    setSize (900, 650);
    setAudioChannels(0, 2);
    _transport_source.addChangeListener(this);

    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    _filter = std::make_unique<WildcardFileFilter>(formatManager.getWildcardForAllFormats(), String { }, String { });

    auto relay_change_callback = [&](uint8_t new_relay)
    {
        _relay = new_relay;
        if (new_relay == 0) {
            _toolbar.hard_press(toolbar::button_t::a, false);
            _toolbar.hard_press(toolbar::button_t::b, false);
        }
        else {
            gain_changed_callback();
            if (_toolbar.get_state(toolbar::button_t::blind)) return;
            std::bitset<8> relay_bt(new_relay);
            _toolbar.hard_press(toolbar::button_t::a, relay_bt.test(relay_a));
            _toolbar.hard_press(toolbar::button_t::b, relay_bt.test(relay_b));
        }
        _toolbar.repaint();
    };

    _ftdi.set_on_button_press_callback(on_button_press);
    _ftdi.set_on_relay_change_callback(relay_change_callback);

    _toolbar.set_on_blind_clicked([=]()
    {
        if (_toolbar.get_state(toolbar::button_t::blind)) {
            relay_change_callback(0);
        }
        else {
            relay_change_callback(_ftdi.get_relay());
        }
        _toolbar.repaint();
    });

    _button_results_header.setClickingTogglesState(true);
    _button_results_header.setConnectedEdges(TextButton::ConnectedOnBottom);
    _button_results_header.setLookAndFeel(&_theme_header);
    _button_results_header.onClick = [&] {
        _results_expanded = !_results_expanded;
        resized();
    };
    _button_results_header.setToggleState(!_results_expanded, dontSendNotification);
    addAndMakeVisible(_button_results_header);

    _results_toolbar.set_on_clear([&]() {
        _trials.clear();
        _results_toolbar.set_result("0 / 0, -.--");
    });
    addAndMakeVisible(_results_toolbar);
}

comp_main::~comp_main() {
    _ftdi.turn_off_relay();
    Sleep(200);
    setLookAndFeel(nullptr);
    _button_results_header.setLookAndFeel(nullptr);
    shutdownAudio();
    _transport_source.setSource(nullptr);
    _ftdi.stopThread(200);
    _settings.closeFiles();
}

void comp_main::track_activate(track* selected_track_new, bool double_click) {
    if (!double_click) {
        for (auto& _track : _tracks) {
            _track->focus(selected_track_new == _track);
        }
    }
    else {
        auto played_before = _state == state_t::playing;
        if (_current_track != selected_track_new)
        {
            if (_current_track) {
                change_state(state_t::stopped);
                _current_track->activate(false);
                _current_track->repaint();
            }
            _current_track = selected_track_new;
            if (_current_track) {
                _current_track->activate(true);
                _current_track->get_processor().load_file_to_transport(_current_track->getFilePath(), _transport_source);
                _current_track->repaint();
                _transport_source.setPosition(_current_track->get_marker());
                if (played_before) {
                    change_state(state_t::starting);
                }
            }
        }
        _toolbar.set_enabled(toolbar::button_t::play, _tracks.size() > 0 && _current_track);
    }
}

void comp_main::prepareToPlay(int samples_per_block, double sample_rate) {
    _current_sample_rate   = sample_rate;
    _current_skip_interval = (static_cast<double>(samples_per_block) * (1000. / sample_rate)) / 100.;

    for (auto& track : _tracks) {
        track->update_info(sample_rate);
    }

    _transport_source.prepareToPlay(samples_per_block, sample_rate);
}

void comp_main::getNextAudioBlock(const AudioSourceChannelInfo& buffer) noexcept {

    buffer.clearActiveBufferRegion();
    //if (_state != state_t::playing) return;
    if (_current_track) {
        if (_fast_reverse) {
            auto new_pos = _transport_source.getCurrentPosition() - _current_skip_interval;
            if (new_pos < 0) {
                new_pos = 0; _fast_reverse = false; // todo: [2]
            }
            _transport_source.setPosition(new_pos);
        }
        if (_fast_forward) {
            auto new_pos = _transport_source.getCurrentPosition() + _current_skip_interval;
            _transport_source.setPosition(new_pos);
        }

        _transport_source.getNextAudioBlock(buffer);

        if (_fast_forward || _fast_reverse) {
            auto fade_zone = buffer.numSamples / 5;
            buffer.buffer->applyGainRamp(0, fade_zone, 0.f, 1.f);
            buffer.buffer->applyGainRamp(buffer.numSamples - fade_zone, fade_zone, 1.f, 0.f);
        }

        _current_track->get_processor().process(*buffer.buffer);
    }
    _master_track.get_processor().process(*buffer.buffer);
}

void comp_main::releaseResources()
{
    _transport_source.releaseResources();
}

void comp_main::paint(Graphics& g) {
    g.fillAll(_colors.get(color_ids::bg_dark).brighter(.2f));
}

void comp_main::resized() {
    auto area = getLocalBounds();

    // toolbar
    _toolbar.setBounds(area.removeFromTop(_toolbar.get_size().second));

    // results header button
    _button_results_header.setBounds(area.removeFromTop(20));
    _results_toolbar.setVisible(_results_expanded);
    if (_results_expanded)
        _results_toolbar.setBounds(area.removeFromTop(50));

    const auto mastertrack_size = _master_track.get_size();
    _master_track.setBounds(0, getHeight() - mastertrack_size.second, getWidth(), mastertrack_size.second);

    auto tracks_height = 5 + (_tracks.size() * 2);
    for (auto track : _tracks) {
        tracks_height += track->getHeight();
    }
    area.removeFromBottom(_master_track.getBounds().getHeight());

    auto area_tracks = area;
    area_tracks.setHeight(tracks_height);

    if (tracks_height > area.getHeight())
        area_tracks.removeFromRight(_viewport_tracks.getScrollBarThickness());

    if (_viewport_tracks_inside)
        _viewport_tracks_inside->setSize(area_tracks.getWidth(), area_tracks.getHeight());

    _viewport_tracks.setBounds(area);

    _toolbar.toFront(false);
    _toolbar.set_enabled(toolbar::button_t::rewind, _current_track && _current_track->get_marker() > 0);
}

bool comp_main::isInterestedInFileDrag(const StringArray &files) {

    for (const auto & file : files) {
        if (_filter && _filter->isFileSuitable(file))
            return true;
    }
    return false;
}

void comp_main::filesDropped(const StringArray &files, int x, int y) {
    ignoreUnused(x);
    ignoreUnused(y);
    if(!files.isEmpty()) {
        for (const auto & file : files) {
            if (_filter && _filter->isFileSuitable(file))
                track_add(file);
        }
    }
}

void comp_main::track_add(const String& file_path) {
    auto _track = new track(file_path, _transport_source);
    _track->update_info(_current_sample_rate);
    _track->set_on_mouse_event([this](track* selected_track, bool double_click)
    {
        track_activate(selected_track, double_click);
        _toolbar.set_enabled(toolbar::button_t::rewind, _current_track->get_marker() > 0);
    });
    _track->set_on_close([this](track* selected_track)
    {
            auto parent = this;
            int index   = _tracks.indexOf(selected_track);
            if (index == -1) return;

            _transport_source.removeChangeListener(selected_track);

            if (selected_track->is_active()) {
                _user_stopped = true;
                change_state(state_t::stopping);
                track* next_track = nullptr;
                if (_tracks.size() > 1) {
                    next_track = (index == _tracks.size() - 1) ? _tracks[index - 1] : _tracks[index + 1];
                }
                track_activate(next_track, true);
                if (!next_track) {
                    _transport_source.setSource(nullptr);
                }
                _user_stopped = false;
            }
            _tracks.remove(index);
            parent->resized();
    });
    _transport_source.addChangeListener(_track);
    _tracks.add(_track);
    _viewport_tracks_inside->addAndMakeVisible(_track);
    if (_tracks.size() == 1) {
        track_activate(_track, true);
    }
    resized();

    _settings.getUserSettings()->setValue("last_path", File(file_path).getParentDirectory().getFullPathName());
    _settings.saveIfNeeded();
}

void comp_main::change_state(state_t new_state)
{
    if (_state != new_state && _current_track)
    {
        _state = new_state;
        switch (_state)
        {
        case state_t::stopped:
            if (_user_stopped) {
                _ftdi.turn_off_relay();
            }
            _toolbar.set_state  (toolbar::button_t::pause, false);
            _toolbar.set_state  (toolbar::button_t::play,  false);
            _toolbar.set_enabled(toolbar::button_t::stop,  false);
            _toolbar.set_enabled(toolbar::button_t::pause, false);
            _current_track->stop(_toolbar.get_state(toolbar::button_t::restart));
            break;

        case state_t::starting:
            _transport_source.start();
            _current_track->play(_toolbar.get_state(toolbar::button_t::restart));
            break;

        case state_t::playing:
            _toolbar.set_state  (toolbar::button_t::pause, false);
            _toolbar.set_state  (toolbar::button_t::play,  true);
            _toolbar.set_enabled(toolbar::button_t::stop,  true);
            _toolbar.set_enabled(toolbar::button_t::pause, true);
            break;

        case state_t::pausing:
            _transport_source.stop();
            _current_track->pause();
            break;

        case state_t::paused:
            _toolbar.set_state(toolbar::button_t::pause, true);
            break;

        case state_t::stopping:
            if (_user_stopped) {
                _ftdi.turn_off_relay();
            }
            _transport_source.stop();
            break;
        }
    }
}

void comp_main::track_change(track* _track, bool is_next) {
    Sleep(50);

    if (_track == nullptr) return;
    int index = _tracks.indexOf(_track);
    if (index == -1) return;

    track* new_track = nullptr;

    if (is_next) {
        index = (index + 1) % _tracks.size();
    }
    else {
        if ((index - 1) < 0) {
            index = _tracks.size() - 1;
        } else {
            index--;
        }
    }
    new_track = _tracks[index];
    if (new_track) {
        auto current_state = _state;
        track_activate(new_track, true);
        if (current_state == state_t::playing && is_next) { // todo: [1]
            change_state(state_t::starting);
        }
    }
}

void comp_main::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == &_transport_source)
    {
        if (_transport_source.isPlaying()) {
            change_state(state_t::playing);
        }
        else if ((_state == state_t::stopping) || (_state == state_t::playing)) {
            change_state(state_t::stopped);
            if (!_user_stopped && _current_track)
            {
                int index = _tracks.indexOf(_current_track);
                if (index == -1) return;
                if (_tracks.size() - 1 > index) {
                    track_activate(_tracks[index + 1], true);
                    change_state(state_t::starting);
                }
                else {
                    _ftdi.turn_off_relay();
                }
            }
            _user_stopped = false;
        }
        else if (_state == state_t::pausing) {
            change_state(state_t::paused);
        }
        if (_current_track) {
            _toolbar.set_enabled(toolbar::button_t::rewind, _current_track->get_marker() > 0);
        }
    }
}

void comp_main::trial_cycle(bit_t button_bt) {
    trial_save();

    auto blind = _toolbar.get_state(toolbar::button_t::blind);
    if (_state != state_t::playing)
    {
        _ftdi.toggle_relay(blind, button_bt);
        Sleep(300);
        change_state(state_t::starting);
        return;
    }
    bit_t relay { unknown };
    std::bitset<8> relay_bt(_ftdi.get_relay());

    if (relay_bt.test(relay_a)) relay = relay_a;
    if (relay_bt.test(relay_b)) relay = relay_b;

    _trials.push_back({ button_bt, relay, blind });
    size_t count_all = 0, count_correct = 0;
    for (const auto& trial : _trials)
    {
        if (trial.button != btn_hz && trial.blind) {
            {
                if (trial.relay != unknown && (trial.button == btn_a || trial.button == btn_b)) {
                    if ((trial.button == btn_a && trial.relay == relay_a) ||
                        (trial.button == btn_b && trial.relay == relay_b))
                    {
                        count_correct++;
                    }
                }
                else
                    return;
            }
            count_all++;
        }
    }
    auto pval = abs(((count_all / 2.) - count_correct) / sqrt(count_all / 4.));
    auto display = String(count_correct) + " / " + String(count_all);
    if (std::isfinite(pval)) {
        display += ", " + String(pval, 2);
    }
    _results_toolbar.set_result(display);

    if (_toolbar.get_state(toolbar::button_t::restart))
    {
        if (_state == state_t::paused)
            change_state(state_t::stopped);
        else {
            _user_stopped = true;
            change_state(state_t::stopping);
        }
    } else {
        _ftdi.turn_off_relay();
        change_state(state_t::pausing);
    }
    Sleep(150);
    _ftdi.toggle_relay(blind, button_bt);
    Sleep(50 + static_cast<DWORD>(_ftdi.rand_range(200)));
    change_state(state_t::starting);
    _user_stopped = false;
}

}
