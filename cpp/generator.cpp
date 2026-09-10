#include "generator.h"

#include <QProcess>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <algorithm>

static int popcountMask(int avail)
{
    return __builtin_popcount(static_cast<unsigned>(avail));
}

int SudokuEngine::solveCount(QVector<int> &board, int limit)
{
    int rows[9] = {};
    int cols[9] = {};
    int boxes[9] = {};

    for (int idx = 0; idx < 81; ++idx) {
        const int val = board[idx];
        if (val != 0) {
            const int r = idx / 9;
            const int c = idx % 9;
            const int mask = 1 << val;
            rows[r] |= mask;
            cols[c] |= mask;
            boxes[boxIndex(r, c)] |= mask;
        }
    }

    auto backtrack = [&](auto &&self, int remaining) -> int {
        int minCount = 10;
        int bestPos = -1;
        int bestMask = 0;

        for (int idx = 0; idx < 81; ++idx) {
            if (board[idx] == 0) {
                const int r = idx / 9;
                const int c = idx % 9;
                const int used = rows[r] | cols[c] | boxes[boxIndex(r, c)];
                const int avail = 0x3FE & (~used);
                const int cnt = popcountMask(avail);
                if (cnt == 0)
                    return 0;
                if (cnt < minCount) {
                    minCount = cnt;
                    bestPos = idx;
                    bestMask = avail;
                    if (cnt == 1)
                        break;
                }
            }
        }

        if (bestPos == -1)
            return 1;

        const int r = bestPos / 9;
        const int c = bestPos % 9;
        const int b = boxIndex(r, c);
        int found = 0;

        for (int val = 1; val <= 9; ++val) {
            const int valMask = 1 << val;
            if (bestMask & valMask) {
                board[bestPos] = val;
                rows[r] |= valMask;
                cols[c] |= valMask;
                boxes[b] |= valMask;

                found += self(self, remaining - found);

                board[bestPos] = 0;
                const int clearMask = ~valMask;
                rows[r] &= clearMask;
                cols[c] &= clearMask;
                boxes[b] &= clearMask;

                if (found >= remaining)
                    break;
            }
        }
        return found;
    };

    return backtrack(backtrack, limit);
}

QVector<int> SudokuEngine::generateSolvedBoard()
{
    int rows[9] = {};
    int cols[9] = {};
    int boxes[9] = {};
    QVector<int> board(81, 0);

    auto setVal = [&](int r, int c, int val) {
        board[r * 9 + c] = val;
        const int mask = 1 << val;
        rows[r] |= mask;
        cols[c] |= mask;
        boxes[boxIndex(r, c)] |= mask;
    };

    auto clearVal = [&](int r, int c, int val) {
        board[r * 9 + c] = 0;
        const int mask = ~(1 << val);
        rows[r] &= mask;
        cols[c] &= mask;
        boxes[boxIndex(r, c)] &= mask;
    };

    for (int b : {0, 4, 8}) {
        const int br = (b / 3) * 3;
        const int bc = (b % 3) * 3;
        QVector<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9};
        auto *rng = QRandomGenerator::global();
        for (int i = nums.size() - 1; i > 0; --i)
            std::swap(nums[i], nums[rng->bounded(i + 1)]);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                setVal(br + i, bc + j, nums[i * 3 + j]);
    }

    auto fillCells = [&](auto &&self) -> bool {
        int minCount = 10;
        int bestPos = -1;
        QVector<int> bestAvail;

        for (int idx = 0; idx < 81; ++idx) {
            if (board[idx] == 0) {
                const int r = idx / 9;
                const int c = idx % 9;
                const int used = rows[r] | cols[c] | boxes[boxIndex(r, c)];
                QVector<int> avail;
                for (int n = 1; n <= 9; ++n) {
                    if (!(used & (1 << n)))
                        avail.append(n);
                }
                if (avail.isEmpty())
                    return false;
                if (avail.size() < minCount) {
                    minCount = avail.size();
                    bestPos = idx;
                    bestAvail = avail;
                    if (minCount == 1)
                        break;
                }
            }
        }

        if (bestPos == -1)
            return true;

        const int r = bestPos / 9;
        const int c = bestPos % 9;
        auto *rng = QRandomGenerator::global();
        for (int i = bestAvail.size() - 1; i > 0; --i)
            std::swap(bestAvail[i], bestAvail[rng->bounded(i + 1)]);

        for (int n : bestAvail) {
            setVal(r, c, n);
            if (self(self))
                return true;
            clearVal(r, c, n);
        }
        return false;
    };

    fillCells(fillCells);
    return board;
}

QPair<QVector<int>, QVector<int>> SudokuEngine::generatePuzzle(const Difficulty &difficulty)
{
    // Optional qqwing fallback
    if (!QStandardPaths::findExecutable(QStringLiteral("qqwing")).isEmpty()) {
        QProcess proc;
        proc.start(QStringLiteral("qqwing"),
                   {QStringLiteral("--generate"), QStringLiteral("--solution"),
                    QStringLiteral("--one-line"), QStringLiteral("--difficulty"), difficulty.key});
        if (proc.waitForFinished(2000) && proc.exitCode() == 0) {
            const QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            if (lines.size() >= 2) {
                QVector<int> puzzle;
                QVector<int> solution;
                puzzle.reserve(81);
                solution.reserve(81);
                for (QChar ch : lines[0].trimmed()) {
                    if (ch.isDigit())
                        puzzle.append(ch.digitValue());
                    else
                        puzzle.append(0);
                }
                for (QChar ch : lines[1].trimmed()) {
                    if (ch.isDigit())
                        solution.append(ch.digitValue());
                    else
                        solution.append(0);
                }
                if (puzzle.size() == 81 && solution.size() == 81)
                    return {puzzle, solution};
            }
        }
    }

    QVector<int> solution = generateSolvedBoard();
    QVector<int> board = solution;
    const int target = difficulty.targetClues;

    QVector<QPair<int, int>> pairs;
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 9; ++c) {
            const int pos1 = r * 9 + c;
            const int pos2 = (8 - r) * 9 + (8 - c);
            if (pos1 <= pos2)
                pairs.append({pos1, pos2});
        }
    }

    auto *rng = QRandomGenerator::global();
    for (int i = pairs.size() - 1; i > 0; --i)
        std::swap(pairs[i], pairs[rng->bounded(i + 1)]);

    int currentClues = 81;
    for (const auto &pair : pairs) {
        if (currentClues <= target)
            break;
        const int pos1 = pair.first;
        const int pos2 = pair.second;
        const int val1 = board[pos1];
        const int val2 = board[pos2];
        board[pos1] = 0;
        board[pos2] = 0;

        QVector<int> probe = board;
        if (solveCount(probe, 2) == 1) {
            currentClues -= (pos1 != pos2 ? 2 : 1);
        } else {
            board[pos1] = val1;
            board[pos2] = val2;
        }
    }

    if (currentClues > target) {
        QVector<int> singles;
        for (int i = 0; i < 81; ++i) {
            if (board[i] != 0)
                singles.append(i);
        }
        for (int i = singles.size() - 1; i > 0; --i)
            std::swap(singles[i], singles[rng->bounded(i + 1)]);

        for (int pos : singles) {
            if (currentClues <= target)
                break;
            const int val = board[pos];
            board[pos] = 0;
            QVector<int> probe = board;
            if (solveCount(probe, 2) == 1) {
                --currentClues;
            } else {
                board[pos] = val;
            }
        }
    }

    return {board, solution};
}
