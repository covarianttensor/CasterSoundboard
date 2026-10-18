#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QCommandLineParser>
#include <QKeyEvent>
#include <QInputDevice>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <QDebug>
#include <csignal>

#include "SoundboardProjectController.h"
#include "HotkeyManager.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"
#include "OscServerController.h"
#include "AudioRoutingController.h"
#include "WebRemoteController.h"
#include "McpServerController.h"
#include "CasterVersion.h"
#include <QSocketNotifier>
#include <iostream>

static void handleSigInt(int)
{
    qInfo() << "\n[CasterSoundboard] Termination signal received. Exiting...";
    QCoreApplication::quit();
}

static bool isTextInputObject(QObject *obj)
{
    while (obj) {
        if (obj->inherits("QQuickTextInput") ||
            obj->inherits("QQuickTextEdit") ||
            obj->inherits("QLineEdit") ||
            obj->inherits("QTextEdit") ||
            obj->inherits("QPlainTextEdit") ||
            obj->inherits("QAbstractSpinBox")) {
            return true;
        }

        const QVariant cursorPosition = obj->property("cursorPosition");
        if (cursorPosition.isValid() && obj->property("selectedText").isValid()) {
            return true;
        }

        const QString className = QString::fromLatin1(obj->metaObject()->className());
        if (className.contains("TextField") || className.contains("TextArea") ||
            className.contains("TextInput") || className.contains("TextEdit")) {
            return true;
        }

        obj = obj->parent();
    }

    return false;
}

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

            // Ignore synthetic keystrokes originating from Caster's own virtual keyboard device
            if (const auto *dev = keyEvent->device()) {
                if (dev->name().contains("Caster Soundboard")) {
                    return true; // Ignore self-injected virtual keystrokes
                }
            }

            // If key capture is actively listening in PTT or shortcut configuration, bypass hotkey interception
            if (m_manager && m_manager->isCapturingKey()) {
                return QObject::eventFilter(watched, event);
            }

            // Never intercept typing when a text field or editor has active keyboard focus
            QObject *focusObj = QGuiApplication::focusObject();
            if (isTextInputObject(focusObj) || isTextInputObject(watched)) {
                return QObject::eventFilter(watched, event);
            }

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
    // Check for headless mode or absent display environment before initializing QGuiApplication
    bool headless = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--headless") == 0) {
            headless = true;
            break;
        }
    }
    const bool noDisplay = qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");
    if ((headless || noDisplay) && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        headless = true;
    }

    signal(SIGINT, handleSigInt);
    signal(SIGTERM, handleSigInt);

    QGuiApplication app(argc, argv);
    app.setApplicationName("CasterSoundboard");
    app.setApplicationDisplayName("Caster Soundboard");
    app.setOrganizationName("JupiterBroadcasting");
    app.setOrganizationDomain("jupiterbroadcasting.com");
    app.setApplicationVersion(CASTER_VERSION_STRING);
    app.setWindowIcon(QIcon(":/qt/qml/CasterSoundboard/src/qml/icons/app.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Caster Soundboard - Touch-Friendly Soundboard for Broadcasters and Podcasters");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption headlessOption("headless", "Run in headless mode without showing the GUI window.");
    QCommandLineOption autoStartWebOption("auto-start-web", "Automatically start Web Remote control server on startup.");
    QCommandLineOption webPortOption("web-port", "Inbound TCP port for Web Remote server (defaults to 5052).", "port", "5052");
    QCommandLineOption webUserOption("web-user", "Username for Web Remote authentication (defaults to 'caster').", "user", "caster");
    QCommandLineOption webPassOption("web-pass", "Password for Web Remote authentication (defaults to 'caster').", "password", "caster");
    QCommandLineOption screenshotOption(QStringList() << "s" << "screenshot", "Capture window screenshot to path and exit.", "path");
    QCommandLineOption drawerOption("open-drawer", "Open tile settings drawer on startup for testing.");
    QCommandLineOption drawerIndexOption("drawer-index", "Tile index to open in drawer on startup.", "index", "0");
    QCommandLineOption viewOption("view", "Initial view index (0=Soundboards, 1=Audio, 2=OSC, 3=Preferences, 4=Documentation, 5=About).", "index", "0");
    QCommandLineOption autoStartOscOption("auto-start-osc", "Automatically start OSC server on startup.");
    QCommandLineOption fullscreenOption(QStringList() << "f" << "fullscreen", "Launch application in fullscreen mode.");
    QCommandLineOption widthOption("window-width", "Initial window width in pixels.", "width");
    QCommandLineOption heightOption("window-height", "Initial window height in pixels.", "height");
    QCommandLineOption hotkeyPromptOption("open-hotkey-prompt", "Open hotkey prompt dialog on startup for testing.");
    QCommandLineOption testSeekMsOption("test-seek-ms", "Set position in ms on tile 0 for visual verification.", "ms");
    QCommandLineOption scrollCueOption("scroll-cue", "Scroll to cue region in drawer on startup for testing.");
    QCommandLineOption deleteTabPromptOption("open-delete-tab-prompt", "Open delete tab confirmation dialog on startup for testing.");
    QCommandLineOption tabHotkeyPromptOption("open-tab-hotkey-prompt", "Open tab hotkey prompt dialog on startup for testing.");
    QCommandLineOption actionHotkeyPromptOption("open-action-hotkey-prompt", "Open action hotkey prompt dialog on startup for testing.", "action");
    QCommandLineOption testRoutingActiveOption("test-routing-active", "Simulate active virtual audio routing for visual testing.");
    QCommandLineOption scrollBottomOption("scroll-bottom", "Scroll active view to bottom for testing.");
    QCommandLineOption openAccordionOption("open-accordion", "Expand advanced accordion on startup.");
    QCommandLineOption testFocusMicOption("test-focus-mic", "Focus microphone selector on startup for visual verification.");
    QCommandLineOption testFocusVirtOption("test-focus-virt", "Focus virtual mic text field on startup for visual verification.");
    QCommandLineOption legacyImportPromptOption("open-legacy-import-prompt", "Open legacy import prompt dialog on startup for testing.");
    QCommandLineOption pttPromptOption("open-ptt-prompt", "Open PTT dialog on startup for testing.");
    QCommandLineOption importDirPromptOption("open-import-dir-prompt", "Open directory import dialog on startup for testing.", "path");
    QCommandLineOption autoStartMcpOption("auto-start-mcp", "Automatically start Model Context Protocol (MCP) AI server on startup.");
    QCommandLineOption mcpPortOption("mcp-port", "Inbound TCP port for MCP server (defaults to 5055).", "port", "5055");
    QCommandLineOption mcpStdioOption("mcp-stdio", "Run MCP server in stdio mode (JSON-RPC 2.0 over stdin/stdout) for AI assistants.");
    parser.addOption(headlessOption);
    parser.addOption(autoStartWebOption);
    parser.addOption(webPortOption);
    parser.addOption(webUserOption);
    parser.addOption(webPassOption);
    parser.addOption(autoStartMcpOption);
    parser.addOption(mcpPortOption);
    parser.addOption(mcpStdioOption);
    parser.addOption(screenshotOption);
    parser.addOption(drawerOption);
    parser.addOption(drawerIndexOption);
    parser.addOption(viewOption);
    parser.addOption(autoStartOscOption);
    parser.addOption(fullscreenOption);
    parser.addOption(widthOption);
    parser.addOption(heightOption);
    parser.addOption(hotkeyPromptOption);
    parser.addOption(testSeekMsOption);
    parser.addOption(scrollCueOption);
    parser.addOption(deleteTabPromptOption);
    parser.addOption(tabHotkeyPromptOption);
    parser.addOption(actionHotkeyPromptOption);
    parser.addOption(testRoutingActiveOption);
    parser.addOption(scrollBottomOption);
    parser.addOption(openAccordionOption);
    parser.addOption(testFocusMicOption);
    parser.addOption(testFocusVirtOption);
    parser.addOption(legacyImportPromptOption);
    parser.addOption(pttPromptOption);
    parser.addOption(importDirPromptOption);
    parser.addPositionalArgument("project", "Project file (.caster) to open on launch.", "[project]");
    parser.process(app);

    if (parser.isSet(headlessOption)) {
        headless = true;
    }

    // Register CasterCore C++ models & controllers with QML engine
    qmlRegisterType<SoundboardProjectController>("CasterCore", 1, 0, "SoundboardProjectController");
    qmlRegisterType<HotkeyManager>("CasterCore", 1, 0, "HotkeyManager");
    qmlRegisterType<SoundboardTabModel>("CasterCore", 1, 0, "SoundboardTabModel");
    qmlRegisterType<SoundboardGridModel>("CasterCore", 1, 0, "SoundboardGridModel");
    qmlRegisterType<SoundTileController>("CasterCore", 1, 0, "SoundTileController");
    qmlRegisterType<OscServerController>("CasterCore", 1, 0, "OscServerController");
    qmlRegisterType<AudioRoutingController>("CasterCore", 1, 0, "AudioRoutingController");
    qmlRegisterType<WebRemoteController>("CasterCore", 1, 0, "WebRemoteController");
    qmlRegisterType<McpServerController>("CasterCore", 1, 0, "McpServerController");

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
    if (parser.isSet(testRoutingActiveOption)) {
        audioRoutingController.setRoutingActiveForTesting(true);
    }

    WebRemoteController webRemoteController;
    webRemoteController.setProjectController(&projectController);
    if (parser.isSet(webPortOption)) {
        webRemoteController.setPort(parser.value(webPortOption).toInt());
    }
    if (parser.isSet(webUserOption)) {
        webRemoteController.setUsername(parser.value(webUserOption));
    }
    if (parser.isSet(webPassOption)) {
        webRemoteController.setPassword(parser.value(webPassOption));
    }
    if (parser.isSet(autoStartWebOption) || webRemoteController.autoStart()) {
        webRemoteController.startServer();
    }

    McpServerController mcpServerController;
    mcpServerController.setProjectController(&projectController);
    mcpServerController.setAudioRoutingController(&audioRoutingController);
    if (parser.isSet(mcpPortOption)) {
        mcpServerController.setPort(parser.value(mcpPortOption).toInt());
    }
    if (parser.isSet(autoStartMcpOption) || mcpServerController.autoStart()) {
        mcpServerController.startServer();
    }

    if (parser.isSet(mcpStdioOption)) {
        auto *notifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, &app);
        QObject::connect(notifier, &QSocketNotifier::activated, [&mcpServerController]() {
            std::string line;
            if (std::getline(std::cin, line)) {
                mcpServerController.processStdioLine(QByteArray::fromStdString(line));
            }
        });
    }

    // Load initial project from CLI or create a fresh default project
    QString legacyStartupFile;
    const QStringList posArgs = parser.positionalArguments();
    if (!posArgs.isEmpty()) {
        const QString startupPath = posArgs.first();
        if (projectController.isLegacyFile(startupPath) && !headless) {
            legacyStartupFile = startupPath;
            projectController.newProject();
        } else {
            projectController.loadProject(startupPath);
        }
    } else {
        projectController.newProject();
    }

    if (parser.isSet(widthOption)) {
        projectController.setWindowWidth(parser.value(widthOption).toInt());
    }
    if (parser.isSet(heightOption)) {
        projectController.setWindowHeight(parser.value(heightOption).toInt());
    }
    if (parser.isSet(fullscreenOption)) {
        projectController.setIsFullScreen(true);
    }

    // Headless mode execution branch: runs without loading QML GUI window
    if (headless) {
        app.setQuitOnLastWindowClosed(false);

        if (!oscController.isRunning() && parser.isSet(autoStartOscOption)) {
            oscController.startServer();
        }
        if (!webRemoteController.isRunning()) {
            webRemoteController.startServer();
        }

        qInfo().noquote() << "=========================================================";
        qInfo().noquote() << QString("Caster Soundboard v%1 (Headless Mode Active)").arg(CASTER_VERSION_STRING);
        qInfo().noquote() << "---------------------------------------------------------";
        qInfo().noquote() << "• Project:    " << (projectController.filePath().isEmpty() ? "New Default Project" : projectController.filePath());
        qInfo().noquote() << "• OSC Server: " << (oscController.isRunning() ? QString("Running on UDP port %1").arg(oscController.inboundPort()) : "Disabled");
        qInfo().noquote() << "• Web Remote: " << (webRemoteController.isRunning() ? QString("Running on %1 (User: %2)").arg(webRemoteController.localUrl()).arg(webRemoteController.username()) : "Disabled");
        for (const QString &url : webRemoteController.availableUrls()) {
            qInfo().noquote() << "              " << url;
        }
        qInfo().noquote() << "• MCP Server: " << (mcpServerController.isRunning() ? QString("Running on %1 (Port %2)").arg(mcpServerController.sseUrl()).arg(mcpServerController.port()) : "Disabled");
        qInfo().noquote() << "Press Ctrl+C to terminate.";
        qInfo().noquote() << "=========================================================";

        return app.exec();
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("projectController", &projectController);
    engine.rootContext()->setContextProperty("hotkeyManager", &hotkeyManager);
    engine.rootContext()->setContextProperty("oscController", &oscController);
    engine.rootContext()->setContextProperty("audioRoutingController", &audioRoutingController);
    engine.rootContext()->setContextProperty("webRemoteController", &webRemoteController);
    engine.rootContext()->setContextProperty("mcpServerController", &mcpServerController);
    engine.rootContext()->setContextProperty("openDrawerOnStart", parser.isSet(drawerOption) || parser.isSet(hotkeyPromptOption) || parser.isSet(scrollCueOption));
    engine.rootContext()->setContextProperty("drawerTileIndex", parser.value(drawerIndexOption).toInt());
    engine.rootContext()->setContextProperty("initialViewIndex", parser.value(viewOption).toInt());
    engine.rootContext()->setContextProperty("openHotkeyPromptOnStart", parser.isSet(hotkeyPromptOption));
    engine.rootContext()->setContextProperty("scrollToCueOnStart", parser.isSet(scrollCueOption));
    engine.rootContext()->setContextProperty("openDeleteTabPromptOnStart", parser.isSet(deleteTabPromptOption));
    engine.rootContext()->setContextProperty("openTabHotkeyPromptOnStart", parser.isSet(tabHotkeyPromptOption));
    engine.rootContext()->setContextProperty("openActionHotkeyPromptOnStart", parser.isSet(actionHotkeyPromptOption) ? (parser.value(actionHotkeyPromptOption).isEmpty() ? QString("stopAll") : parser.value(actionHotkeyPromptOption)) : QString());
    engine.rootContext()->setContextProperty("scrollBottomOnStart", parser.isSet(scrollBottomOption));
    engine.rootContext()->setContextProperty("openAccordionOnStart", parser.isSet(openAccordionOption));
    engine.rootContext()->setContextProperty("focusMicOnStart", parser.isSet(testFocusMicOption));
    engine.rootContext()->setContextProperty("focusVirtOnStart", parser.isSet(testFocusVirtOption));
    engine.rootContext()->setContextProperty("fullscreenOnStart", parser.isSet(fullscreenOption));
    engine.rootContext()->setContextProperty("legacyStartupFile", legacyStartupFile);
    engine.rootContext()->setContextProperty("openLegacyImportPromptOnStart", parser.isSet(legacyImportPromptOption));
    engine.rootContext()->setContextProperty("openPttPromptOnStart", parser.isSet(pttPromptOption));
    engine.rootContext()->setContextProperty("openImportDirPromptOnStart", parser.isSet(importDirPromptOption));
    engine.rootContext()->setContextProperty("importDirInitialPath", parser.value(importDirPromptOption));

    // Install application-wide hotkey event filter
    GlobalHotkeyFilter hotkeyFilter(&hotkeyManager, &app);
    app.installEventFilter(&hotkeyFilter);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    engine.loadFromModule("CasterSoundboard", "Main");
#else
    const QUrl url(QStringLiteral("qrc:/qt/qml/CasterSoundboard/src/qml/Main.qml"));
    engine.load(url);
#endif

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
