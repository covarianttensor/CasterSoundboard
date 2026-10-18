#include <QTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTcpSocket>

#include "McpServerController.h"
#include "SoundboardProjectController.h"
#include "AudioRoutingController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

class TestMcpServer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        m_project = new SoundboardProjectController(this);
        m_routing = new AudioRoutingController(this);
        m_mcp = new McpServerController(this);

        m_mcp->setProjectController(m_project);
        m_mcp->setAudioRoutingController(m_routing);
        m_mcp->setPort(5060); // Use 5060 for testing to avoid conflicts

        // Populate sample sound tab
        m_project->newProject();
        m_project->tabModel()->renameTab(0, "Main SFX");

        auto *grid = m_project->tabModel()->gridModelAt(0);
        QVERIFY(grid != nullptr);
        auto *tile0 = grid->appendTile();
        QVERIFY(tile0 != nullptr);
        tile0->setDisplayName("Test Chime");
        tile0->setFilePath("tests/cues/test_loop_chime.wav");

        // Add a second tab
        auto *grid2 = m_project->tabModel()->addTab("Background Ambience");
        QVERIFY(grid2 != nullptr);
        QCOMPARE(m_project->tabModel()->count(), 2);
    }

    void cleanupTestCase()
    {
        if (m_mcp) {
            m_mcp->stopServer();
        }
    }

    void testServerStartStop()
    {
        QVERIFY(!m_mcp->isRunning());
        QSignalSpy runSpy(m_mcp, &McpServerController::runningChanged);

        bool started = m_mcp->startServer();
        QVERIFY(started);
        QVERIFY(m_mcp->isRunning());
        QCOMPARE(runSpy.count(), 1);

        m_mcp->stopServer();
        QVERIFY(!m_mcp->isRunning());
        QCOMPARE(runSpy.count(), 2);
    }

    void testInitializeRpc()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 1;
        req["method"] = "initialize";
        req["params"] = QJsonObject();

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QCOMPARE(res["jsonrpc"].toString(), QString("2.0"));
        QCOMPARE(res["id"].toInt(), 1);
        QVERIFY(res.contains("result"));

        QJsonObject result = res["result"].toObject();
        QCOMPARE(result["protocolVersion"].toString(), QString("2024-11-05"));
        QVERIFY(result.contains("capabilities"));
        QCOMPARE(result["serverInfo"].toObject()["name"].toString(), QString("caster-soundboard"));
    }

    void testToolsListRpc()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 2;
        req["method"] = "tools/list";

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(result.contains("tools"));

        QJsonArray tools = result["tools"].toArray();
        QVERIFY(tools.size() >= 7);

        QStringList expectedTools = {
            "caster_get_state",
            "caster_trigger_sound",
            "caster_stop_all",
            "caster_set_ducking",
            "caster_switch_tab",
            "caster_set_volume",
            "caster_get_audio_routing"
        };

        QStringList actualTools;
        for (const auto &val : tools) {
            actualTools << val.toObject()["name"].toString();
        }

        for (const auto &expected : expectedTools) {
            QVERIFY2(actualTools.contains(expected), qPrintable(QString("Missing tool: %1").arg(expected)));
        }
    }

    void testCallGetState()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 3;
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "caster_get_state";
        params["arguments"] = QJsonObject();
        req["params"] = params;

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(!result["isError"].toBool());

        QJsonArray content = result["content"].toArray();
        QCOMPARE(content.size(), 1);
        QString text = content.first().toObject()["text"].toString();

        QJsonDocument parsedState = QJsonDocument::fromJson(text.toUtf8());
        QVERIFY(parsedState.isObject());
        QCOMPARE(parsedState.object()["appName"].toString(), QString("CasterSoundboard"));
        QVERIFY(parsedState.object().contains("tabs"));
        QCOMPARE(parsedState.object()["tabs"].toArray().size(), 2);
    }

    void testCallTriggerSound()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 4;
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "caster_trigger_sound";
        QJsonObject args;
        args["soundName"] = "Test Chime";
        params["arguments"] = args;
        req["params"] = params;

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(!result["isError"].toBool());

        QString text = result["content"].toArray().first().toObject()["text"].toString();
        QVERIFY(text.contains("Test Chime"));
    }

    void testCallStopAll()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 5;
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "caster_stop_all";
        params["arguments"] = QJsonObject();
        req["params"] = params;

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(!result["isError"].toBool());
        QString text = result["content"].toArray().first().toObject()["text"].toString();
        QVERIFY(text.contains("Stop All executed"));
    }

    void testCallSetDucking()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 6;
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "caster_set_ducking";
        QJsonObject args;
        args["enabled"] = true;
        args["duckingRatio"] = 0.35;
        params["arguments"] = args;
        req["params"] = params;

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(!result["isError"].toBool());

        QVERIFY(m_project->isDuckingActive());
        QCOMPARE(m_project->duckingVolumeRatio(), 0.35);

        // Turn ducking off
        args["enabled"] = false;
        params["arguments"] = args;
        req["params"] = params;
        m_mcp->handleJsonRpc(req);
        QVERIFY(!m_project->isDuckingActive());
    }

    void testCallSwitchTab()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 7;
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "caster_switch_tab";
        QJsonObject args;
        args["tabName"] = "Background Ambience";
        params["arguments"] = args;
        req["params"] = params;

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(!result["isError"].toBool());

        QCOMPARE(m_project->tabModel()->currentIndex(), 1);

        // Switch back by index
        args.remove("tabName");
        args["tabIndex"] = 0;
        params["arguments"] = args;
        req["params"] = params;
        m_mcp->handleJsonRpc(req);
        QCOMPARE(m_project->tabModel()->currentIndex(), 0);
    }

    void testCallSetVolume()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 8;
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "caster_set_volume";
        QJsonObject args;
        args["masterVolume"] = 0.72;
        params["arguments"] = args;
        req["params"] = params;

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(!result["isError"].toBool());

        QCOMPARE(m_project->masterVolume(), 0.72);
    }

    void testCallAudioRouting()
    {
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 9;
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "caster_get_audio_routing";
        params["arguments"] = QJsonObject();
        req["params"] = params;

        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("result"));
        QJsonObject result = res["result"].toObject();
        QVERIFY(!result["isError"].toBool());

        QString text = result["content"].toArray().first().toObject()["text"].toString();
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
        QVERIFY(doc.isObject());
        QVERIFY(doc.object().contains("virtualMicName"));
    }

    void testErrorHandling()
    {
        // Unknown method
        QJsonObject req;
        req["jsonrpc"] = "2.0";
        req["id"] = 10;
        req["method"] = "invalid_method";
        QJsonObject res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("error"));
        QCOMPARE(res["error"].toObject()["code"].toInt(), -32601);

        // Unknown tool
        req["method"] = "tools/call";
        QJsonObject params;
        params["name"] = "non_existent_tool";
        req["params"] = params;
        res = m_mcp->handleJsonRpc(req);
        QVERIFY(res.contains("error"));
        QCOMPARE(res["error"].toObject()["code"].toInt(), -32602);
    }

private:
    SoundboardProjectController *m_project{nullptr};
    AudioRoutingController *m_routing{nullptr};
    McpServerController *m_mcp{nullptr};
};

QTEST_MAIN(TestMcpServer)
#include "test_mcp_server.moc"
