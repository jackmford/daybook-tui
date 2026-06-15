# Daybook: C TUI Habit & Journal Tracker Project Plan

## Project Idea

**Daybook** is a terminal-based habit and journal tracker written in C.

The goal is not just to build another todo app. The goal is to build a small, personal, local-first terminal tool that helps answer:

> Why did I or didn’t I do the thing today?

It should eventually help track habits, streaks, daily notes, blockers, energy, and patterns over time.

The project also exists as a way to learn lower-level programming concepts through something personally useful and visually satisfying.

---

## Core TUI Mental Model

A TUI is mostly this loop:

```text
draw screen
read keypress
update state
draw screen again
```

At the lowest level, a TUI is controlled printing:

- Clear the screen
- Move the cursor
- Print text at specific positions
- Read keypresses
- Update internal state
- Redraw the screen

---

## Current Status: Milestone 0

You already have a basic C program that:

- Prints text
- Clears the terminal screen
- Moves the cursor home
- Prints a simple Daybook screen

Current program:

```c
#include <stdio.h>


// need a way to define "streaks"
// they should have days
//
// -> a TUI is just controlled printing

int main(int argc, char* argv[]) {
	printf("Hello World is a disappearer...\n");
	// \033 == ESC
	// \033[2J clears the screen
	// \033[H is cursor-home
	printf("\033[2J\033[H");
	printf("Daybook\n");
	printf("[ ] Study C\n");
	printf("[ ] Run\n");
	printf("[ ] Read\n");
	return 0;
}
```

Important correction:

```text
\033[2J clears the screen
\033[H moves the cursor to the home position
```

The uppercase/lowercase matters. `\033[H` is the cursor-home sequence.

---

## Target Version 1

The first real version should be intentionally tiny.

When running:

```bash
./daybook
```

The screen should show:

```text
Daybook

> [ ] Study C
  [ ] Run
  [ ] Read

j/k move · space toggle · q quit
```

Behavior:

```text
j       move selection down
k       move selection up
space   toggle selected habit
q       quit
```

No persistence yet. No dates. No journal. No config. Just a screen that feels alive.

---

## Project Phases

## Phase 1: Controlled Printing

Goal: manually draw a simple terminal screen.

Learn:

- `printf`
- ANSI escape codes
- Clearing the screen
- Moving the cursor
- Redrawing output

Useful escape codes:

```text
\033[2J      clear entire screen
\033[H       move cursor to top-left / home position
\033[?25l    hide cursor
\033[?25h    show cursor
```

Deliverable:

```text
Daybook
[x] Study C
[ ] Run
[ ] Read
```

---

## Phase 2: Raw Keyboard Input

Goal: read one keypress at a time without waiting for Enter.

Normal terminal behavior is line-based:

```text
type characters -> press Enter -> program receives input
```

A TUI needs immediate keypresses:

```text
press j -> program receives j immediately
press q -> program quits immediately
```

Learn:

- `termios`
- canonical mode
- echo mode
- disabling line buffering
- restoring terminal settings on exit

Concepts:

```text
canonical mode = line-by-line input
raw-ish mode   = character-by-character input
echo on        = terminal displays what you type
echo off       = your program controls the display
```

Deliverable:

A program that prints each key as soon as it is pressed, without requiring Enter.

---

## Phase 3: Selection Cursor

Goal: move a selector through a list of habits.

Screen:

```text
Daybook

> [ ] Study C
  [ ] Run
  [ ] Read

j/k move · q quit
```

Learn:

- Program state
- Arrays
- Indexes
- Input loop
- Redrawing

State needed:

```text
selected_index = 0
habit_count = 3
habits = ["Study C", "Run", "Read"]
```

Rules:

```text
j moves down unless already at the bottom
k moves up unless already at the top
q exits the loop
```

Deliverable:

A selector that moves with `j` and `k`.

---

## Phase 4: Toggle Habits

Goal: press space to toggle the selected habit.

Screen:

```text
Daybook

> [x] Study C
  [ ] Run
  [ ] Read

j/k move · space toggle · q quit
```

Learn:

- Structs
- Booleans / integers as booleans
- Updating state
- Redrawing UI from state

Possible data model:

```c
struct Habit {
    char name[64];
    int done;
};
```

Rules:

```text
space toggles done from 0 -> 1 or 1 -> 0
screen redraws after each keypress
```

Deliverable:

A working in-memory habit checklist.

---

## Phase 5: Save and Load Today

Goal: persist today’s habit state to disk.

Possible file:

```text
~/.daybook/today.txt
```

Possible contents:

```text
Study C=false
Run=true
Read=false
```

Learn:

- `fopen`
- `fgets`
- `fprintf`
- `fclose`
- String parsing
- Error handling
- Creating directories
- Using `$HOME`

