"""
Persistence manager for Sudoku game state and highscores.
Stores JSON files in ~/.local/share/omarchy-sudoku/.
"""

import json
import os
from pathlib import Path
from typing import Any, Optional


class StorageManager:
    def __init__(self):
        data_dir = os.environ.get("XDG_DATA_HOME")
        if data_dir:
            self.base_dir = Path(data_dir) / "omarchy-sudoku"
        else:
            self.base_dir = Path.home() / ".local" / "share" / "omarchy-sudoku"

        self.base_dir.mkdir(parents=True, exist_ok=True)
        self.save_file = self.base_dir / "savegame.json"
        self.highscore_file = self.base_dir / "highscores.json"

    def has_saved_game(self) -> bool:
        return self.save_file.is_file()

    def save_game(self, state: dict[str, Any]) -> None:
        try:
            temp_file = self.save_file.with_suffix(".tmp")
            with open(temp_file, "w", encoding="utf-8") as f:
                json.dump(state, f, indent=2)
            temp_file.replace(self.save_file)
        except Exception as e:
            print(f"Error saving game: {e}")

    def load_game(self) -> Optional[dict[str, Any]]:
        if not self.has_saved_game():
            return None
        try:
            with open(self.save_file, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            print(f"Error loading saved game: {e}")
            return None

    def delete_saved_game(self) -> None:
        if self.save_file.exists():
            try:
                self.save_file.unlink()
            except Exception as e:
                print(f"Error deleting saved game: {e}")

    def get_all_highscores(self) -> dict[str, int]:
        if not self.highscore_file.is_file():
            return {"simple": 0, "easy": 0, "intermediate": 0, "expert": 0}
        try:
            with open(self.highscore_file, "r", encoding="utf-8") as f:
                data = json.load(f)
                return {
                    "simple": int(data.get("simple", 0)),
                    "easy": int(data.get("easy", 0)),
                    "intermediate": int(data.get("intermediate", 0)),
                    "expert": int(data.get("expert", 0)),
                }
        except Exception as e:
            print(f"Error reading highscores: {e}")
            return {"simple": 0, "easy": 0, "intermediate": 0, "expert": 0}

    def get_highscore(self, difficulty_key: str) -> int:
        scores = self.get_all_highscores()
        return scores.get(difficulty_key, 0)

    def set_highscore(self, difficulty_key: str, score: int) -> bool:
        """Update highscore if greater. Returns True if a new highscore was set."""
        scores = self.get_all_highscores()
        current = scores.get(difficulty_key, 0)
        if score > current:
            scores[difficulty_key] = score
            try:
                temp_file = self.highscore_file.with_suffix(".tmp")
                with open(temp_file, "w", encoding="utf-8") as f:
                    json.dump(scores, f, indent=2)
                temp_file.replace(self.highscore_file)
                return True
            except Exception as e:
                print(f"Error writing highscores: {e}")
        return False
