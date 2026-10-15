#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QCommandLineParser>
#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <QDebug>

#include "SoundboardProjectController.h"
#include "HotkeyManager.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

class GlobalHotkeyFilter : public QObject
{
public:
    explicit GlobalHotkeyFilter(HotkeyManager *manager, QObject *parent = nullptr)
        : QObject(parent), m_manager(manager)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (m_manager && m_manager->handleKeyEvent(keyEvent)) {
                return true; // Hotkey consumed
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    HotkeyManager *m_manager{nullptr};
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("CasterSoundboard");
    app.setApplicationDisplayName("Caster Soundboard");
    app.setOrganizationName("JupiterBroadcasting");
    app.setOrganizationDomain("jupiterbroadcasting.com");
    app.setApplicationVersion("2.0.0");
    app.setWindowIcon(QIcon(":/qt/qml/CasterSoundboard/src/qml/icons/app.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Caster Soundboard - Touch-Friendly Soundboard for Broadcasters and Podcasters");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption screenshotOption(QStringList() << "s" << "screenshot", "Capture window screenshot to path and exit.", "path");
    parser.addOption(screenshotOption);
    parser.addPositionalArgument("project", "Project file (.caster) to open on launch.", "[project]");
    parser.process(app);

    // Register CasterCore C++ models & controllers with QML engine
    qmlRegisterType<SoundboardProjectController>("CasterCore", 1, 0, "SoundboardProjectController");
    qmlRegisterType<HotkeyManager>("CasterCore", 1, 0, "HotkeyManager");
    qmlRegisterType<SoundboardTabModel>("CasterCore", 1, 0, "SoundboardTabModel");
    qmlRegisterType<SoundboardGridModel>("CasterCore", 1, 0, "SoundboardGridModel");
    qmlRegisterType<SoundTileController>("CasterCore", 1, 0, "SoundTileController");

    SoundboardProjectController projectController;
    HotkeyManager hotkeyManager;
    hotkeyManager.monitorProject(&projectController);

    // Load initial project from CLI or create a fresh default project
    const QStringList posArgs = parser.positionalArguments();
    if (!posArgs.isEmpty()) {
        projectController.loadProject(posArgs.first());
    } else {
        projectController.newProject();
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("projectController", &projectController);
    engine.rootContext()->setContextProperty("hotkeyManager", &hotkeyManager);

    // Install application-wide hotkey event filter
    GlobalHotkeyFilter hotkeyFilter(&hotkeyManager, &app);
    app.installEventFilter(&hotkeyFilter);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("CasterSoundboard", "Main");

    if (parser.isSet(screenshotOption)) {
        const QString outPath = parser.value(screenshotOption);
        QTimer::singleShot(350, [&engine, outPath, &app]() {
            if (!engine.rootObjects().isEmpty()) {
                if (auto *window = qobject_cast<QQuickWindow*>(engine.rootObjects().first())) {
                    QImage img = window->grabWindow();
                    img.save(outPath);
                    qInfo() << "Screenshot saved to:" << outPath;
                }
            }
            app.quit();
        });
    }

    return app.exec();
}
