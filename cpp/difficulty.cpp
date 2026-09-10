#include "difficulty.h"

const Difficulty &Difficulty::easy()
{
    static const Difficulty d{QStringLiteral("simple"), QStringLiteral("Easy"), 28, 40};
    return d;
}

const Difficulty &Difficulty::medium()
{
    static const Difficulty d{QStringLiteral("easy"), QStringLiteral("Medium"), 56, 34};
    return d;
}

const Difficulty &Difficulty::hard()
{
    static const Difficulty d{QStringLiteral("intermediate"), QStringLiteral("Hard"), 112, 29};
    return d;
}

const Difficulty &Difficulty::master()
{
    static const Difficulty d{QStringLiteral("expert"), QStringLiteral("Master"), 156, 25};
    return d;
}

const std::array<Difficulty, 4> &Difficulty::all()
{
    static const std::array<Difficulty, 4> diffs{easy(), medium(), hard(), master()};
    return diffs;
}

Difficulty Difficulty::fromKey(const QString &key)
{
    const QString raw = key.trimmed();
    if (raw.isEmpty())
        return easy();

    for (const auto &diff : all()) {
        if (raw == diff.key)
            return diff;
    }
    for (const auto &diff : all()) {
        if (raw.compare(diff.label, Qt::CaseInsensitive) == 0)
            return diff;
    }
    for (const auto &diff : all()) {
        if (raw.compare(diff.key, Qt::CaseInsensitive) == 0)
            return diff;
    }
    return easy();
}
