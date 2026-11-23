# Class Project Walkthrough

## 1. Overview

Your team is building one command-line program, either the **order book** (Project 1) or the **alert service** (Project 2). It reads commands from a text file and prints results. To get full credit you also need to **prove** your data structure choices with measurements and short written reports.

You will produce four kinds of things:

| Kind | Examples
|---|---
| **Code** | Your data structures, the app, `main.cpp`
| **Tests** | Unit tests and small command scripts
| **Evidence** | Benchmark results (CSV), plots, sanitizer logs
| **Writing** | DDRs, README, contribution statements, demo script

The starter kit already handles building, testing, timing, and generating test data.

Note that you will all be working on the same project. **Using version control, like GitHub, is *highly* recommended** but it is not required. The easiest way to work on this project is to all have a copy of the starter-kit, work on your individual files, and choose one (or all) team member(s) that will get the completed build on their system.

TAs will be testing your code on the SCU ECC. If you do not test your code on the SCU ECC, there are **no guarentees** that your code will even compile on our end.

## 2. The repository at a glance

Below is a finished repository. **[KIT]** marks files that come with the starter kit; do not edit them. **[YOU]** marks files your team creates. Where the slot says `team`, use your own team name (for example `include/team7/`).

```
project/
├── CMakeLists.txt                  [KIT]
├── README.md                       [YOU]  rewrite for your project
├── CONTRIBUTIONS.md                [YOU]  one paragraph per student
│
├── include/
│   ├── kit/                        [KIT]
│   └── team/                       [YOU]  ALL of your C++ code except main.cpp
│       ├── <structure1>.h                 one file per data structure
│       ├── <structure2>.h
│       ├── <structure3>.h
│       ├── stl_<name>.h                   STL versions, for comparison
│       ├── <app>.h                        the application logic
│       └── commands.h                     reading and dispatching commands
│
├── src/
│   └── main.cpp                    [YOU]  short: reads flags, starts the app
│
├── tests/
│   ├── test_main.cpp, test_kit.cpp [KIT]
│   ├── test_<structure1>.cpp       [YOU]  one test file per structure
│   ├── test_<structure2>.cpp
│   ├── test_<structure3>.cpp
│   ├── test_app.cpp                [YOU]  tests for the whole app
│   └── scripts/                    [YOU]  small command files + expected output
│
├── bench/                          [RECOMMENDED]
│   └── bench.cpp                   [YOU]  the benchmark program
│
├── results/                        [RECOMMENDED]
│   ├── results.csv                 [YOU]  raw benchmark numbers
│   └── plots/                      [YOU]  M2 and M3 plots (PNG or PDF)
│
├── logs/
│   ├── asan_tests.txt              [YOU]  sanitizer run of the tests
│   └── asan_app.txt                [YOU]  sanitizer run of the app
│
├── docs/
│   ├── OUTPUT_FORMAT.md            [KIT]
│   └── <any design docs>           [YOU] Any documents you created, including your DDRs, scheduling, role assignments, etc
│
├── demo/
│   └── demo.txt                    [YOU]  the command file you replay in class
│
├── samples/                        [KIT]
├── tools/                          [KIT]
└── third_party/doctest/            [KIT]
```

Your team will have at least **3 hand-built structures**, including at least one linear structure (list, queue, ring buffer, heap array, …) and at least one tree. That's why the tree shows three `<structure>` files. Add more if your design needs them. Note that, since the requirements state that 3 must be hand made, any additional structures can use STL.

## 3. Per-File Overview

### 3.1 `include/team/<structure>.h`

A hand-built data structure, written as a **class template** (for example `template <class T> class RingBuffer`), with its own nodes, links, and operations.

**Tips:**
- `#pragma once` on the first line, so the file is never included twice in one program.
- The class inside `namespace team { … }`, so your names can't clash with anyone else's.
- The operations your app needs (for example `push_back`, `pop_front`, `insert`, `erase`, `find`).
- A `kit::Counters stats_;` member, and `KIT_COUNT(...)` calls where the work happens (each comparison, each node visited, each `new`).
- `stats()` to read the counters and `reset_stats()` to zero them.
- A destructor that frees all heap memory

Each student should be tasked with writing one structure each to distribute work evenly.

### 3.2 Each data structure must have an associated STL implementation

A small wrapper class with **the same functions** as your hand-built structure, but implemented with a standard library container (`std::map`, `std::list`, `std::vector`, …).

It is recommended to **build the STL version first**, get the app working, then replace it with your own implementation.

### 3.3 `include/team/<app>.h`

The class that does the real work. For Project 1 this is the order book (matching, resting, cancelling, history). For Project 2 it's the alert service (subscriptions, publishing, prices, the clock).

Make sure to follow the rules from the handout and `docs/OUTPUT_FORMAT.md`, built out of your structures. It prints results only when `--quiet` is off, and it always prints `STATS`.

### 3.4 `include/team/commands.h` (Recommended, but can be built into `<app>.h`)

The code that reads one line at a time, splits it into words, checks the arguments, and calls the right function in your app class.

- Skip blank lines and lines starting with `#`, and don't count them as commands.
- Count all other lines from 1, including ones that turn out to be errors.
- Turn prices into whole cents (`50.1` becomes `5010`). **Never store a price as a `double`.**
- Print `ERROR unknown command` or `ERROR bad command` exactly as `docs/OUTPUT_FORMAT.md` says.

