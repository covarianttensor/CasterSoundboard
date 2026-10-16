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
#include "OscServerController.h"
#include "AudioRoutingController.h"

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
    QCommandLineOption drawerOption("open-drawer", "Open tile settings drawer on startup for testing.");
    QCommandLineOption drawerIndexOption("drawer-index", "Tile index to open in drawer on startup.", "index", "0");
    QCommandLineOption viewOption("view", "Initial view index (0=Soundboards, 1=Audio, 2=OSC, 3=Preferences).", "index", "0");
    QCommandLineOption autoStartOscOption("auto-start-osc", "Automatically start OSC server on startup.");
    QCommandLineOption widthOption("window-width", "Initial window width in pixels.", "width");
    QCommandLineOption heightOption("window-height", "Initial window height in pixels.", "height");
    QCommandLineOption hotkeyPromptOption("open-hotkey-prompt", "Open hotkey prompt dialog on startup for testing.");
    QCommandLineOption testSeekMsOption("test-seek-ms", "Set position in ms on tile 0 for visual verification.", "ms");
    QCommandLineOption scrollCueOption("scroll-cue", "Scroll to cue region in drawer on startup for testing.");
    QCommandLineOption deleteTabPromptOption("open-delete-tab-prompt", "Open delete tab confirmation dialog on startup for testing.");
    parser.addOption(screenshotOption);
    parser.addOption(drawerOption);
    parser.addOption(drawerIndexOption);
    parser.addOption(viewOption);
    parser.addOption(autoStartOscOption);
    parser.addOption(widthOption);
    parser.addOption(heightOption);
    parser.addOption(hotkeyPromptOption);
    parser.addOption(testSeekMsOption);
    parser.addOption(scrollCueOption);
    parser.addOption(deleteTabPromptOption);
    parser.addPositionalArgument("project", "Project file (.caster) to open on launch.", "[project]");
    parser.process(app);

    // Register CasterCore C++ models & controllers with QML engine
    qmlRegisterType<SoundboardProjectController>("CasterCore", 1, 0, "SoundboardProjectController");
    qmlRegisterType<HotkeyManager>("CasterCore", 1, 0, "HotkeyManager");
    qmlRegisterType<SoundboardTabModel>("CasterCore", 1, 0, "SoundboardTabModel");
    qmlRegisterType<SoundboardGridModel>("CasterCore", 1, 0, "SoundboardGridModel");
    qmlRegisterType<SoundTileController>("CasterCore", 1, 0, "SoundTileController");
    qmlRegisterType<OscServerController>("CasterCore", 1, 0, "OscServerController");
    qmlRegisterType<AudioRoutingController>("CasterCore", 1, 0, "AudioRoutingController");

    SoundboardProjectController projectController;
    HotkeyManager hotkeyManager;
    hotkeyManager.monitorProject(&projectController);

    OscServerController oscController;
    oscController.setProjectController(&projectController);
    if (parser.isSet(autoStartOscOption) || oscController.autoStart()) {
        oscController.startServer();
    }

    AudioRoutingController audioRoutingController;
    audioRoutingController.setProjectController(&projectController);

    // Load initial project from CLI or create a fresh default project
    const QStringList posArgs = parser.positionalArguments();
    if (!posArgs.isEmpty()) {
        projectController.loadProject(posArgs.first());
    } else {
        projectController.newProject();
    }

    if (parser.isSet(widthOption)) {
        projectController.setWindowWidth(parser.value(widthOption).toInt());
    }
    if (parser.isSet(heightOption)) {
        projectController.setWindowHeight(parser.value(heightOption).toInt());
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("projectController", &projectController);
    engine.rootContext()->setContextProperty("hotkeyManager", &hotkeyManager);
    engine.rootContext()->setContextProperty("oscController", &oscController);
    engine.rootContext()->setContextProperty("audioRoutingController", &audioRoutingController);
    engine.rootContext()->setContextProperty("openDrawerOnStart", parser.isSet(drawerOption) || parser.isSet(hotkeyPromptOption) || parser.isSet(scrollCueOption));
    engine.rootContext()->setContextProperty("drawerTileIndex", parser.value(drawerIndexOption).toInt());
    engine.rootContext()->setContextProperty("initialViewIndex", parser.value(viewOption).toInt());
    engine.rootContext()->setContextProperty("openHotkeyPromptOnStart", parser.isSet(hotkeyPromptOption));
    engine.rootContext()->setContextProperty("scrollToCueOnStart", parser.isSet(scrollCueOption));
    engine.rootContext()->setContextProperty("openDeleteTabPromptOnStart", parser.isSet(deleteTabPromptOption));

    // Install application-wide hotkey event filter
    GlobalHotkeyFilter hotkeyFilter(&hotkeyManager, &app);
    app.installEventFilter(&hotkeyFilter);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("CasterSoundboard", "Main");

    if (parser.isSet(testSeekMsOption)) {
        int seekMs = parser.value(testSeekMsOption).toInt();
        QTimer::singleShot(200, [&projectController, seekMs]() {
            if (projectController.tabModel()->count() > 0) {
                auto *grid = projectController.tabModel()->gridModelAt(0);
                if (grid && grid->count() > 0) {
                    auto *tile = grid->tileAt(0);
                    if (tile) {
                        tile->onDurationChanged(1200);
                        tile->onPositionChanged(seekMs);
                    }
                }
            }
        });
    }

    if (parser.isSet(screenshotOption)) {
        const QString outPath = parser.value(screenshotOption);
        int delay = (parser.isSet(testSeekMsOption) || parser.isSet(scrollCueOption)) ? 800 : 350;
        QTimer::singleShot(delay, [&engine, outPath, &app]() {
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
