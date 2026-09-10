import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 820
    height: 800
    minimumWidth: 500
    minimumHeight: 560
    title: "Sudoku"
    color: theme.background

    // Current view state: "welcome", "game", "win"
    property string currentView: "welcome"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header Bar
        HeaderBar {
            id: headerBar
            Layout.fillWidth: true
            onGoHome: {
                if (game.inGame && !game.isFinished()) {
                    game.save_current_state();
                }
                window.currentView = "welcome";
            }
            onNewGame: {
                window.currentView = "welcome";
            }
        }

        // View Stack
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Welcome View
            WelcomeView {
                id: welcomeView
                anchors.fill: parent
                visible: window.currentView === "welcome"
                onStartGame: function(diffKey) {
                    game.startNewGame(diffKey);
                    window.currentView = "game";
                    boardView.forceActiveFocus();
                }
                onResumeGame: {
                    game.resumeGame();
                    window.currentView = "game";
                    boardView.forceActiveFocus();
                }
            }

            // Game View (Board + Bottom Keypad)
            Item {
                id: gameContainer
                anchors.fill: parent
                visible: window.currentView === "game"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    BoardView {
                        id: boardView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    KeypadBar {
                        id: keypadBar
                        Layout.fillWidth: true
                    }
                }
            }

            // Win View
            WinView {
                id: winView
                anchors.fill: parent
                visible: window.currentView === "win"
                onPlayAgain: {
                    game.startNewGame(game.difficultyKey);
                    window.currentView = "game";
                    boardView.forceActiveFocus();
                }
                onReturnToMenu: {
                    window.currentView = "welcome";
                }
            }
        }
    }

    // Connect game win signal
    Connections {
        target: game

        function onGameWon(points, fails, highscore, isNewRecord) {
            winView.finalPoints = points;
            winView.finalFails = fails;
            winView.currentHighscore = highscore;
            winView.isRecord = isNewRecord;
            winView.finalTime = game.formattedTime;
            window.currentView = "win";
        }
    }

    Component.onCompleted: {
        // If there's an unfinished game, prompt or show welcome with resume prominent
    }
}
