// counters.h - operation counters for hand-built data structures.
//
// Give each structure a kit::Counters member and bump it where the work happens:
//
//     KIT_COUNT(stats_, comparisons);      // one comparison
//     KIT_COUNT_N(stats_, visits, 3);      // three elements visited
//
// Build with -DCOUNTERS=OFF to compile every KIT_COUNT away when you want
// timings with zero counter overhead.
#pragma once

#include <cstdint>
#include <map>
#include <ostream>
#include <string>

#ifndef KIT_COUNTERS_ENABLED
#define KIT_COUNTERS_ENABLED 1
#endif

namespace kit {

struct Counters {
    std::uint64_t comparisons = 0;  // key comparisons
    std::uint64_t visits = 0;       // elements or nodes touched
    std::uint64_t allocations = 0;  // calls to new / allocations of storage
    std::uint64_t moves = 0;        // elements moved, copied, or relinked
    // Anything specific to your structure, by name: extra["resizes"], extra["swaps"], ...
    std::map<std::string, std::uint64_t> extra;

    void reset() { *this = Counters{}; }

    Counters& operator+=(const Counters& o) {
        comparisons += o.comparisons;
        visits += o.visits;
        allocations += o.allocations;
        moves += o.moves;
        for (const auto& [k, v] : o.extra) extra[k] += v;
        return *this;
    }

    // One line, e.g. "orders: comparisons=120 visits=40 allocations=8 moves=0 resizes=2"
    void print(std::ostream& os, const std::string& label) const {
        os << label << ": comparisons=" << comparisons << " visits=" << visits
           << " allocations=" << allocations << " moves=" << moves;
        for (const auto& [k, v] : extra) os << ' ' << k << '=' << v;
        os << '\n';
    }
};

}  // namespace kit

#if KIT_COUNTERS_ENABLED
#define KIT_COUNT(c, field) (++(c).field)
#define KIT_COUNT_N(c, field, n) ((c).field += (n))
#define KIT_COUNT_EXTRA(c, name) (++(c).extra[name])
#else
#define KIT_COUNT(c, field) ((void)0)
#define KIT_COUNT_N(c, field, n) ((void)0)
#define KIT_COUNT_EXTRA(c, name) ((void)0)
#endif
