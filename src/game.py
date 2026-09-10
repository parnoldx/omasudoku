"""
Sudoku Game Engine and QObject bridge for Qt Quick.
Maintains board state, timer, points, factor multiplier, fails, undo history,
and provides full navigation and input handling.
"""

from typing import Optional
from PySide6.QtCore import QObject, Signal, Property, Slot, QTimer
from src.generator import Difficulty, SudokuEngine
from src.storage import StorageManager


class SudokuGame(QObject):
    boardChanged = Signal()
    scoreChanged = Signal()
    factorChanged = Signal()
    failsChanged = Signal()
    timeChanged = Signal()
    selectionChanged = Signal()
    notesModeChanged = Signal()
    gameStateChanged = Signal()
    canResumeChanged = Signal()

    # Animation events
    cellFlash = Signal(int, int, bool)   # row, col, is_correct
    rowCompleted = Signal(int)           # row index
    colCompleted = Signal(int)           # col index
    boxCompleted = Signal(int)           # box index (0-8)
    gameWon = Signal(int, int, int, bool) # points, fails, highscore, is_new_record

    TIME_FACTOR_REDUCE = 34
    POINTS_BLOCK_ROW = 11.25

    def __init__(self, storage=None, parent=None):
        super().__init__(parent)
        self.storage = storage if storage is not None else StorageManager()

        self._board = [0] * 81
        self._solution = [0] * 81
        self._initial_clues = [False] * 81
        self._notes = [set() for _ in range(81)]

        self._selected_row = -1
        self._selected_col = -1
        self._highlight_num = 0

        self._difficulty = Difficulty.EASY
        self._points = 0
        self._factor = 28
        self._time = 0
        self._fails = 0
        self._is_paused = False
        self._in_game = False
        self._notes_mode = False

        # Undo history: list of dicts with previous states of modified cells
        self._history = []

        # Game timer
        self._timer = QTimer(self)
        self._timer.setInterval(1000)
        self._timer.timeout.connect(self._on_second_tick)

    # --- Timers & Factor Decay ---
    def _on_second_tick(self):
        if self._is_paused or not self._in_game:
            return
        self._time += 1
        self.timeChanged.emit()

        if self._time % self.TIME_FACTOR_REDUCE == 0 and self._factor > 1:
            self._factor -= 1
            self.factorChanged.emit()

    # --- Start & Resume ---
    @Slot(str)
    def startNewGame(self, difficulty_key: str):
        self._difficulty = Difficulty.from_key(difficulty_key)
        puzzle, solution = SudokuEngine.generate_puzzle(self._difficulty)

        self._board = list(puzzle)
        self._solution = list(solution)
        self._initial_clues = [val != 0 for val in puzzle]
        self._notes = [set() for _ in range(81)]
        self._history.clear()

        self._factor = self._difficulty.factor
        self._points = 0
        self._time = 0
        self._fails = 0
        self._is_paused = False
        self._in_game = True
        self._notes_mode = False

        # Find first empty cell or default to 0, 0
        self._selected_row = 0
        self._selected_col = 0
        self._update_highlight_num()

        self._timer.start()

        self.boardChanged.emit()
        self.scoreChanged.emit()
        self.factorChanged.emit()
        self.failsChanged.emit()
        self.timeChanged.emit()
        self.selectionChanged.emit()
        self.notesModeChanged.emit()
        self.gameStateChanged.emit()
        self.storage.delete_saved_game()
        self.canResumeChanged.emit()

    @Slot()
    def resumeGame(self):
        data = self.storage.load_game()
        if not data:
            return

        self._difficulty = Difficulty.from_key(data.get("difficulty", "simple"))
        self._board = list(data.get("board", [0] * 81))
        self._solution = list(data.get("solution", [0] * 81))
        self._initial_clues = list(data.get("initial_clues", [False] * 81))

        raw_notes = data.get("notes", [])
        self._notes = [set(n) for n in raw_notes] if raw_notes else [set() for _ in range(81)]

        self._factor = int(data.get("factor", self._difficulty.factor))
        self._points = int(data.get("points", 0))
        self._time = int(data.get("time", 0))
        self._fails = int(data.get("fails", 0))

        self._selected_row = 0
        self._selected_col = 0
        self._update_highlight_num()

        self._is_paused = False
        self._in_game = True
        self._notes_mode = False
        self._history.clear()

        self._timer.start()

        self.boardChanged.emit()
        self.scoreChanged.emit()
        self.factorChanged.emit()
        self.failsChanged.emit()
        self.timeChanged.emit()
        self.selectionChanged.emit()
        self.notesModeChanged.emit()
        self.gameStateChanged.emit()

    @Slot()
    def pauseGame(self):
        self._is_paused = True
        self._timer.stop()
        self.gameStateChanged.emit()

    @Slot()
    def resumeTimer(self):
        if self._in_game and self._is_paused:
            self._is_paused = False
            self._timer.start()
            self.gameStateChanged.emit()

    @Slot()
    def togglePause(self):
        if self._is_paused:
            self.resumeTimer()
        else:
            self.pauseGame()

    # --- Save & Storage ---
    @Slot()
    def returnToMenu(self):
        """Pause game and persist progress when leaving to main menu."""
        if self._in_game and not self.is_finished():
            self.pauseGame()
            self.save_current_state()

    @Slot()
    def save_current_state(self):
        if not self._in_game or self.is_finished():
            return
        # Only persist if player made actual moves, notes, or spent time
        has_moves = any(self._board[i] != 0 and not self._initial_clues[i] for i in range(81))
        has_notes = any(len(n) > 0 for n in self._notes)
        if not (has_moves or has_notes or self._time > 10):
            return
        state = {
            "difficulty": self._difficulty.key,
            "board": self._board,
            "solution": self._solution,
            "initial_clues": self._initial_clues,
            "notes": [list(n) for n in self._notes],
            "factor": self._factor,
            "points": self._points,
            "time": self._time,
            "fails": self._fails,
        }
        self.storage.save_game(state)
        self.canResumeChanged.emit()

    # --- Selection & Navigation ---
    @Slot(int, int)
    def selectCell(self, row: int, col: int):
        if 0 <= row < 9 and 0 <= col < 9:
            self._selected_row = row
            self._selected_col = col
            self._update_highlight_num()
            self.selectionChanged.emit()

    @Slot(int, int)
    def moveSelection(self, d_row: int, d_col: int):
        if self._selected_row == -1 or self._selected_col == -1:
            self.selectCell(0, 0)
            return

        new_row = max(0, min(8, self._selected_row + d_row))
        new_col = max(0, min(8, self._selected_col + d_col))
        self.selectCell(new_row, new_col)

    def _update_highlight_num(self):
        if 0 <= self._selected_row < 9 and 0 <= self._selected_col < 9:
            idx = self._selected_row * 9 + self._selected_col
            self._highlight_num = self._board[idx]
        else:
            self._highlight_num = 0

    # --- Input Handling ---
    @Slot(int)
    def enterNumber(self, num: int):
        if not self._in_game or self._is_paused:
            return
        if self._selected_row < 0 or self._selected_col < 0:
            return

        idx = self._selected_row * 9 + self._selected_col

        # Cannot edit initial clue
        if self._initial_clues[idx]:
            return

        if self._notes_mode:
            # Toggle pencil mark note
            if 1 <= num <= 9:
                if num in self._notes[idx]:
                    self._notes[idx].remove(num)
                else:
                    self._notes[idx].add(num)
                self.boardChanged.emit()
            return

        # Regular number entry
        if self._board[idx] != 0:
            # Already filled
            return

        if 1 <= num <= 9:
            expected = self._solution[idx]
            if num == expected:
                # Correct!
                self._history.append({
                    "idx": idx,
                    "prev_val": 0,
                    "prev_notes": set(self._notes[idx]),
                    "points_added": num * self._factor,
                })
                self._board[idx] = num
                self._notes[idx].clear()
                self._points += num * self._factor
                self._update_highlight_num()

                self.cellFlash.emit(self._selected_row, self._selected_col, True)
                self.boardChanged.emit()
                self.scoreChanged.emit()
                self.selectionChanged.emit()

                # Clean up this note from same row, col, and 3x3 box
                self._clear_peer_notes(self._selected_row, self._selected_col, num)

                # Check row, column, and box completions
                self._check_completions(self._selected_row, self._selected_col)

                # Check if puzzle is won
                if self.is_finished():
                    self._on_won()
                else:
                    self.save_current_state()
            else:
                # Fail / wrong number!
                self._fails += 1
                self.failsChanged.emit()
                self.cellFlash.emit(self._selected_row, self._selected_col, False)

    def _clear_peer_notes(self, row: int, col: int, num: int):
        changed = False
        # Row and column
        for i in range(9):
            if num in self._notes[row * 9 + i]:
                self._notes[row * 9 + i].remove(num)
                changed = True
            if num in self._notes[i * 9 + col]:
                self._notes[i * 9 + col].remove(num)
                changed = True
        # Box
        br, bc = (row // 3) * 3, (col // 3) * 3
        for r in range(br, br + 3):
            for c in range(bc, bc + 3):
                if num in self._notes[r * 9 + c]:
                    self._notes[r * 9 + c].remove(num)
                    changed = True
        if changed:
            self.boardChanged.emit()

    def _check_completions(self, row: int, col: int):
        bonus = int(self.POINTS_BLOCK_ROW * self._factor)

        # Col check
        if all(self._board[r * 9 + col] != 0 for r in range(9)):
            self._points += bonus
            self.colCompleted.emit(col)

        # Row check
        if all(self._board[row * 9 + c] != 0 for c in range(9)):
            self._points += bonus
            self.rowCompleted.emit(row)

        # Box check
        br, bc = (row // 3) * 3, (col // 3) * 3
        box_idx = (row // 3) * 3 + (col // 3)
        if all(self._board[r * 9 + c] != 0 for r in range(br, br + 3) for c in range(bc, bc + 3)):
            self._points += bonus
            self.boxCompleted.emit(box_idx)

        self.scoreChanged.emit()

    @Slot()
    def clearSelected(self):
        if not self._in_game or self._is_paused:
            return
        if self._selected_row < 0 or self._selected_col < 0:
            return
        idx = self._selected_row * 9 + self._selected_col
        if self._initial_clues[idx]:
            return

        if self._notes[idx]:
            self._notes[idx].clear()
            self.boardChanged.emit()

    @Slot()
    def toggleNotesMode(self):
        self._notes_mode = not self._notes_mode
        self.notesModeChanged.emit()

    @Slot()
    def undo(self):
        if not self._history:
            return
        last = self._history.pop()
        idx = last["idx"]
        self._board[idx] = last["prev_val"]
        self._notes[idx] = set(last["prev_notes"])
        self._points = max(0, self._points - last.get("points_added", 0))

        self._selected_row, self._selected_col = divmod(idx, 9)
        self._update_highlight_num()

        self.boardChanged.emit()
        self.scoreChanged.emit()
        self.selectionChanged.emit()
        self.save_current_state()

    def is_finished(self) -> bool:
        return all(self._board[i] == self._solution[i] and self._board[i] != 0 for i in range(81))

    @Slot(result=bool)
    def isFinished(self) -> bool:
        return self.is_finished()

    def _on_won(self):
        self._timer.stop()
        self._in_game = False
        self.storage.delete_saved_game()
        self.canResumeChanged.emit()

        is_new_record = False
        if self._fails <= 3:
            is_new_record = self.storage.set_highscore(self._difficulty.key, self._points)

        current_hs = self.storage.get_highscore(self._difficulty.key)
        self.gameWon.emit(self._points, self._fails, current_hs, is_new_record)
        self.gameStateChanged.emit()

    # --- Property Getters for QML ---
    @Property(list, notify=boardChanged)
    def board(self) -> list:
        return self._board

    @Property(list, notify=boardChanged)
    def initialClues(self) -> list:
        return self._initial_clues

    @Property(list, notify=boardChanged)
    def notes(self) -> list:
        # Return list of sorted list of numbers for each cell
        return [sorted(list(n)) for n in self._notes]

    @Property(int, notify=selectionChanged)
    def selectedRow(self) -> int:
        return self._selected_row

    @Property(int, notify=selectionChanged)
    def selectedCol(self) -> int:
        return self._selected_col

    @Property(int, notify=selectionChanged)
    def highlightNum(self) -> int:
        return self._highlight_num

    @Property(int, notify=scoreChanged)
    def points(self) -> int:
        return self._points

    @Property(int, notify=factorChanged)
    def factor(self) -> int:
        return self._factor

    @Property(int, notify=failsChanged)
    def fails(self) -> int:
        return self._fails

    @Property(int, notify=timeChanged)
    def time(self) -> int:
        return self._time

    @Property(str, notify=timeChanged)
    def formattedTime(self) -> str:
        minutes = self._time // 60
        seconds = self._time % 60
        return f"{minutes:02d}:{seconds:02d}"

    @Property(str, notify=gameStateChanged)
    def difficultyKey(self) -> str:
        return self._difficulty.key

    @Property(str, notify=gameStateChanged)
    def difficultyLabel(self) -> str:
        return self._difficulty.label

    @Property(bool, notify=gameStateChanged)
    def isPaused(self) -> bool:
        return self._is_paused

    @Property(bool, notify=gameStateChanged)
    def inGame(self) -> bool:
        return self._in_game

    @Property(bool, notify=notesModeChanged)
    def notesMode(self) -> bool:
        return self._notes_mode

    @Property(bool, notify=canResumeChanged)
    def canResume(self) -> bool:
        return self.storage.has_saved_game()

    @Property(int, notify=gameStateChanged)
    def highscore(self) -> int:
        return self.storage.get_highscore(self._difficulty.key)

    @Slot(str, result=int)
    def getHighscoreFor(self, diff_key: str) -> int:
        return self.storage.get_highscore(diff_key)
