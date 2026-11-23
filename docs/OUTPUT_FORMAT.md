# Command and output format

Your app reads commands from standard input, one per line, and writes results to standard
output. Following this format exactly lets you check your app against the sample scripts
with `diff`, and lets the instructor run hidden test scripts the same way.

## Rules for both projects

- Blank lines and lines starting with `#` are ignored and do not count as commands.
- Words are separated by spaces. Commands and keywords are uppercase (`BUY`, `SUB`); keywords
  inside a command such as `below`, `at`, and `every` are lowercase.
- Prices have at most two decimal places (`50`, `50.1`, and `50.10` are all valid) and are
  always printed with exactly two (`50.10`). Store prices as whole cents to avoid
  floating-point rounding errors.
- A command the app does not know prints `ERROR unknown command`. A known command with
  missing or invalid arguments prints `ERROR bad command`. Then the app moves on.
- Every line `STATS` prints must start with `STATS `. What follows is up to you. STATS
  lines are left out when outputs are compared.
- `--quiet` on the command line suppresses all output except `STATS`. Use it for
  benchmarks, since timing the printing of millions of lines measures the terminal, not
  your data structures.

To check your app against a sample:

```
./app < samples/p1_demo.txt | grep -v '^STATS' | diff - samples/p1_demo.expected.txt
```

No output from `diff` means a match.

## Project 1: order book

Each command is numbered from 1 in the order it is read, skipping blank and comment lines.
`HISTORY` uses these numbers.

| Command | Output |
| --- | --- |
| `BUY <user> <id> <qty> <price>` | One `TRADE` line per fill, then `REST <id> <qty> <price>` if anything is left over |
| `SELL <user> <id> <qty> <price>` | Same as BUY |
| `MARKET BUY <user> <id> <qty>` | One `TRADE` line per fill, then `UNFILLED <id> <qty>` if the book ran out |
| `MARKET SELL <user> <id> <qty>` | Same as MARKET BUY |
| `CANCEL <id>` | `CANCELLED <id> <remaining qty>`, or `ERROR unknown order <id>` if the order is not resting |
| `BOOK [depth]` | Up to `depth` lines `ASK <price> <total qty> <order count>`, best (lowest) first, then up to `depth` lines `BID ...`, best (highest) first, then `END`. Default depth is 5 |
| `HISTORY <user> [n]` | That user's events, oldest first, then `END`. With n, only the last n events |
| `STATS` | Your own counters, each line starting with `STATS ` |

Matching rules:

- `<id>` is a positive whole number, unique across all orders, including market orders and
  cancelled ones. Reusing one prints `ERROR duplicate id <id>` and the order is ignored.
- `<qty>` is a positive whole number.
- A buy matches the lowest ask first; a limit buy only matches asks at or below its price.
  A sell matches the highest bid first; a limit sell only matches bids at or above its price.
- At the same price, the order that rested first fills first.
- Every trade happens at the resting order's price.
- `TRADE <buy id> <sell id> <qty> <price>` lists the buy order's id first, whichever side
  arrived last.
- The unfilled part of a market order is discarded, never rested.

`HISTORY` event lines, where `#<n>` is the number of the command that caused the event:

```
#<n> PLACE <id> BUY|SELL <qty> <price>     (price is MKT for a market order)
#<n> FILL <id> <qty> <price>               (one per fill, for both the buyer and the seller)
#<n> CANCEL <id> <qty>
#<n> UNFILLED <id> <qty>
```

A user with no events prints only `END`.
