#pragma once

#include "difficulty.h"
#include "storage.h"

#include <QObject>
#include <QSet>
#include <QTimer>
#include <QVariantList>
#include <QVector>
#include <optional>

struct HistoryEntry {
    int idx = 0;
    int prevVal = 0;
    QSet<int> prevNotes;
    int pointsAdded = 0;
};

class SudokuGame : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList board READ board NOTIFY boardChanged)
    Q_PROPERTY(QVariantList initialClues READ initialClues NOTIFY boardChanged)
    Q_PROPERTY(QVariantList notes READ notes NOTIFY boardChanged)
    Q_PROPERTY(int selectedRow READ selectedRow NOTIFY selectionChanged)
    Q_PROPERTY(int selectedCol READ selectedCol NOTIFY selectionChanged)
    Q_PROPERTY(int highlightNum READ highlightNum NOTIFY selectionChanged)
    Q_PROPERTY(int points READ points NOTIFY scoreChanged)
    Q_PROPERTY(int factor READ factor NOTIFY factorChanged)
    Q_PROPERTY(int fails READ fails NOTIFY failsChanged)
    Q_PROPERTY(int time READ time NOTIFY timeChanged)
    Q_PROPERTY(QString formattedTime READ formattedTime NOTIFY timeChanged)
    Q_PROPERTY(QString difficultyKey READ difficultyKey NOTIFY gameStateChanged)
    Q_PROPERTY(QString difficultyLabel READ difficultyLabel NOTIFY gameStateChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY gameStateChanged)
    Q_PROPERTY(bool inGame READ inGame NOTIFY gameStateChanged)
    Q_PROPERTY(bool notesMode READ notesMode NOTIFY notesModeChanged)
    Q_PROPERTY(bool canResume READ canResume NOTIFY canResumeChanged)
    Q_PROPERTY(int highscore READ highscore NOTIFY gameStateChanged)

public:
    static constexpr int TIME_FACTOR_REDUCE = 34;
    static constexpr double POINTS_BLOCK_ROW = 11.25;

    explicit SudokuGame(StorageManager *storage = nullptr, QObject *parent = nullptr);
    ~SudokuGame() override;

    QVariantList board() const;
    QVariantList initialClues() const;
    QVariantList notes() const;
    int selectedRow() const { return m_selectedRow; }
    int selectedCol() const { return m_selectedCol; }
    int highlightNum() const { return m_highlightNum; }
    int points() const { return m_points; }
    int factor() const { return m_factor; }
    int fails() const { return m_fails; }
    int time() const { return m_time; }
    QString formattedTime() const;
    QString difficultyKey() const { return m_difficulty.key; }
    QString difficultyLabel() const { return m_difficulty.label; }
    bool isPaused() const { return m_isPaused; }
    bool inGame() const { return m_inGame; }
    bool notesMode() const { return m_notesMode; }
    bool canResume() const;
    int highscore() const;

    bool is_finished() const;

    // Exposed for tests / native access
    const QVector<int> &solutionRaw() const { return m_solution; }
    QVector<int> &boardRaw() { return m_board; }
    QVector<bool> &initialCluesRaw() { return m_initialClues; }
    QVector<HistoryEntry> &historyRaw() { return m_history; }
    void setFactorRaw(int f) { m_factor = f; }
    void setTimeRaw(int t) { m_time = t; }
    void setPointsRaw(int p) { m_points = p; }
    void setFailsRaw(int f) { m_fails = f; }
    void onSecondTick();
    void onWon();

public slots:
    void startNewGame(const QString &difficultyKey);
    void resumeGame();
    void pauseGame();
    void resumeTimer();
    void togglePause();
    void returnToMenu();
    void save_current_state();
    void selectCell(int row, int col);
    void moveSelection(int dRow, int dCol);
    void enterNumber(int num);
    void clearSelected();
    void toggleNotesMode();
    void undo();
    bool isFinished() const;
    int getHighscoreFor(const QString &diffKey) const;

signals:
    void boardChanged();
    void scoreChanged();
    void factorChanged();
    void failsChanged();
    void timeChanged();
    void selectionChanged();
    void notesModeChanged();
    void gameStateChanged();
    void canResumeChanged();
    void cellFlash(int row, int col, bool isCorrect);
    void rowCompleted(int row);
    void colCompleted(int col);
    void boxCompleted(int box);
    void gameWon(int points, int fails, int highscore, bool isNewRecord);

private:
    void updateHighlightNum();
    void clearPeerNotes(int row, int col, int num);
    void checkCompletions(int row, int col);

    StorageManager *m_storage = nullptr;
    bool m_ownsStorage = false;

    QVector<int> m_board;
    QVector<int> m_solution;
    QVector<bool> m_initialClues;
    QVector<QSet<int>> m_notes;

    int m_selectedRow = -1;
    int m_selectedCol = -1;
    int m_highlightNum = 0;

    Difficulty m_difficulty;
    int m_points = 0;
    int m_factor = 28;
    int m_time = 0;
    int m_fails = 0;
    bool m_isPaused = false;
    bool m_inGame = false;
    bool m_notesMode = false;

    QVector<HistoryEntry> m_history;
    QTimer m_timer;
};
