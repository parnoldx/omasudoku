import os
import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from PySide6.QtCore import QCoreApplication

app = QCoreApplication.instance() or QCoreApplication(sys.argv)

from src.generator import Difficulty, SudokuEngine
from src.game import SudokuGame
from src.storage import StorageManager
from src.theme import OmarchyTheme


class TestSudokuEngine(unittest.TestCase):
    def test_solve_count_solved(self):
        board = SudokuEngine.generate_solved_board()
        self.assertEqual(len(board), 81)
        self.assertTrue(all(1 <= x <= 9 for x in board))
        self.assertEqual(SudokuEngine.solve_count(board, limit=2), 1)

    def test_all_difficulties(self):
        for diff in Difficulty:
            puzzle, solution = SudokuEngine.generate_puzzle(diff)
            self.assertEqual(len(puzzle), 81)
            self.assertEqual(len(solution), 81)
            self.assertEqual(SudokuEngine.solve_count(list(puzzle), limit=2), 1)
            for p, s in zip(puzzle, solution):
                if p != 0:
                    self.assertEqual(p, s)


class TestSudokuGameLogic(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp()
        self.storage = StorageManager(base_dir=self.test_dir)
        self.game = SudokuGame(storage=self.storage)

    def tearDown(self):
        shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_start_game_and_properties(self):
        self.game.startNewGame("simple")
        self.assertTrue(self.game.inGame)
        self.assertEqual(self.game.difficultyKey, "simple")
        self.assertEqual(self.game.difficultyLabel, "Easy")
        self.assertEqual(self.game.factor, 28)
        self.assertEqual(self.game.points, 0)
        self.assertEqual(self.game.fails, 0)
        self.assertEqual(self.game.time, 0)
        self.assertEqual(self.game.formattedTime, "00:00")

    def test_selection_and_navigation(self):
        self.game.startNewGame("simple")
        self.game.selectCell(0, 0)
        self.assertEqual(self.game.selectedRow, 0)
        self.assertEqual(self.game.selectedCol, 0)

        self.game.moveSelection(0, 1)
        self.assertEqual(self.game.selectedCol, 1)

        self.game.moveSelection(1, 0)
        self.assertEqual(self.game.selectedRow, 1)

        self.game.selectCell(8, 8)
        self.game.moveSelection(1, 1)
        self.assertEqual(self.game.selectedRow, 8)
        self.assertEqual(self.game.selectedCol, 8)

        self.game.selectCell(0, 0)
        self.game.moveSelection(-1, -1)
        self.assertEqual(self.game.selectedRow, 0)
        self.assertEqual(self.game.selectedCol, 0)

    def test_correct_entry_and_scoring(self):
        self.game.startNewGame("simple")
        idx = next(i for i in range(81) if self.game.board[i] == 0)
        r, c = divmod(idx, 9)
        self.game.selectCell(r, c)

        correct_val = self.game._solution[idx]
        factor = self.game.factor
        expected_points = correct_val * factor

        self.game.enterNumber(correct_val)
        self.assertEqual(self.game.board[idx], correct_val)
        self.assertGreaterEqual(self.game.points, expected_points)
        self.assertEqual(self.game.fails, 0)

    def test_wrong_entry_and_fail_count(self):
        self.game.startNewGame("simple")
        idx = next(i for i in range(81) if self.game.board[i] == 0)
        r, c = divmod(idx, 9)
        self.game.selectCell(r, c)

        correct_val = self.game._solution[idx]
        wrong_val = (correct_val % 9) + 1

        self.game.enterNumber(wrong_val)
        self.assertEqual(self.game.fails, 1)
        self.assertEqual(self.game.board[idx], 0)
        self.assertEqual(self.game.points, 0)

    def test_notes_mode(self):
        self.game.startNewGame("simple")
        idx = next(i for i in range(81) if self.game.board[i] == 0)
        r, c = divmod(idx, 9)
        self.game.selectCell(r, c)

        self.game.toggleNotesMode()
        self.assertTrue(self.game.notesMode)

        self.game.enterNumber(3)
        self.game.enterNumber(7)
        self.assertIn(3, self.game.notes[idx])
        self.assertIn(7, self.game.notes[idx])

        self.game.enterNumber(3)
        self.assertNotIn(3, self.game.notes[idx])
        self.assertIn(7, self.game.notes[idx])

    def test_factor_decay_on_tick(self):
        self.game.startNewGame("simple")
        initial_factor = self.game.factor
        for _ in range(34):
            self.game._on_second_tick()
        self.assertEqual(self.game.time, 34)
        self.assertEqual(self.game.factor, initial_factor - 1)

    def test_undo(self):
        self.game.startNewGame("simple")
        idx = next(i for i in range(81) if self.game.board[i] == 0)
        r, c = divmod(idx, 9)
        self.game.selectCell(r, c)

        correct_val = self.game._solution[idx]
        self.game.enterNumber(correct_val)
        self.assertEqual(self.game.board[idx], correct_val)
        self.assertGreater(self.game.points, 0)

        self.game.undo()
        self.assertEqual(self.game.board[idx], 0)
        self.assertEqual(self.game.points, 0)

    def test_save_and_resume_in_isolated_dir(self):
        # Starts with no saved game
        self.assertFalse(self.game.canResume)

        self.game.startNewGame("simple")
        idx = next(i for i in range(81) if self.game.board[i] == 0)
        r, c = divmod(idx, 9)
        self.game.selectCell(r, c)
        correct_val = self.game._solution[idx]
        self.game.enterNumber(correct_val)

        self.game.save_current_state()
        self.assertTrue(self.game.canResume)

        # Fresh instance using same test directory
        resumed_game = SudokuGame(storage=self.storage)
        self.assertTrue(resumed_game.canResume)
        resumed_game.resumeGame()
        self.assertTrue(resumed_game.inGame)
        self.assertEqual(resumed_game.board[idx], correct_val)
        self.assertEqual(resumed_game.points, self.game.points)

    def test_no_resume_on_fresh_game_without_moves(self):
        # Fresh game started, but no moves made
        self.assertFalse(self.game.canResume)
        self.game.startNewGame("intermediate")
        # should not be resumable since untouched
        self.assertFalse(self.game.canResume)
        self.assertFalse(self.storage.has_saved_game())

        # Calling save_current_state on untouched game still shouldn't save
        self.game.save_current_state()
        self.assertFalse(self.game.canResume)
        self.assertFalse(self.storage.has_saved_game())


class TestOmarchyTheme(unittest.TestCase):
    def test_theme_properties(self):
        theme = OmarchyTheme()
        self.assertIn(theme.mode, ["dark", "light"])
        self.assertTrue(theme.accent.isValid())
        self.assertTrue(theme.background.isValid())
        self.assertTrue(theme.foreground.isValid())


if __name__ == "__main__":
    unittest.main()
