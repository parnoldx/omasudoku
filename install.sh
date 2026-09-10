#!/bin/bash
set -e

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="$HOME/.local/bin"
APP_DESKTOP_DIR="$HOME/.local/share/applications"
ICON_DIR="$HOME/.local/share/icons/hicolor/scalable/apps"

mkdir -p "$BIN_DIR" "$APP_DESKTOP_DIR" "$ICON_DIR"

# 1. Create launcher in ~/.local/bin
cat << LAUNCHER > "$BIN_DIR/omarchy-sudoku"
#!/bin/bash
exec python3 "$APP_DIR/main.py" "\$@"
LAUNCHER
chmod +x "$BIN_DIR/omarchy-sudoku"

# 2. Install icon
cp "$APP_DIR/data/org.omarchy.sudoku.svg" "$ICON_DIR/org.omarchy.sudoku.svg"

# 3. Install desktop entry
cp "$APP_DIR/data/org.omarchy.sudoku.desktop" "$APP_DESKTOP_DIR/org.omarchy.sudoku.desktop"

# 4. Update desktop database if tool is present
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$APP_DESKTOP_DIR" || true
fi

echo "Successfully installed Omarchy Sudoku!"
echo "You can launch it with: omarchy-sudoku or from the Omarchy app launcher."
