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

## Project 2: alert service

The simulated clock starts at day 0, 00:00, and only `TICK` moves it. Times print as
`D<day> <HH:MM>`, for example `D0 08:00` or `D3 17:30`.

Topics are paths of words separated by `/`, such as `news/sports/soccer`. A subscription
to a topic also covers every topic below it: `news` covers `news/sports/soccer`.

| Command | Output |
| --- | --- |
| `SUB <user> <topic>` | `OK SUB <user> <topic>`. Alerts for this topic are delivered as soon as they are published |
| `SUB <user> <topic> at <HH:MM>` | Same output. Publications are held and delivered together every day at that time, starting with the first such time after now |
| `SUB <user> <topic> every <hours>` | Same output. Publications are held and delivered together every `hours` hours, the first time `hours` hours after now |
| `SUB <user> <item> below <price>` | Same output. The user is alerted when the item's price drops below `price`; see PRICE |
| `UNSUB <user> <topic>` | `OK UNSUB <user> <topic>`, or `ERROR not subscribed <user> <topic>`. Held publications for it are discarded |
| `PUB <topic> <message>` | One `ALERT` line per user delivered right now. The message is the rest of the line |
| `PRICE <item> <price>` | One `ALERT` line per user whose threshold this price change crossed |
| `TICK <minutes>` | One `ALERT` line per held publication delivered during that time, then `TIME <new time>` |
| `INBOX <user> [n]` | The user's last n alerts (default K), oldest first, as `ALERT` lines, then `END` |
| `HISTORY <user> [n]` | That user's events, oldest first, then `END`. With n, only the last n events |
| `STATS` | Your own counters, each line starting with `STATS ` |

Alert lines:

```
ALERT <time> <user> <topic> <message>
ALERT <time> <user> <item> price <new price> below <threshold>
```

Delivery rules:

- A user subscribed twice to the same topic gets `ERROR already subscribed <user> <topic>`.
  Combining `below` with `at` or `every` is `ERROR bad command`.
- `PUB` reaches each user at most once. If several of a user's subscriptions cover the topic,
  the most specific one (the longest topic) decides whether the alert is sent now or held.
- `below` subscriptions only react to `PRICE` on exactly that item, never to `PUB`.
- A `PRICE` alerts a `below <t>` subscriber when the new price is less than t and the
  previous price for that item was t or more. The first price ever seen for an item counts
  as a drop from infinity. A subscriber is not alerted again until the price has gone back
  up to t or more and dropped again.
- Order of ALERT lines for one `PUB`: by when the deciding subscription was made, oldest
  first. For one `PRICE`: by threshold, highest first, then by when the subscription was made.
- During `TICK`, deliveries happen in time order; deliveries due at the same minute go in the
  order their subscriptions were made. Each delivery prints its held publications in the
  order they were published, all stamped with the delivery time. A delivery with nothing
  held prints nothing.
- A publication made at the same minute as a scheduled delivery that has already happened
  waits for the next one.
- Each user's inbox holds the last K alerts. K is 10 unless set with `--inbox K` on the
  command line.

`HISTORY` event lines, which are kept in full even after the inbox drops old alerts:

```
<time> SUB <topic> [below <price> | at <HH:MM> | every <hours>]   (as typed)
<time> UNSUB <topic>
<time> ALERT <topic> <message>
```
