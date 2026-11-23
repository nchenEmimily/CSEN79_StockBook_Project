# Team project starter kit

This kit sets up the build, testing, timing, and test data, so you can spend the week on
your design. It contains no data structures and no application code. Choosing and building
those is the assignment.

## What's here

| Path | What it is |
| --- | --- |
| `CMakeLists.txt` | Builds your app, your tests, and the workload generators |
| `include/kit/counters.h` | `kit::Counters` and the `KIT_COUNT` macros for operation counts |
| `include/kit/bench.h` | `kit::median_ns` for timing, `kit::read_lines` for loading input, `kit::CsvWriter` for results |
| `tests/` | doctest runner plus tests for the kit, which show the testing style to use |
| `tools/` | `gen_orderbook` and `gen_alerts`, the seeded workload generators (W1 to W3) |
| `samples/` | A demo script and its exact expected output for each project |
| `docs/OUTPUT_FORMAT.md` | The exact command and output format your app must follow |
| `src/` | Your application goes here, starting with `src/main.cpp` |
| `third_party/doctest/` | The doctest single-header test framework (MIT license) |

Put each of your data structures in its own header under `include/`, for example
`include/team/my_structure.h`, so both the app and the tests can include it.

## Build

You need a C++17 compiler and CMake 3.16 or later.

```sh
cmake -S . -B build              # optimized Release build, counters on
cmake --build build -j
./build/tests                    # run unit tests
```

Other builds:

```sh
CXX=clang++ cmake -S . -B build-asan -DSANITIZE=ON      # AddressSanitizer + UBSan, for the leak check
cmake -S . -B build-fast -DCOUNTERS=OFF     # counters compiled away, for pure timings
```

Hand in the output of `./build-asan/tests` and a run of your app under `build-asan`
showing no errors.

## Generate workloads

```
./build/gen_orderbook --workload W1 --n 100000 --seed 42 > w1_100k.txt
./build/gen_alerts    --workload W2 --subs 100000 --events 10000 --seed 42 > w2.txt
```

The same options and seed always produce the same file on any machine. Record the seeds
you used in your report. The options and their defaults are listed at the top of each
generator's source file in `tools/`.

## Check correctness

```
./build/app < samples/p1_demo.txt | grep -v '^STATS' | diff - samples/p1_demo.expected.txt
```

No output means your app matches. The instructor will run hidden scripts the same way, so
follow `docs/OUTPUT_FORMAT.md` exactly.

## Count and time

Give each hand-built structure a `kit::Counters` member and bump it where the work happens:

```cpp
#include "kit/counters.h"

class MyStructure {
public:
    const kit::Counters& stats() const { return stats_; }
    void reset_stats() { stats_.reset(); }
    // ...
private:
    kit::Counters stats_;
};

// inside a method:
KIT_COUNT(stats_, comparisons);
KIT_COUNT_N(stats_, visits, 1);
KIT_COUNT_EXTRA(stats_, "resizes");   // anything specific to your structure
```

Time a whole replay and append one CSV row per measurement:

```cpp
#include "kit/bench.h"

auto lines = kit::read_lines("w1_100k.txt");      // load BEFORE timing
kit::Counters total;
const int runs = 5;
double ns = kit::median_ns(runs,
    [&] { /* untimed: build a fresh app in --quiet mode */ },
    [&] { /* timed: feed every line to the app */ },
    [&] { /* untimed: add the app's counters into total */ });
// total holds counters summed over the runs; store one run's worth.
kit::Counters per_run;
per_run.comparisons = total.comparisons / runs;
// ... same for the other fields you use
kit::CsvWriter csv("results.csv");
csv.row({"team7", "orderbook", "W1", "main", lines.size(), "replay", runs, ns, per_run});
```

Every row records the build flags, so a debug-build timing is easy to spot. Plot from
`results.csv` with any tool you like.
