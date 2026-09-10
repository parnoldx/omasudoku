# Sudoku for Omarchy

**Website:** [parnoldx.github.io/omasudoku](https://parnoldx.github.io/omasudoku/)

A native Sudoku arcade puzzle for **Omarchy**.

![Omarchy Sudoku](data/org.omarchy.sudoku.svg)

---

## Highlights

- **Looks like Omarchy** — picks up your desktop theme and updates live when it changes.
- **Arcade scoring** — four difficulties (Easy → Master), time-decay multiplier, line/box bonuses, three-mistake series.
- **Keyboard-first** — arrows / vim keys, digits & numpad, left-hand keypad (`qwe`/`asd`/`zxc`), notes, undo, pause. Digit keys also highlight matching numbers on the board.
- **Pick up later** — unfinished games resume; best scores per difficulty.

---

## Install

```bash
./install.sh
```

That builds and installs:

- `~/.local/bin/omarchy-sudoku`
- Desktop entry + icon for the Omarchy app launcher

Then run `omarchy-sudoku` or search for **Sudoku**.

---

## Build & test (developers)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/omarchy-sudoku

cmake --build build --target test_game
./build/test_game
# or: ctest --test-dir build --output-on-failure
```
