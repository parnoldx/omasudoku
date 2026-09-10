"""
Theme manager for Omarchy.
Parses ~/.local/state/omarchy/current/theme/colors.toml
and monitors it using QFileSystemWatcher to support real-time hot-reloading.
"""

import os
from pathlib import Path
import tomllib
from PySide6.QtCore import QObject, Signal, Property, QFileSystemWatcher, Slot
from PySide6.QtGui import QColor


class OmarchyTheme(QObject):
    themeChanged = Signal()

    def __init__(self, parent=None):
        super().__init__(parent)

        self._theme_dir = Path.home() / ".local" / "state" / "omarchy" / "current" / "theme"
        self._colors_file = self._theme_dir / "colors.toml"

        # Defaults (Nord-like / Omarchy dark fallback)
        self._mode = "dark"
        self._accent = "#81a1c1"
        self._background = "#2e3440"
        self._dark_background = "#222730"
        self._darker_background = "#191c23"
        self._lighter_background = "#3b4252"
        self._foreground = "#d8dee9"
        self._dark_foreground = "#667080"
        self._light_foreground = "#adb5c4"
        self._muted = "#4c566a"
        self._selection = "#434c5e"
        self._red = "#bf616a"
        self._green = "#a3be8c"
        self._yellow = "#ebcb8b"
        self._orange = "#d5967a"
        self._blue = "#81a1c1"
        self._cyan = "#88c0d0"
        self._magenta = "#b48ead"

        self.load_theme()

        # Set up watcher
        self._watcher = QFileSystemWatcher(self)
        self._setup_watcher()

    def _setup_watcher(self):
        paths_to_watch = []
        if self._colors_file.exists():
            paths_to_watch.append(str(self._colors_file))
        if self._theme_dir.exists():
            paths_to_watch.append(str(self._theme_dir))

        if paths_to_watch:
            self._watcher.addPaths(paths_to_watch)
            self._watcher.fileChanged.connect(self._on_file_changed)
            self._watcher.directoryChanged.connect(self._on_dir_changed)

    @Slot(str)
    def _on_file_changed(self, path):
        self.load_theme()
        # Re-add file if it was replaced/atomic-renamed
        if self._colors_file.exists() and str(self._colors_file) not in self._watcher.files():
            self._watcher.addPath(str(self._colors_file))

    @Slot(str)
    def _on_dir_changed(self, path):
        self.load_theme()
        if self._colors_file.exists() and str(self._colors_file) not in self._watcher.files():
            self._watcher.addPath(str(self._colors_file))

    def load_theme(self):
        if not self._colors_file.is_file():
            return

        try:
            with open(self._colors_file, "rb") as f:
                data = tomllib.load(f)

            self._mode = data.get("mode", self._mode)
            self._accent = data.get("accent", self._accent)
            self._background = data.get("background", self._background)
            self._dark_background = data.get("dark_background", self._dark_background)
            self._darker_background = data.get("darker_background", self._darker_background)
            self._lighter_background = data.get("lighter_background", self._lighter_background)
            self._foreground = data.get("foreground", self._foreground)
            self._dark_foreground = data.get("dark_foreground", self._dark_foreground)
            self._light_foreground = data.get("light_foreground", self._light_foreground)
            self._muted = data.get("muted", self._muted)
            self._selection = data.get("selection", self._selection)
            self._red = data.get("red", self._red)
            self._green = data.get("green", self._green)
            self._yellow = data.get("yellow", self._yellow)
            self._orange = data.get("orange", self._orange)
            self._blue = data.get("blue", self._blue)
            self._cyan = data.get("cyan", self._cyan)
            self._magenta = data.get("magenta", self._magenta)

            self.themeChanged.emit()
        except Exception as e:
            print(f"Error reading Omarchy theme: {e}")

    # Properties exposed to QML
    @Property(str, notify=themeChanged)
    def mode(self) -> str:
        return self._mode

    @Property(QColor, notify=themeChanged)
    def accent(self) -> QColor:
        return QColor(self._accent)

    @Property(QColor, notify=themeChanged)
    def background(self) -> QColor:
        return QColor(self._background)

    @Property(QColor, notify=themeChanged)
    def darkBackground(self) -> QColor:
        return QColor(self._dark_background)

    @Property(QColor, notify=themeChanged)
    def darkerBackground(self) -> QColor:
        return QColor(self._darker_background)

    @Property(QColor, notify=themeChanged)
    def lighterBackground(self) -> QColor:
        return QColor(self._lighter_background)

    @Property(QColor, notify=themeChanged)
    def foreground(self) -> QColor:
        return QColor(self._foreground)

    @Property(QColor, notify=themeChanged)
    def darkForeground(self) -> QColor:
        return QColor(self._dark_foreground)

    @Property(QColor, notify=themeChanged)
    def lightForeground(self) -> QColor:
        return QColor(self._light_foreground)

    @Property(QColor, notify=themeChanged)
    def muted(self) -> QColor:
        return QColor(self._muted)

    @Property(QColor, notify=themeChanged)
    def selection(self) -> QColor:
        return QColor(self._selection)

    @Property(QColor, notify=themeChanged)
    def red(self) -> QColor:
        return QColor(self._red)

    @Property(QColor, notify=themeChanged)
    def green(self) -> QColor:
        return QColor(self._green)

    @Property(QColor, notify=themeChanged)
    def yellow(self) -> QColor:
        return QColor(self._yellow)

    @Property(QColor, notify=themeChanged)
    def orange(self) -> QColor:
        return QColor(self._orange)

    @Property(QColor, notify=themeChanged)
    def blue(self) -> QColor:
        return QColor(self._blue)

    @Property(QColor, notify=themeChanged)
    def cyan(self) -> QColor:
        return QColor(self._cyan)

    @Property(QColor, notify=themeChanged)
    def magenta(self) -> QColor:
        return QColor(self._magenta)
