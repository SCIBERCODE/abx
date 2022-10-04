#pragma once
#pragma comment(lib, "legacy_stdio_definitions.lib")
#if _M_X64
#pragma comment(lib, "..\\..\\Libs\\d2xx\\amd64\\ftd2xx.lib")
#else
#pragma comment(lib, "..\\..\\Libs\\d2xx\\i386\\ftd2xx.lib")
#endif
#define FTD2XX_STATIC

#include <d2xx\ftd2xx.h>
#include <bitset>

#include "rand.h"

namespace abx {

constexpr size_t   _A = 1;
constexpr size_t   _B = 2;
constexpr size_t  _HZ = 3;
constexpr size_t _REV = 4;
constexpr size_t _FWD = 5;

class ftdi : public Thread,
             public Timer
{
private:
    enum class bit_t : size_t {
        relay_a = 0,
        btn_b   = 1,
        unknown = 2,
        btn_rev = 3,
        relay_b = 4,
        btn_hz  = 5,
        btn_a   = 6,
        btn_fwd = 7
    };

    constexpr auto to_pos(bit_t bit) noexcept
    {
        return static_cast<std::underlying_type_t<bit_t>>(bit);
    }

    const std::map<bit_t, size_t> _bit_to_buttons
    {
        { bit_t::btn_a,     _A },
        { bit_t::btn_b,     _B },
        { bit_t::btn_hz,   _HZ },
        { bit_t::btn_rev, _REV },
        { bit_t::btn_fwd, _FWD }
    };

public:

    auto get_buttons_mask() {
        return _buttons_mask.to_ulong();
    }

    void toggle_relay(bool blind, size_t button) {
        if (!button) {
            DBG("error #2");
            return;
        }

        auto rand_relay = [this] {
            return rand_range(2) == 0 ? to_pos(bit_t::relay_a) : to_pos(bit_t::relay_b);
        };

        std::bitset<8> relay_bt;
        if (blind) {
            relay_bt.set(rand_relay());
        }
        else {
            switch (button) {
            case _A:  relay_bt.set(to_pos(bit_t::relay_a)); break;
            case _B:  relay_bt.set(to_pos(bit_t::relay_b)); break;
            case _HZ: relay_bt.set(rand_relay()); break;
            }
        }
        _relay = relay_bt.to_ulong();
    }

    void turn_off_relay() {
        _relay = 0;
    }

    size_t get_relay() {
        std::bitset<8> relay_bt(static_cast<uint64_t>(_relay.get()));
        if (relay_bt.test(to_pos(bit_t::relay_a))) return _A;
        if (relay_bt.test(to_pos(bit_t::relay_b))) return _B;
        return 0;
    };

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    ftdi() : Thread(""),
             _xs1024(abx::init_rnd())
    {
        for (const auto [bit, button] : _bit_to_buttons) {
            _buttons_mask.set(to_pos(bit));
        }
        abx::init_rnd();
        startTimer(333); // check ftdi // todo: [10]
    }

    void run() override {
        DWORD     written {};
        FT_STATUS status  {};

        _last_out = 0;
        _last_in  = 0;
        _waiting  = true;
        _timer    = GetTickCount64();

        while (!threadShouldExit() && _handle)
        {
            if (_relay.get() != _last_out) {
                MessageManager::callAsync(
                    [=]() {
                        _callback_on_relay_change(_relay.get());
                    }
                );
            }
            _current_out = 0;
            if (_waiting) {
                if ((GetTickCount64() - _timer) >= 100) {
                    _waiting     = false;
                    _last_out    = _relay.get();
                    _current_out = _relay.get();
                }
            }
            else {
                if (_relay.get() == _last_out) {
                    _current_out = _relay.get();
                }
                else {
                    _waiting = true;
                    _timer   = ULLONG_MAX;
                }
            }
            //--------------------------------------------------------------
            std::bitset<8> relay_check { _current_out };
            if (relay_check.count() > 1) {
                _ft_tx = 0;
                FT_Write(_handle, &_ft_tx, 1, &written);
                //break;
            }
            //--------------------------------------------------------------
            _ft_tx = _current_out;
            status = FT_Write(_handle, &_ft_tx, 1, &written);
            status = FT_Read (_handle, &_ft_rx, 1, &written);
            if (status == FT_OK)
            {
                if ((_ft_rx & _buttons_mask.to_ulong()) != _last_in)
                {
                    _last_in = _ft_rx & _buttons_mask.to_ulong();
                    //if (_last_in != _buttons_mask.to_ulong())
                    {
                        MessageManager::callAsync(
                            [=]() {
                                size_t button_pressed {};
                                std::bitset<8> in(_last_in);
                                in.flip();
                                for (const auto [bit, button] : _bit_to_buttons)
                                {
                                    if (in.test(to_pos(bit)))
                                    {
                                        button_pressed = button;
                                        break;
                                    }
                                }
                                _callback_on_button_press(button_pressed);
                            }
                        );
                    }
                }
            }

            if (_timer == ULLONG_MAX) {
                _timer = GetTickCount64();
            }
        }
    }

    ~ftdi() {
        DWORD written {};
        if (_handle != nullptr) {
            _ft_tx = 0;
            FT_Write(_handle, &_ft_tx, 1, &written);
            FT_Close(_handle);
        }
    }

    void set_on_button_press_callback(const std::function<void(size_t)>& callback) {
        _callback_on_button_press = callback;
    }

    void set_on_relay_change_callback(const std::function<void(size_t)>& callback) {
        _callback_on_relay_change = callback;
    }

    void set_on_status_change_callback(const std::function<void(String)>& callback) {
        _callback_on_status_change = callback;
    }

    void timerCallback() override { // bug: [1]
        FT_STATUS st;
        DWORD     num;

        st = FT_ListDevices(&num, nullptr, FT_LIST_NUMBER_ONLY);
        if (st != FT_OK) return;

        if (!num && _handle) {
            stopThread(500);
            FT_Close(&_handle);
            _handle = nullptr;
            MessageManager::callAsync(
                [=]() {
                    _callback_on_status_change({});
                });
            return;
        }
        if (num && _handle == nullptr)
        {
            st = FT_Open(0, &_handle);
            auto handle = _handle;
            if (st != FT_OK || handle == nullptr) return;

            std::bitset<8> relays_mask;
            relays_mask.set(to_pos(bit_t::relay_a));
            relays_mask.set(to_pos(bit_t::relay_b));
            if (FT_OK != FT_SetBitMode     (handle, 0, 0) ||
                FT_OK != FT_SetBitMode     (handle, static_cast<UCHAR>(relays_mask.to_ulong()), FT_BITMODE_SYNC_BITBANG) ||
                FT_OK != FT_SetTimeouts    (handle, 100, 100) ||
                FT_OK != FT_SetBaudRate    (handle, FT_BAUD_14400) ||
                FT_OK != FT_SetLatencyTimer(handle, 1)
                )
            {
                FT_Close(&_handle);
                _handle = nullptr;
                return;
            }
            char name[MAX_PATH] { };
            FT_GetDeviceInfo(_handle, nullptr, nullptr, nullptr, &name[0], nullptr);

            MessageManager::callAsync(
                [=]() {
                    _callback_on_status_change("Remote: " + (strlen(name) ? String(name) : "FTDI device"));
                });
            startThread();
        }
    }

    size_t rand_range(size_t range) {
        auto xs = _xs1024.next();
        return xs % range;
    }

private:
    std::bitset<8>    _buttons_mask;
    FT_HANDLE         _handle      {};
    size_t            _ft_rx       {},
                      _ft_tx       {},
                      _last_in     {},
                      _last_out    {},
                      _current_out {};
    Atomic<size_t>    _relay       {};
    bool              _waiting     { true };
    uint64_t          _timer       {};
    abx::xorshift1024 _xs1024;

    std::function<void(size_t)> _callback_on_button_press;
    std::function<void(size_t)> _callback_on_relay_change;
    std::function<void(String)> _callback_on_status_change;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ftdi);
};

}