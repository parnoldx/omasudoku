#include "difficulty.h"
#include "game.h"
#include "generator.h"
#include "storage.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class TestSudokuGame : public QObject {
    Q_OBJECT
private slots:
    void difficulty_storage_keys();
    void difficulty_labels();
    void highscore_bucket_isolation();
    void broken_series_no_highscore();
    void scoring_value_times_factor();
    void completion_bonus();
    void factor_floor();
    void digit_highlight_on_filled_or_clue();
    void engine_generates_unique();
};

void TestSudokuGame::difficulty_storage_keys()
{
    QCOMPARE(Difficulty::fromKey(QStringLiteral("simple")).label, QStringLiteral("Easy"));
    QCOMPARE(Difficulty::fromKey(QStringLiteral("simple")).factor, 28);
    QCOMPARE(Difficulty::fromKey(QStringLiteral("easy")).label, QStringLiteral("Medium"));
    QCOMPARE(Difficulty::fromKey(QStringLiteral("easy")).factor, 56);
    QCOMPARE(Difficulty::fromKey(QStringLiteral("intermediate")).label, QStringLiteral("Hard"));
    QCOMPARE(Difficulty::fromKey(QStringLiteral("intermediate")).factor, 112);
    QCOMPARE(Difficulty::fromKey(QStringLiteral("expert")).label, QStringLiteral("Master"));
    QCOMPARE(Difficulty::fromKey(QStringLiteral("expert")).factor, 156);
    // Medium key must NOT become Easy
    QCOMPARE(Difficulty::fromKey(QStringLiteral("easy")).key, QStringLiteral("easy"));
}

void TestSudokuGame::difficulty_labels()
{
    QCOMPARE(Difficulty::fromKey(QStringLiteral("Easy")).key, QStringLiteral("simple"));
    QCOMPARE(Difficulty::fromKey(QStringLiteral("Medium")).key, QStringLiteral("easy"));
    QCOMPARE(Difficulty::fromKey(QStringLiteral("Hard")).key, QStringLiteral("intermediate"));
    QCOMPARE(Difficulty::fromKey(QStringLiteral("Master")).key, QStringLiteral("expert"));
}

void TestSudokuGame::highscore_bucket_isolation()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    StorageManager storage(dir.path());

    const QStringList keys{QStringLiteral("simple"), QStringLiteral("easy"),
                           QStringLiteral("intermediate"), QStringLiteral("expert")};
    for (const QString &key : keys) {
        SudokuGame game(&storage);
        game.startNewGame(key);
        game.boardRaw() = game.solutionRaw();
        game.setPointsRaw(1000 + key[0].unicode());
        game.setFailsRaw(0);
        game.onWon();
        QCOMPARE(storage.getHighscore(key), game.points());
        QCOMPARE(game.getHighscoreFor(key), game.points());
    }
}

void TestSudokuGame::broken_series_no_highscore()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    StorageManager storage(dir.path());
    SudokuGame game(&storage);
    game.startNewGame(QStringLiteral("intermediate"));
    game.boardRaw() = game.solutionRaw();
    game.setPointsRaw(99999);
    game.setFailsRaw(4);
    game.onWon();
    QCOMPARE(storage.getHighscore(QStringLiteral("intermediate")), 0);
}

void TestSudokuGame::scoring_value_times_factor()
{
    const QList<QPair<QString, int>> cases{
        {QStringLiteral("simple"), 28},
        {QStringLiteral("easy"), 56},
        {QStringLiteral("intermediate"), 112},
        {QStringLiteral("expert"), 156},
    };
    for (const auto &c : cases) {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        StorageManager storage(dir.path());
        SudokuGame game(&storage);
        game.startNewGame(c.first);
        QCOMPARE(game.factor(), c.second);
        int idx = -1;
        for (int i = 0; i < 81; ++i) {
            if (game.boardRaw()[i] == 0) {
                idx = i;
                break;
            }
        }
        QVERIFY(idx >= 0);
        game.selectCell(idx / 9, idx % 9);
        const int val = game.solutionRaw()[idx];
        game.enterNumber(val);
        QVERIFY(!game.historyRaw().isEmpty());
        QCOMPARE(game.historyRaw().last().pointsAdded, val * c.second);
        QVERIFY(game.points() >= val * c.second);
    }
}

