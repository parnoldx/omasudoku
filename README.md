# Sudoku for Omarchy

A native, modern Sudoku arcade puzzle game built for **Omarchy** with **Qt Quick (QML) & PySide6**.

Inspired by and honoring the game mechanics, arcade scoring, and ergonomics of Peter Arnold's previous elementary OS game ([parnoldx/sudoku](https://github.com/parnoldx/sudoku)).

![Omarchy Sudoku](data/org.omarchy.sudoku.svg)

---

## Highlights & Features

- **Native Omarchy Theme Integration**:
  - Automatically loads and live-binds to `~/.local/state/omarchy/current/theme/colors.toml`.
  - Supports live hot-reloading when your Omarchy theme changes without restarting the game.
- **Arcade Pacing & Scoring (from the elementary OS game)**:
  - **4 Difficulty Levels**:
    - **Easy** (`simple`): Initial factor **x28** (~40 clues)
    - **Medium** (`easy`): Initial factor **x56** (~34 clues)
    - **Hard** (`intermediate`): Initial factor **x112** (~29 clues)
    - **Master** (`expert`): Initial factor **x156** (~25 clues)
  - **Time Multiplier Decay**: Factor reduces by 1 every 34 seconds (down to minimum 1).
  - **Points**: `value * factor` for every correct number placed.
  - **Completion Bonuses**: Completing a row, column, or 3×3 box awards `11.25 * factor` points and triggers a ripple wave animation across the completed line or box!
  - **Mistakes & Series Rules**:
    - Header tracks mistakes (`✓`, `✗`, `✗ ✗`, `✗ ✗ ✗`).
    - More than 3 mistakes breaks the series (`BROKEN SERIES`) and invalidates highscore recording for that run.
- **Fluid Qt Quick Animations**:
  - Smooth tile reveal zoom, matching number halos, wave completion animations, and error shake.
- **Ergonomic Controls**:
  - **Arrow keys** and **Vim keys** (`h`, `j`, `k`, `l`) for navigation.
  - **Digit keys** (`1`–`9`) and **Numpad**.
  - **Left-Hand Keypad** (from the elementary OS game):
    - `q, w, e` → 7, 8, 9
    - `a, s, d` → 4, 5, 6
    - `z, x, c` / `y, x, c` → 1, 2, 3
  - **Pencil Mark / Notes Mode**: `N` or `Space` (with auto-cleaning peer notes when a cell is solved).
  - **Undo**: `U` or `Ctrl+Z`.
  - **Pause**: `P` or spacebar.
- **Zero-Dependency Engine**:
  - Lightning-fast native Python bitmask solver (~3ms) with MRV heuristic ensuring valid unique solutions.
  - Automatic `qqwing` CLI fallback if installed.
- **Persistence**:
  - Automatic game state autosave and "Resume Unfinished Game" button on the welcome screen.
  - High scores tracked per difficulty level in `~/.local/share/omarchy-sudoku/highscores.json`.

---

## Installation & Launching

Run the installation script:
```bash
./install.sh
```

This installs:
- Executable launcher: `~/.local/bin/omarchy-sudoku`
- Desktop entry: `~/.local/share/applications/org.omarchy.sudoku.desktop`
- Scalable SVG icon: `~/.local/share/icons/hicolor/scalable/apps/org.omarchy.sudoku.svg`

You can launch it directly from your terminal:
```bash
omarchy-sudoku
```
Or search for **Sudoku** in your Omarchy application launcher (`SUPER + SPACE` or `omarchy menu`).

---

## Running Tests

Run the test suite:
```bash
python3 -m unittest discover tests/
```
