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

namespace abx {

enum bit_t : uint8_t {
    relay_a = 0,
    btn_b   = 1,
    unknown = 2,
    btn_rev = 3,
    relay_b = 4,
    btn_hz  = 5,
    btn_a   = 6,
    btn_fwd = 7
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class ftdi : public Thread,
             public Timer
{
public:

    auto get_buttons_mask() {
        return _buttons_mask.to_ulong();
    }

    void toggle_relay(bool blind, bit_t button_bt = unknown) {
        std::bitset<8> relay_bt;
        if (blind) {
            relay_bt.set(rand_range(2) == 0 ? relay_a : relay_b);
        }
        else {
            if (button_bt == btn_a)  relay_bt.set(relay_a);
            if (button_bt == btn_hz) relay_bt.set(rand_range(2) == 0 ? relay_a : relay_b);
            if (button_bt == btn_b)  relay_bt.set(relay_b);
        }
        _relay = static_cast<uint8_t>(relay_bt.to_ulong());
    }

    void turn_off_relay() { _relay = 0; }

    uint8_t get_relay() { return _relay.get(); };

    ftdi() : Thread(""), _xs1024(init_rnd())
    {
        _buttons_mask.set(btn_rev);
        _buttons_mask.set(btn_a);
        _buttons_mask.set(btn_hz);
        _buttons_mask.set(btn_b);
        _buttons_mask.set(btn_fwd);

        init_rnd();
        startTimer(333); // check ftdi // todo: [10]
    }

    void run() override {
        DWORD     written { };
        FT_STATUS status  { };

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
                                _callback_on_button_press(_last_in);
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
        DWORD written { };
        if (_handle != nullptr) {
            _ft_tx = 0;
            FT_Write(_handle, &_ft_tx, 1, &written);
            FT_Close(_handle);
        }
    }

    void set_on_button_press_callback(const std::function<void(uint8_t buttons_pressed)>& callback) {
        _callback_on_button_press = callback;
    }

    void set_on_relay_change_callback(const std::function<void(uint8_t new_relay)>& callback) {
        _callback_on_relay_change = callback;
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
            //_toolbar.disconnect(); // todo: [11]
            return;
        }
        if (num && _handle == nullptr)
        {
            st = FT_Open(0, &_handle);
            auto handle = _handle;
            if (st != FT_OK || handle == nullptr) return;

            std::bitset<8> relays_mask;
            relays_mask.set(relay_a);
            relays_mask.set(relay_b);
            if (FT_OK != FT_SetBitMode     (handle, 0, 0) ||
                FT_OK != FT_SetBitMode     (handle, (UCHAR)relays_mask.to_ulong(), FT_BITMODE_SYNC_BITBANG) ||
                FT_OK != FT_SetTimeouts    (handle, 100, 100) ||
                FT_OK != FT_SetBaudRate    (handle, FT_BAUD_14400) ||
                FT_OK != FT_SetLatencyTimer(handle, 1)
                )
            {
                FT_Close(&_handle);
                _handle = nullptr;
                return;
            }
            //_toolbar.connect();
            startThread();
        }
    }

    /*
    //////////////////////////////////////////////////////////////////////////////////////////
    */
    class xorshift1024 {
    private:
        std::array<uint64_t, 16> s;
        int p = 0;
    public:
        explicit xorshift1024(std::array<uint64_t, 16>& seed) : s(seed), p(0) {}
        uint64_t next(void) {
            const uint64_t s0 = s[p];
            uint64_t s1 = s[p = (p + 1) & 15];
            s1 ^= s1 << 31;
            s[p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);
            return s[p] * static_cast<uint64_t>(1181783497276652981);
        }
    };

    xorshift1024 init_rnd() {
        class splitmix64 {
        private:
            uint64_t x;
        public:
            explicit splitmix64(uint64_t seed) : x(seed) { }
            uint64_t next() {
                uint64_t z =    (x += static_cast<uint64_t>(0x9E3779B97F4A7C15));
                z = (z ^ (z >> 30)) * static_cast<uint64_t>(0xBF58476D1CE4E5B9);
                z = (z ^ (z >> 27)) * static_cast<uint64_t>(0x94D049BB133111EB);
                return z ^ (z >> 31);
            }
        };
        splitmix64 sm(std::chrono::system_clock::now().time_since_epoch().count());
        std::array<uint64_t, 16> seed;
        std::generate(seed.begin(), seed.end(), [sm]() mutable { return sm.next(); });
        return xorshift1024(seed);
    }

    size_t rand_range(size_t range) {
        auto xs = _xs1024.next();
        return xs % range;
    }

private:
    FT_HANDLE       _handle   { };
    std::bitset<8>  _buttons_mask;
    uint8_t         _ft_rx,
                    _ft_tx,
                    _last_in  { },
                    _last_out { },
                    _current_out;
    Atomic<uint8_t> _relay    { };
    bool            _waiting  { true };
    uint64_t        _timer;
    xorshift1024    _xs1024;

    std::function<void(uint8_t buttons_pressed)> _callback_on_button_press;
    std::function<void(uint8_t new_relay)>       _callback_on_relay_change;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ftdi);
};

}