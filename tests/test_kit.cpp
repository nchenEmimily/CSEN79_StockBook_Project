// Tests for the kit helpers. They also show the doctest style to use for your own structures:
// one TEST_CASE per behaviour, SUBCASEs for edge cases, CHECK for each expectation.
#include <cstdio>
#include <fstream>
#include <string>

#include "doctest/doctest.h"
#include "kit/bench.h"
#include "kit/counters.h"

TEST_CASE("counters count, combine, and reset") {
    kit::Counters c;
    KIT_COUNT(c, comparisons);
    KIT_COUNT_N(c, visits, 3);
    KIT_COUNT_EXTRA(c, "resizes");
#if KIT_COUNTERS_ENABLED
    CHECK(c.comparisons == 1);
    CHECK(c.visits == 3);
    CHECK(c.extra["resizes"] == 1);

    kit::Counters total;
    total += c;
    total += c;
    CHECK(total.visits == 6);
    CHECK(total.extra["resizes"] == 2);
#endif
    c.reset();
    CHECK(c.comparisons == 0);
    CHECK(c.extra.empty());
}

TEST_CASE("median_ns runs setup, body, and after the right number of times") {
    int setups = 0, bodies = 0, afters = 0;
    double ns = kit::median_ns(
        5, [&] { ++setups; }, [&] { ++bodies; }, [&] { ++afters; });
    CHECK(setups == 6);  // one warm-up plus five timed runs
    CHECK(bodies == 6);
    CHECK(afters == 5);
    CHECK(ns >= 0.0);
    CHECK_THROWS(kit::median_ns(0, [] {}, [] {}));
}

TEST_CASE("CsvWriter writes the header once and one line per row") {
    const std::string path = "kit_test_results.csv";
    std::remove(path.c_str());
    {
        kit::CsvWriter csv(path);
        kit::Counters c;
        c.comparisons = 10;
        c.extra["swaps"] = 2;
        csv.row({"team0", "orderbook", "W1", "main", 100, "replay", 5, 1000.0, c});
    }
    {
        kit::CsvWriter csv(path);  // reopening appends without a second header
        csv.row({"team0", "orderbook", "W1", "alt", 100, "replay", 5, 2000.0, kit::Counters{}});
    }
    std::ifstream in(path);
    std::string header, row1, row2, extra;
    std::getline(in, header);
    std::getline(in, row1);
    std::getline(in, row2);
    CHECK(header.rfind("team,project,workload,impl,n,", 0) == 0);
    CHECK(row1.rfind("team0,orderbook,W1,main,100,replay,5,1000,10,10,", 0) == 0);
    CHECK(row1.find("swaps=2") != std::string::npos);
    CHECK(row2.rfind("team0,orderbook,W1,alt,", 0) == 0);
    CHECK_FALSE(std::getline(in, extra));
    in.close();
    std::remove(path.c_str());
}

TEST_CASE("read_lines keeps every line, including empty ones") {
    const std::string path = "kit_test_lines.txt";
    {
        std::ofstream out(path);
        out << "BUY u1 1 10 5.00\n\nBOOK\n";
    }
    auto lines = kit::read_lines(path);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "BUY u1 1 10 5.00");
    CHECK(lines[1].empty());
    CHECK(lines[2] == "BOOK");
    std::remove(path.c_str());
    CHECK_THROWS(kit::read_lines("no_such_file.txt"));
}
