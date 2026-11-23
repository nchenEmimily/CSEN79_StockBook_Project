// rng.h - a small random number generator that gives the same sequence on every
// compiler and platform (std:: distributions do not), so a seed always means the same workload.
#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace kit {

class Rng {
public:
    explicit Rng(std::uint64_t seed) : s_(seed ? seed : 0x9E3779B97F4A7C15ULL) {}

    std::uint64_t next() {  // splitmix64
        std::uint64_t z = (s_ += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    double uniform() { return (next() >> 11) * (1.0 / 9007199254740992.0); }  // [0, 1)
    long long between(long long lo, long long hi) {  // [lo, hi]
        return lo + static_cast<long long>(next() % static_cast<std::uint64_t>(hi - lo + 1));
    }
    bool chance(double p) { return uniform() < p; }
    double normal(double mean, double sd) {  // Box-Muller
        double u1 = uniform(), u2 = uniform();
        if (u1 < 1e-300) u1 = 1e-300;
        return mean + sd * std::sqrt(-2.0 * std::log(u1)) * std::cos(6.283185307179586 * u2);
    }

private:
    std::uint64_t s_;
};

// Zipf sampler over ranks 0..n-1 (rank 0 most popular), exponent s.
class Zipf {
public:
    Zipf(std::size_t n, double s) : cdf_(n) {
        double sum = 0;
        for (std::size_t i = 0; i < n; ++i) cdf_[i] = (sum += 1.0 / std::pow(double(i + 1), s));
        for (auto& c : cdf_) c /= sum;
    }
    std::size_t sample(Rng& r) const {
        double u = r.uniform();
        std::size_t lo = 0, hi = cdf_.size() - 1;
        while (lo < hi) {
            std::size_t mid = (lo + hi) / 2;
            if (cdf_[mid] < u) lo = mid + 1; else hi = mid;
        }
        return lo;
    }

private:
    std::vector<double> cdf_;
};

inline std::string cents(long long c) {
    std::string frac = std::to_string(c % 100);
    return std::to_string(c / 100) + "." + (frac.size() < 2 ? "0" : "") + frac;
}

}  // namespace kit
