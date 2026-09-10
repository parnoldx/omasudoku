"""
Sudoku Puzzle Generator and Solver.
High-speed bitmask-based backtracking solver with MRV (Minimum Remaining Values).
Supports unique solution verification and difficulty-targeted clue carving.
Also supports qqwing CLI if available on the system.
"""

from enum import Enum
import random
import shutil
import subprocess


class Difficulty(Enum):
    EASY = ("simple", "Easy", 28, 40)
    MEDIUM = ("easy", "Medium", 56, 34)
    HARD = ("intermediate", "Hard", 112, 29)
    MASTER = ("expert", "Master", 156, 25)

    def __init__(self, key: str, label: str, factor: int, target_clues: int):
        self.key = key
        self.label = label
        self.factor = factor
        self.target_clues = target_clues

    @classmethod
    def from_key(cls, key: str) -> "Difficulty":
        for diff in cls:
            if diff.key == key or diff.name.lower() == key.lower() or diff.label.lower() == key.lower():
                return diff
        return cls.EASY


class SudokuEngine:
    """Fast bitmask solver and puzzle generator."""

    @staticmethod
    def _box_index(r: int, c: int) -> int:
        return (r // 3) * 3 + (c // 3)

    @classmethod
    def solve_count(cls, board: list[int], limit: int = 2) -> int:
        """Count solutions up to limit (returns 0, 1, or limit)."""
        rows = [0] * 9
        cols = [0] * 9
        boxes = [0] * 9

        for idx, val in enumerate(board):
            if val != 0:
                r, c = divmod(idx, 9)
                mask = 1 << val
                rows[r] |= mask
                cols[c] |= mask
                boxes[cls._box_index(r, c)] |= mask

        def backtrack(remaining: int) -> int:
            nonlocal limit
            # MRV heuristic
            min_count = 10
            best_pos = -1
            best_mask = 0

            for idx in range(81):
                if board[idx] == 0:
                    r, c = divmod(idx, 9)
                    used = rows[r] | cols[c] | boxes[cls._box_index(r, c)]
                    avail = 0x3FE & (~used)
                    cnt = bin(avail).count("1")
                    if cnt == 0:
                        return 0
                    if cnt < min_count:
                        min_count = cnt
                        best_pos = idx
                        best_mask = avail
                        if cnt == 1:
                            break

            if best_pos == -1:
                return 1

            r, c = divmod(best_pos, 9)
            b = cls._box_index(r, c)
            found = 0

            for val in range(1, 10):
                val_mask = 1 << val
                if best_mask & val_mask:
                    board[best_pos] = val
                    rows[r] |= val_mask
                    cols[c] |= val_mask
                    boxes[b] |= val_mask

                    found += backtrack(remaining - found)

                    board[best_pos] = 0
                    clear_mask = ~val_mask
                    rows[r] &= clear_mask
                    cols[c] &= clear_mask
                    boxes[b] &= clear_mask

                    if found >= remaining:
                        break

            return found

        return backtrack(limit)

    @classmethod
    def generate_solved_board(cls) -> list[int]:
        """Generate a random valid 9x9 completed board."""
        rows = [0] * 9
        cols = [0] * 9
        boxes = [0] * 9
        board = [0] * 81

        def set_val(r: int, c: int, val: int):
            board[r * 9 + c] = val
            mask = 1 << val
            rows[r] |= mask
            cols[c] |= mask
            boxes[cls._box_index(r, c)] |= mask

        def clear_val(r: int, c: int, val: int):
            board[r * 9 + c] = 0
            mask = ~(1 << val)
            rows[r] &= mask
            cols[c] &= mask
            boxes[cls._box_index(r, c)] &= mask

        # Fill the 3 independent 3x3 diagonal boxes first
        for b in [0, 4, 8]:
            br, bc = (b // 3) * 3, (b % 3) * 3
            nums = list(range(1, 10))
            random.shuffle(nums)
            for i in range(3):
                for j in range(3):
                    set_val(br + i, bc + j, nums[i * 3 + j])

        # Fill remaining with randomized candidates
        def fill_cells() -> bool:
            min_count = 10
            best_pos = -1
            best_avail = []

            for idx in range(81):
                if board[idx] == 0:
                    r, c = divmod(idx, 9)
                    used = rows[r] | cols[c] | boxes[cls._box_index(r, c)]
                    avail = [n for n in range(1, 10) if not (used & (1 << n))]
                    if not avail:
                        return False
                    if len(avail) < min_count:
                        min_count = len(avail)
                        best_pos = idx
                        best_avail = avail
                        if min_count == 1:
                            break

            if best_pos == -1:
                return True

            r, c = divmod(best_pos, 9)
            random.shuffle(best_avail)
            for n in best_avail:
                set_val(r, c, n)
                if fill_cells():
                    return True
                clear_val(r, c, n)

            return False

        fill_cells()
        return board

    @classmethod
    def generate_puzzle(cls, difficulty: Difficulty) -> tuple[list[int], list[int]]:
        """
        Generate (puzzle, solution) ensuring exactly 1 unique solution.
        First tries qqwing if installed; otherwise uses fast native generation.
        """
        # Check qqwing first if available
        if shutil.which("qqwing"):
            try:
                proc = subprocess.run(
                    ["qqwing", "--generate", "--solution", "--one-line", "--difficulty", difficulty.key],
                    capture_output=True,
                    text=True,
                    timeout=2,
                    check=True,
                )
                lines = [line.strip() for line in proc.stdout.strip().splitlines() if line.strip()]
                if len(lines) >= 2:
                    puzzle = [int(c) if c.isdigit() else 0 for c in lines[0]]
                    solution = [int(c) if c.isdigit() else 0 for c in lines[1]]
                    if len(puzzle) == 81 and len(solution) == 81:
                        return puzzle, solution
            except Exception:
                pass

        # Native generator
        solution = cls.generate_solved_board()
        board = list(solution)
        target = difficulty.target_clues

        # Symmetric pairs removal
        pairs = []
        for r in range(5):
            for c in range(9):
                pos1 = r * 9 + c
                pos2 = (8 - r) * 9 + (8 - c)
                if pos1 <= pos2:
                    pairs.append((pos1, pos2))

        random.shuffle(pairs)
        current_clues = 81

        for pos1, pos2 in pairs:
            if current_clues <= target:
                break

            val1 = board[pos1]
            val2 = board[pos2]
            board[pos1] = 0
            board[pos2] = 0

            # Test unique solution
            if cls.solve_count(board, limit=2) == 1:
                current_clues -= (2 if pos1 != pos2 else 1)
            else:
                # Backtrack
                board[pos1] = val1
                board[pos2] = val2

        # If not enough clues removed by symmetry, try single-cell removals
        if current_clues > target:
            singles = [i for i in range(81) if board[i] != 0]
            random.shuffle(singles)
            for pos in singles:
                if current_clues <= target:
                    break
                val = board[pos]
                board[pos] = 0
                if cls.solve_count(board, limit=2) == 1:
                    current_clues -= 1
                else:
                    board[pos] = val

        return board, solution
