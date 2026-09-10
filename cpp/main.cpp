#include "game.h"
#include <QCoreApplication>
#include "theme.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QTimer>
#include <QQuickWindow>
#include <QImage>
#include <csignal>

static QString findQmlMain(const QString &appDir)
{
    const QStringList candidates = {
        appDir + QStringLiteral("/qml/Main.qml"),
        appDir + QStringLiteral("/../qml/Main.qml"),
        appDir + QStringLiteral("/../../qml/Main.qml"),
        QDir::cleanPath(appDir + QStringLiteral("/../share/omarchy-sudoku/qml/Main.qml")),
    };
    for (const QString &c : candidates) {
        if (QFileInfo::exists(c))
            return QFileInfo(c).absoluteFilePath();
    }
    // APPDIR env or project-relative from cwd
    const QByteArray appdirEnv = qgetenv("OMARCHY_SUDOKU_ROOT");
    if (!appdirEnv.isEmpty()) {
        const QString p = QString::fromLocal8Bit(appdirEnv) + QStringLiteral("/qml/Main.qml");
        if (QFileInfo::exists(p))
            return QFileInfo(p).absoluteFilePath();
    }
    return {};
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, SIG_DFL);

    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "wayland;xcb");

    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Omarchy"));
    app.setApplicationName(QStringLiteral("Sudoku"));
    app.setDesktopFileName(QStringLiteral("org.omarchy.sudoku"));

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString iconPath = [&]() {
        const QStringList icons = {
            appDir + QStringLiteral("/data/org.omarchy.sudoku.svg"),
            appDir + QStringLiteral("/../data/org.omarchy.sudoku.svg"),
            appDir + QStringLiteral("/../../data/org.omarchy.sudoku.svg"),
        };
        for (const QString &p : icons) {
            if (QFileInfo::exists(p))
                return QFileInfo(p).absoluteFilePath();
        }
        return QString();
    }();
    if (!iconPath.isEmpty())
        app.setWindowIcon(QIcon(iconPath));

    OmarchyTheme theme;
    SudokuGame game;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("theme"), &theme);
    engine.rootContext()->setContextProperty(QStringLiteral("game"), &game);

    QObject::connect(&app, &QGuiApplication::aboutToQuit, &game, [&game]() {
        if (game.inGame() && !game.is_finished()) {
            bool hasMoves = false;
            const QVariantList board = game.board();
            const QVariantList clues = game.initialClues();
            for (int i = 0; i < 81; ++i) {
                if (board[i].toInt() != 0 && !clues[i].toBool()) {
                    hasMoves = true;
                    break;
                }
            }
            if (hasMoves || game.time() > 10)
                game.save_current_state();
        }
    });

    const QString qmlFile = findQmlMain(appDir);
    if (qmlFile.isEmpty()) {
        qCritical("Error: Could not find qml/Main.qml near the binary.");
        return 1;
    }
    engine.load(QUrl::fromLocalFile(qmlFile));
    if (engine.rootObjects().isEmpty()) {
        qCritical("Error: Could not load QML user interface.");
        return 1;
    }

    // Docs screenshot: OMARCHY_SUDOKU_SCREENSHOT=/path/ingame.png starts a Medium
    // game, grabs the window, writes PNG, then exits.
    const QString shotPath = QString::fromLocal8Bit(qgetenv("OMARCHY_SUDOKU_SCREENSHOT"));
    if (!shotPath.isEmpty()) {
        QObject *root = engine.rootObjects().constFirst();
        root->setProperty("currentView", QStringLiteral("game"));
        game.startNewGame(QStringLiteral("easy"));
        QTimer::singleShot(900, &app, [root, shotPath]() {
            auto *win = qobject_cast<QQuickWindow *>(root);
            if (!win) {
                qCritical("Screenshot: root is not a QQuickWindow");
                QCoreApplication::exit(1);
                return;
            }
            const QImage img = win->grabWindow();
            if (img.isNull() || !img.save(shotPath)) {
                qCritical("Screenshot: failed to write %s", qPrintable(shotPath));
                QCoreApplication::exit(1);
                return;
            }
            qInfo("Wrote screenshot %s", qPrintable(shotPath));
            QCoreApplication::exit(0);
        });
    }

    return app.exec();
}
