// gen_orderbook - writes a Project 1 command script to standard output.
//
//   gen_orderbook --workload W1|W2|W3 --n 100000 [--seed 42] [--users 1000] [--no-queries]
//
// W1 realistic:     prices cluster near a mid price that wanders; 30% cancels.
// W2 trending:      bids follow a mid price that only rises, so new price levels keep
//                   arriving above all others; asks wait far above; 10% cancels.
// W3 cancel-heavy:  90% cancels.
// About 3% of commands are market orders. Unless --no-queries is given, about 1% are
// BOOK 10 or HISTORY <user> queries. Some cancels target orders that have already filled;
// the app should answer those with "ERROR unknown order <id>", which is expected.
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "rng.h"

int main(int argc, char** argv) {
    std::string workload = "W1";
    long long n = 100000, users = 1000;
    unsigned long long seed = 42;
    bool queries = true;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto val = [&]() -> std::string {
            if (i + 1 >= argc) { std::cerr << "missing value for " << a << '\n'; std::exit(2); }
            return argv[++i];
        };
        if (a == "--workload") workload = val();
        else if (a == "--n") n = std::stoll(val());
        else if (a == "--seed") seed = std::stoull(val());
        else if (a == "--users") users = std::stoll(val());
        else if (a == "--no-queries") queries = false;
        else { std::cerr << "unknown option " << a << '\n'; return 2; }
    }
    double cancel_rate;
    if (workload == "W1") cancel_rate = 0.30;
    else if (workload == "W2") cancel_rate = 0.10;
    else if (workload == "W3") cancel_rate = 0.90;
    else { std::cerr << "workload must be W1, W2, or W3\n"; return 2; }

    kit::Rng rng(seed);
    long long mid = 10000;  // cents
    long long next_id = 1;
    long long w2_ceiling = mid + n / 10 + 1000;
    std::vector<long long> cancellable;  // ids of limit orders not yet cancelled
    std::string out;
    out.reserve(static_cast<size_t>(n) * 28);

    auto user = [&] { return "u" + std::to_string(rng.between(0, users - 1)); };

    for (long long i = 0; i < n; ++i) {
        if (workload == "W2") {
            if (i % 10 == 0) mid += 1;
        } else if (rng.chance(0.05)) {
            mid += rng.chance(0.5) ? 1 : -1;
            if (mid < 1000) mid = 1000;
        }

        double r = rng.uniform();
        if (queries && r < 0.01) {
            if (rng.chance(0.5)) out += "BOOK 10\n";
            else out += "HISTORY " + user() + "\n";
            continue;
        }
        if (rng.chance(cancel_rate) && !cancellable.empty()) {
            size_t k = static_cast<size_t>(rng.between(0, (long long)cancellable.size() - 1));
            out += "CANCEL " + std::to_string(cancellable[k]) + "\n";
            cancellable[k] = cancellable.back();
            cancellable.pop_back();
            continue;
        }
        bool buy = rng.chance(0.5);
        long long qty = rng.between(1, 10) * 10;
        long long id = next_id++;
        if (rng.chance(0.03)) {
            out += std::string("MARKET ") + (buy ? "BUY " : "SELL ") + user() + " " +
                   std::to_string(id) + " " + std::to_string(qty) + "\n";
            continue;
        }
        long long price;
        if (workload == "W2" && !buy) {
            // Sellers wait well above the highest price the trend will reach.
            price = w2_ceiling + static_cast<long long>(rng.normal(0.0, 50.0));
        } else {
            // Buyers bid a little under the mid, sellers ask a little over; some cross.
            // In W2 bids hug the rising mid, so new price levels keep appearing at the top.
            double offset = workload == "W2" ? -rng.between(0, 5) : rng.normal(buy ? -25.0 : 25.0, 15.0);
            price = mid + static_cast<long long>(offset >= 0 ? offset + 0.5 : offset - 0.5);
        }
        if (price < 1) price = 1;
        out += std::string(buy ? "BUY " : "SELL ") + user() + " " + std::to_string(id) + " " +
               std::to_string(qty) + " " + kit::cents(price) + "\n";
        // Only orders that probably rested on arrival are cancel targets. Some fill later anyway.
        if (buy ? price < mid : price > mid) cancellable.push_back(id);
    }
    std::fwrite(out.data(), 1, out.size(), stdout);
}
