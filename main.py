#!/usr/bin/env python3
"""
Sudoku for Omarchy
Native Qt Quick (QML) game with Omarchy theme integration.
"""

import os
from pathlib import Path
import signal
import sys

from PySide6.QtCore import QUrl, QCoreApplication
from PySide6.QtGui import QGuiApplication, QIcon
from PySide6.QtQml import QQmlApplicationEngine

from src.game import SudokuGame
from src.theme import OmarchyTheme


def main():
    # Handle SIGINT cleanly
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    # Wayland / App settings
    os.environ.setdefault("QT_QPA_PLATFORM", "wayland;xcb")

    app = QGuiApplication(sys.argv)
    app.setOrganizationName("Omarchy")
    app.setApplicationName("Sudoku")
    app.setDesktopFileName("org.omarchy.sudoku")

    # Icon
    icon_path = Path(__file__).parent / "data" / "org.omarchy.sudoku.svg"
    if icon_path.is_file():
        app.setWindowIcon(QIcon(str(icon_path)))

    theme = OmarchyTheme()
    game = SudokuGame()

    engine = QQmlApplicationEngine()
    context = engine.rootContext()
    context.setContextProperty("theme", theme)
    context.setContextProperty("game", game)

    # Save game on application exit if in progress
    def on_about_to_quit():
        if game.inGame and not game.is_finished():
            game.save_current_state()

    app.aboutToQuit.connect(on_about_to_quit)

    qml_file = Path(__file__).parent / "qml" / "Main.qml"
    engine.load(QUrl.fromLocalFile(str(qml_file)))

    if not engine.rootObjects():
        print("Error: Could not load QML user interface.", file=sys.stderr)
        sys.exit(1)

    sys.exit(app.exec())


if __name__ == "__main__":
    main()