### 3.5 `src/main.cpp`

1. Read the command-line flags: `--quiet`, your swap flag (for example `--impl=main|alt`), and for Project 2, `--inbox K`.
2. Pick which structure implementation to use.
3. Call the code in `commands.h` to read standard input.

The app won't build at all until this file exists. CMake only creates `app` when there is at least one `.cpp` in `src/`.

### 3.6 `tests/test_<structure>.cpp`

The structure of a test script is close to `tests/test_kit.cpp`. You can find lots of information about CMake tests online.

**What to test**:
- An empty structure
- One element
- Duplicate values
- Removing the first, the last, and the only element
- Using the structure again after it becomes empty
- `check_invariants()` returning `true` after each change

### 3.7 `tests/test_app.cpp`

Runs your app on small scripts inside a test and checks the output. Include one test that runs **both** `--impl` choices and checks they print the same thing.

### 3.8 `bench/bench.cpp`: the benchmark program

A separate program that loads a workload file, replays it on your app several times, and writes one CSV row per measurement using the kit's `kit::median_ns` and `kit::CsvWriter` (see the kit README for an example).

Put it in its own `bench/` folder, because everything in `src/` is glued into `app`, and a second `main()` there would break the build. Add **one line** at the end of `CMakeLists.txt`:
```cmake
add_executable(bench bench/bench.cpp)
```
This is the only change you should make to `CMakeLists.txt`.

**Benchmark Requirements**:
- Built with the optimized Release build (the default).
- Load the input **before** starting the clock.
- Run with `--quiet` behaviour, with no printing in the timed part (printing can be slow).
- Report the median of 5 runs.

### 3.9 Generating Workloads
It is recommended that you automate the process of generating your workloads. This can be done using shell scripts, Python scripts, cpp scripts, etc. The minimum requirement that we need is that you either clearly state what workloads you are gathering your numbers from. Camino doesn't like when you submit gigantic text files, so it is a good idea to remove the actual test files before submitting.

Each workload is seeded, meaning that the randomly generated output is reproducable between executions given the seed is the same between. Since you are not directly submitting the files you tested on, it is important to include what seeds were used to generate your workloads.

### 3.10 `results/results.csv`

**`results.csv`:** written automatically by `kit::CsvWriter`. Commit the raw file and don't edit it by hand.

**`plots/`:** at minimum, one **M2** plot (time per operation against N, both axes on a log scale) and one **M3** plot (your structure against each alternative). Use any tool: Python, a spreadsheet, or anything else. Every plot needs a title, axis labels with units, and a legend. The rubric says plots must be "readable without explanation."

### 3.11 `logs/asan_tests.txt` and `logs/asan_app.txt`

Saved output from running under AddressSanitizer, which detects memory leaks and memory errors:
```bash
cmake -S . -B build-asan -DSANITIZE=ON
cmake --build build-asan -j
./build-asan/tests > logs/asan_tests.txt 2>&1
./build-asan/app --quiet < samples/p1_demo.txt > logs/asan_app.txt 2>&1
```
A clean log has **no** lines saying `ERROR: AddressSanitizer` or `LeakSanitizer`.

If the ASan build fails to link on the lab machines try the following alternatives in order:
1. `CXX=clang++ cmake -S . -B build-asan -DSANITIZE=ON`
1. `valgrind --leak-check=full ./build/tests > logs/valgrind_tests.txt 2>&1`

A clean log for these builds proves your project has no memory leaks

### 3.12 Schedule and role assignment

Since you will be submitting a single .zip, including a `docs` folder may be to your benefit. Using Markdown (like the `.md` file you are reading right now) will allow you to write your documents easily in your IDE

It is highly recommended that you create the following:
1. A document showing team member assignments to different roles/tasks
    - Who does what data structure
    - Who does what role (benchmarking, implementation, tests, etc.)
1. An outline of your plan over the course of the next few weeks
1. Your DDRs, in a single file or multiple files. Make sure to mention what student owns what data structure in the DDR

**Commit parts 1–4 in the first lab.** Git records the date, and that date proves your prediction came before your measurements. A wrong prediction that you explain earns full credit; a right answer with no prediction does not.

### 3.13 `demo/demo.txt`

The exact command file you replay in class.
- **Project 1:** must show a large order sweeping several price levels, then a cancel.
- **Project 2:** must show a price drop alerting exactly the right users, then a `TICK` that fires scheduled deliveries.

Keep it short enough that the output fits on screen. Use `#` comments to label each part.

## 4. Suggested order of work

1. Read the handout, `docs/OUTPUT_FORMAT.md`, and the sample scripts together
1. Fill in roles and dates and write DDR parts 1–4
1. Get a working app using **STL containers first**
1. Pass the sample: `./build/app < samples/pX_demo.txt \| grep -v '^STATS' \| diff - samples/pX_demo.expected.txt`
1. Write your own structures and their tests
1. Swap them into the app behind the same functions; add the swap flag
1. Add counters, `STATS`, script tests
1. Run the sanitizer early and fix leaks
1. Finish DDR parts 5–6, README, contributions; rehearse the demo