Behavior:

```text
start program -> load today's file if it exists
make changes -> save on quit
```

Deliverable:

Quit and reopen the app with habit state preserved.

---

## Phase 6: Add a Journal Entry

Goal: add a simple daily note.

Screen:

```text
Daybook

Habits
> [x] Study C
  [ ] Run
  [x] Read

Journal
Felt tired after work, but did 20 minutes.

j/k move · space toggle · e edit journal · q quit
```

Important rule:

Do not build a full text editor yet.

When the user presses `e`, temporarily leave raw mode and prompt for a normal line:

```text
Journal entry:
>
```

Then return to the TUI.

Learn:

- Switching terminal modes
- Reading lines with `fgets`
- Storing strings
- Saving multiline or single-line text

Deliverable:

A daily habit tracker with one journal note.

---

## Phase 7: Dates and Daily Files

Goal: store a separate entry per day.

Possible structure:

```text
~/.daybook/
  habits.txt
  entries/
    2026-06-12.txt
    2026-06-13.txt
```

Possible daily entry:

```text
date: 2026-06-12
energy: 6
mood: decent

habit: Study C false unclear-next-action
habit: Run true
habit: Read true

journal:
Felt tired after work but read for 20 minutes.
```

Learn:

- Working with dates in C
- File paths
- Directory structure
- More robust parsing

Deliverable:

Opening Daybook always loads the entry for today’s date.

---

## Phase 8: Weekly Review

Goal: summarize patterns over the last week.

Possible screen:

```text
Weekly Review

Study C     3/7
Run         4/7
Read        5/7

Most common blockers:
- tired after work
- unclear next action
```

Learn:

- Reading multiple files
- Aggregating data
- Counting completions
- Basic reporting

Deliverable:

A weekly summary screen.

---

## Phase 9: Blockers and Reflection

Goal: track why a habit was missed.

Possible prompt:

```text
Missed: Study C

Why?
[1] Too tired
[2] No clear next step
[3] Too much work
[4] Forgot
[5] Chose something else
[6] Other
```

This is the feature that makes Daybook different from a normal habit tracker.

Most habit trackers only ask:

```text
Did you do it?
```

Daybook should ask:

```text
What got in the way?
```

Deliverable:

Missed habits can have a reason attached.

---

## Phase 10: Optional ncurses Rewrite or Expansion

After building the basics manually, consider using `ncurses`.

Do not start here. First learn the raw mechanics.

`ncurses` can help with:

- Windows
- Boxes
- Colors
- Terminal resizing
- Keyboard handling
- Cleaner screen refreshing

Possible future screen:

```text
┌ Today ─────────────┐ ┌ Streaks ───────────┐
│ [x] Run            │ │ Read       5 days   │
│ [ ] Study C        │ │ Study C    1 day    │
│ [x] Read           │ │ Run        3 days   │
└────────────────────┘ └────────────────────┘

┌ Journal ──────────────────────────────────┐
│ Felt low energy after work. Need smaller  │
│ next action for C tomorrow.               │
└───────────────────────────────────────────┘
```

---

## Suggested Study Sessions

## Session 1: Terminal Output

Goal: clear the screen and print a basic Daybook view.

You are already here.

Next improvements:

- Hide the cursor
- Restore the cursor before exiting
- Wrap drawing in a `draw_screen()` function

---

## Session 2: Raw Input

Goal: read single keypresses immediately.

Build a tiny program that:

```text
press any key -> prints the key
press q -> quits
```

No Daybook logic yet.

---

## Session 3: Main Loop

Goal: combine drawing and input.

Pseudo-logic:

```text
setup terminal
while running:
    draw screen
    read key
    update state
restore terminal
```

---

## Session 4: Move Selection

Goal: use `j` and `k` to move through habits.

---

## Session 5: Toggle Habit

Goal: press space to check or uncheck a habit.

---

## Session 6: Save and Load

Goal: persist habit state to disk.

---

## Design Principles

## 1. Build the smallest useful thing

Do not build a giant productivity system.

Start with:

```text
three habits
one screen
three keys
no persistence
```

Then grow it.

## 2. Make it feel alive early

The first magic moment is moving a selector and toggling a checkbox.

Get there quickly.

## 3. Plain text first

Use simple files before SQLite.

Plain text is easier to debug and feels Unix-y.

## 4. Track friction, not just streaks

The most interesting feature is not streak-counting.

The most interesting feature is discovering why habits fail.

## 5. The app should help build itself

Use Daybook to track work on Daybook.

Example habit:

```text
[ ] Work on Daybook for 25 minutes
```

When you miss it, log why.

---

## Immediate Next Step

Refactor the current program into a `draw_screen()` function.

Target shape:

```text
main()
  draw_screen()
  return 0
```

Then add the next milestone: raw input.
