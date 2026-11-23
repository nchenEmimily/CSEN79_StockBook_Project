// bench.h - timing and CSV output, so every team reports results the same way.
//
// Typical use:
//
//     auto lines = kit::read_lines("w1_100000.txt");   // read input BEFORE timing
//     kit::Counters total;
//     double ns = kit::median_ns(5,
//         [&] { /* setup, not timed: build a fresh app */ },
//         [&] { /* timed: replay every line */ },
//         [&] { /* after each run, not timed: collect counters into total */ });
//     kit::CsvWriter csv("results.csv");
//     csv.row({"team7", "orderbook", "W1", "main", lines.size(), "replay", 5, ns, total});
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "kit/counters.h"

#ifndef KIT_BUILD_FLAGS
#define KIT_BUILD_FLAGS "unknown"
#endif

namespace kit {

class Timer {
public:
    Timer() : start_(Clock::now()) {}
    void restart() { start_ = Clock::now(); }
    double elapsed_ns() const {
        return std::chrono::duration<double, std::nano>(Clock::now() - start_).count();
    }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point start_;
};

// Runs setup (untimed), body (timed), after (untimed) `runs` times and returns the
// median time of body in nanoseconds. One extra warm-up run is done first and discarded.
inline double median_ns(int runs, const std::function<void()>& setup,
                        const std::function<void()>& body,
                        const std::function<void()>& after = [] {}) {
    if (runs < 1) throw std::invalid_argument("runs must be >= 1");
    setup();
    body();  // warm-up
    std::vector<double> times;
    for (int i = 0; i < runs; ++i) {
        setup();
        Timer t;
        body();
        times.push_back(t.elapsed_ns());
        after();
    }
    std::sort(times.begin(), times.end());
    std::size_t mid = times.size() / 2;
    return times.size() % 2 ? times[mid] : (times[mid - 1] + times[mid]) / 2.0;
}

// Reads a whole text file into memory, one string per line.
inline std::vector<std::string> read_lines(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::vector<std::string> lines;
    for (std::string line; std::getline(in, line);) lines.push_back(line);
    return lines;
}

struct BenchRow {
    std::string team;       // team name
    std::string project;    // "orderbook" or "alerts"
    std::string workload;   // "W1", "W2", "W3", or your own
    std::string impl;       // which implementation ran, e.g. the value of your swap flag
    std::uint64_t n;        // input size
    std::string operation;  // what was timed, e.g. "replay", "cancel", "history"
    int runs;               // number of timed runs
    double median_ns;       // median total time of one run
    Counters counters;      // counters for ONE run (divide totals by runs before passing)
};

// Appends rows to a CSV file, writing the header when the file is new or empty.
class CsvWriter {
public:
    explicit CsvWriter(const std::string& path) {
        bool fresh = true;
        {
            std::ifstream probe(path);
            fresh = !probe || probe.peek() == std::ifstream::traits_type::eof();
        }
        out_.open(path, std::ios::app);
        if (!out_) throw std::runtime_error("cannot write " + path);
        if (fresh)
            out_ << "team,project,workload,impl,n,operation,runs,median_ns,ns_per_op,"
                    "comparisons,visits,allocations,moves,extra,build_flags\n";
    }

    void row(const BenchRow& r) {
        double per_op = r.n ? r.median_ns / static_cast<double>(r.n) : 0.0;
        std::string extra;
        for (const auto& [k, v] : r.counters.extra) {
            if (!extra.empty()) extra += ';';
            extra += k + "=" + std::to_string(v);
        }
        out_ << r.team << ',' << r.project << ',' << r.workload << ',' << r.impl << ',' << r.n
             << ',' << r.operation << ',' << r.runs << ',' << r.median_ns << ',' << per_op << ','
             << r.counters.comparisons << ',' << r.counters.visits << ','
             << r.counters.allocations << ',' << r.counters.moves << ',' << extra << ','
             << KIT_BUILD_FLAGS << '\n';
        out_.flush();
    }

private:
    std::ofstream out_;
};

}  // namespace kit