void TestSudokuGame::completion_bonus()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    StorageManager storage(dir.path());
    SudokuGame game(&storage);
    game.startNewGame(QStringLiteral("easy"));
    QCOMPARE(game.factor(), 56);
    const int factor = game.factor();
    const int bonus = static_cast<int>(11.25 * factor);
    QCOMPARE(bonus, 630);

    QVector<int> solution = game.solutionRaw();
    QVector<int> board = solution;
    const int target = 8;
    board[target] = 0;
    board[1 * 9 + 8] = 0;
    board[1 * 9 + 6] = 0;
    game.boardRaw() = board;
    for (int i = 0; i < 81; ++i)
        game.initialCluesRaw()[i] = board[i] != 0;
    game.setPointsRaw(0);
    game.historyRaw().clear();

    game.selectCell(0, 8);
    const int val = solution[target];
    game.enterNumber(val);
    QCOMPARE(game.points(), val * factor + bonus);
    QCOMPARE(game.boardRaw()[1 * 9 + 8], 0);
}

void TestSudokuGame::factor_floor()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    StorageManager storage(dir.path());
    SudokuGame game(&storage);
    game.startNewGame(QStringLiteral("simple"));
    game.setFactorRaw(2);
    game.setTimeRaw(33);
    game.onSecondTick();
    QCOMPARE(game.factor(), 1);
    for (int i = 0; i < 34; ++i)
        game.onSecondTick();
    QCOMPARE(game.factor(), 1);
}

void TestSudokuGame::digit_highlight_on_filled_or_clue()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    StorageManager storage(dir.path());
    SudokuGame game(&storage);
    game.startNewGame(QStringLiteral("simple"));

    int idx = -1;
    for (int i = 0; i < 81; ++i) {
        if (game.initialCluesRaw()[i]) {
            idx = i;
            break;
        }
    }
    QVERIFY(idx >= 0);
    game.selectCell(idx / 9, idx % 9);
    const int clueVal = game.boardRaw()[idx];
    QCOMPARE(game.highlightNum(), clueVal);

    const int other = (clueVal % 9) + 1;
    game.enterNumber(other);
    QCOMPARE(game.highlightNum(), other);
    QCOMPARE(game.boardRaw()[idx], clueVal);

    int empty = -1;
    for (int i = 0; i < 81; ++i) {
        if (game.boardRaw()[i] == 0) {
            empty = i;
            break;
        }
    }
    QVERIFY(empty >= 0);
    game.selectCell(empty / 9, empty % 9);
    const int wrong = (game.solutionRaw()[empty] % 9) + 1;
    const int failsBefore = game.fails();
    game.enterNumber(wrong);
    QCOMPARE(game.highlightNum(), wrong);
    QCOMPARE(game.fails(), failsBefore + 1);
}

void TestSudokuGame::engine_generates_unique()
{
    QVector<int> board = SudokuEngine::generateSolvedBoard();
    QCOMPARE(board.size(), 81);
    QVector<int> probe = board;
    QCOMPARE(SudokuEngine::solveCount(probe, 2), 1);

    auto puzzle = SudokuEngine::generatePuzzle(Difficulty::easy());
    QCOMPARE(puzzle.first.size(), 81);
    QCOMPARE(puzzle.second.size(), 81);
    QVector<int> p = puzzle.first;
    QCOMPARE(SudokuEngine::solveCount(p, 2), 1);
}

QTEST_MAIN(TestSudokuGame)
#include "test_game.moc"
