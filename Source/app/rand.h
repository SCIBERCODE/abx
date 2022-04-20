#pragma once

namespace abx {

/*
//////////////////////////////////////////////////////////////////////////////////////////
*/
class xorshift1024 {
private:
    std::array<uint64_t, 16> s;
    int p = 0;
public:
    explicit xorshift1024(std::array<uint64_t, 16>& seed) : s(seed), p(0) { }
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
            uint64_t z = (x += static_cast<uint64_t>(0x9E3779B97F4A7C15));
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

}