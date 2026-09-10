# Sudoku for Omarchy

**Website:** [parnoldx.github.io/omasudoku](https://parnoldx.github.io/omasudoku/)

A native Sudoku arcade puzzle for **Omarchy**, built with **C++/Qt6** and **Qt Quick (QML)**.

Inspired by the arcade scoring and ergonomics of the earlier elementary OS game ([parnoldx/sudoku](https://github.com/parnoldx/sudoku)).

![Omarchy Sudoku](data/org.omarchy.sudoku.svg)

---

## Highlights

- **Omarchy theme** — follows `~/.local/state/omarchy/current/theme/colors.toml` and reloads live when the theme changes.
- **Arcade scoring** — four difficulties (Easy → Master), time-decay multiplier, line/box bonuses, three-mistake series.
- **Keyboard-first** — arrows / vim keys, digits & numpad, left-hand keypad (`qwe`/`asd`/`zxc`), notes, undo, pause. Digit keys also highlight matching numbers on the board.
- **Native engine** — C++ bitmask solver (optional `qqwing` fallback).
- **Autosave & highscores** — resume unfinished games; best scores per difficulty in `~/.local/share/omarchy-sudoku/`.

---

## Install

```bash
./install.sh
```

This builds the Qt6 binary and installs:

- `~/.local/bin/omarchy-sudoku`
- Desktop entry + icon for the Omarchy app launcher

Then run `omarchy-sudoku` or search for **Sudoku**.

---

## Build & test (developers)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/omarchy-sudoku

cmake --build build --target test_game
./build/test_game
# or: ctest --test-dir build --output-on-failure
```

Requires Qt6 (Core, Gui, Qml, Quick, Test), CMake ≥ 3.21, and a C++17 compiler. Ninja is optional.
