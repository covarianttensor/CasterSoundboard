#include <QtTest>
#include <QJsonDocument>
#include <QFile>
#include <QProcess>
#include "PulseCompatBackend.h"
#include "CasterPlaybackRouter.h"
#include <QTemporaryDir>
#include <signal.h>
#include <errno.h>

static QStringList processWarnings;
static void captureWarnings(QtMsgType, const QMessageLogContext &, const QString &message) {
    if (message.contains("QProcess: Destroyed")) processWarnings.append(message);
}
using namespace Audio;
namespace {
QJsonArray fixture(const char *name) {
    QFile f(QFINDTESTDATA(QString("fixtures/") + name + ".json"));
    if (!f.open(QIODevice::ReadOnly)) qFatal("Fixture missing");
    return QJsonDocument::fromJson(f.readAll()).array();
}
struct FakeServer {
    QMap<QString,QJsonArray> lists;
    QString defaultSink = "sink.card.jbl", defaultSource = "source.card.focusrite";
    QStringList mutations;
    int serial = 500, loads = 0, failLoad = 0;
    bool failDefault = false, failUnload = false;
    FakeServer() { for (auto k : {"cards", "sinks", "sources"}) lists[k] = fixture(k); }
    QString argument(const QStringList &a, const QString &key) {
        for (auto v : a) if (v.startsWith(key + "=")) return v.mid(key.size()+1);
        return {};
    }
    CommandResult run(const QStringList &a) {
        if (a == QStringList({"-f","json","info"})) return {true, QString::fromUtf8(QJsonDocument(QJsonObject{{"server_name","Fake PipeWire"},{"default_sink_name",defaultSink},{"default_source_name",defaultSource}}).toJson()), {}};
        if (a.mid(0,3) == QStringList({"-f","json","list"})) return {true,QString::fromUtf8(QJsonDocument(lists[a[3]]).toJson()),{}};
        mutations << a.join(' ');
        if (a[0] == "set-card-profile") {
            auto cards = lists["cards"];
            for (int i=0;i<cards.size();++i) { auto c=cards[i].toObject(); if (c["name"]==a[1]) { c["active_profile"]=a[2]; cards[i]=c; } }
            lists["cards"]=cards;
            // Simulate endpoint renaming on profile activation.
            if (a[2].contains("input:")) {
                lists["sources"]=fixture("sources");
                auto sinks=lists["sinks"];
                for(int i=0;i<sinks.size();++i) { auto s=sinks[i].toObject(); if(s["name"]=="sink.card.focusrite") { s["name"]="sink.card.focusrite.2"; sinks[i]=s; } }
                lists["sinks"]=sinks;
            }
            return {true,{}, {}};
        }
        if (a[0] == "load-module") {
            if (++loads == failLoad) return {false,{},"Injected module failure"};
            int id=serial++;
            lists["modules"].append(QJsonObject{{"index",id},{"name",a[1]},{"argument",a.mid(2).join(' ')}});
            if(a[1]=="module-null-sink") {
                QString name=argument(a,"sink_name");
                lists["sinks"].append(QJsonObject{{"index",id},{"name",name}});
                lists["sources"].append(QJsonObject{{"index",id},{"name",name+".monitor"}});
            }
            if(a[1]=="module-remap-source") {
                QString name=argument(a,"source_name");
                lists["sources"].append(QJsonObject{{"index",id},{"name",name}});
            }
            return {true,QString::number(id),{}};
        }
        if (a[0]=="unload-module") {
            if (failUnload) { failUnload=false; return {false, {}, "Injected cleanup failure"}; }
            for (auto kind : {"modules","sinks","sources"}) {
                QJsonArray next;
                for(auto v:lists[kind]) if(QString::number(v.toObject()["index"].toInt())!=a[1]) next.append(v);
                lists[kind]=next;
            }
            return {true,{}, {}};
        }
        if(a[0]=="set-default-sink") {
            if(failDefault) { failDefault=false; return {false,{},"Injected default failure"}; }
            defaultSink=a[1]; return {true,{}, {}};
        }
        if(a[0]=="set-default-source") {defaultSource=a[1];return {true,{}, {}};}
        if(a[0]=="move-sink-input") return {true,{}, {}};
        return {false,{},"Unimplemented fake command: "+a.join(' ')};
    }
    Configuration config() {
        Configuration c;c.microphone="card.focusrite|input|analog-input";c.systemOutput="card.jbl|output|analog-output";
        c.monitorOutput="card.focusrite|output|analog-output";c.defaultInput=true;return c;
    }
    void outputOnly() {
        auto cards=lists["cards"];
        for(int i=0;i<cards.size();++i) {auto c=cards[i].toObject();if(c["name"]=="card.focusrite") {c["active_profile"]="output:analog-stereo";cards[i]=c;}}
        lists["cards"]=cards;QJsonArray sources;
        for(auto v:lists["sources"]) if(v.toObject()["name"].toString().endsWith(".monitor")) sources.append(v);
        lists["sources"]=sources;
    }
};
}
class AudioTests : public QObject {
    Q_OBJECT
private slots:
    void subscriberShutdown() {
        QTemporaryDir dir;
        QFile script(dir.filePath("pactl")); QVERIFY(script.open(QIODevice::WriteOnly));
        script.write("#!/bin/sh\necho $$ > \"$CASTER_TEST_PID\"\nexec /bin/sleep 60\n"); script.close();
        QVERIFY(script.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        const auto oldPath=qgetenv("PATH"); const auto oldPid=qgetenv("CASTER_TEST_PID");
        qputenv("PATH",dir.path().toUtf8()+":"+oldPath); qputenv("CASTER_TEST_PID",dir.filePath("pid").toUtf8());
        processWarnings.clear(); auto previous=qInstallMessageHandler(captureWarnings);
        qint64 pid=0;
        {
            Audio::CasterPlaybackRouter router; router.setEnabled(true);
            for(int i=0;i<100 && !QFile::exists(dir.filePath("pid"));++i) QTest::qWait(10);
            QFile file(dir.filePath("pid")); if(file.open(QIODevice::ReadOnly)) pid=file.readAll().trimmed().toLongLong();
        }
        qInstallMessageHandler(previous); qputenv("PATH",oldPath);
        if (oldPid.isNull()) qunsetenv("CASTER_TEST_PID"); else qputenv("CASTER_TEST_PID",oldPid);
        QVERIFY(pid>0); QCOMPARE(::kill(pid,0),-1); QCOMPARE(errno,ESRCH);
        QVERIFY2(processWarnings.isEmpty(),qPrintable(processWarnings.join("\n")));
    }
    void inactiveInputAndRenaming() {
        FakeServer f;f.outputOnly();PulseCompatBackend b([&](const QStringList&a){return f.run(a);});
        auto d=b.probe(); QVERIFY2(d.error.isEmpty(),qPrintable(d.error));
        bool found=false;for(auto e:d.endpoints) if(e.id==f.config().microphone) {found=true;QVERIFY(!e.active);QVERIFY(e.available);}
        QVERIFY(found);auto p=b.validateConfiguration(f.config());QVERIFY2(p.error.isEmpty(),qPrintable(p.error));
        QVERIFY(!p.metadata["profiles"].toMap().isEmpty());QVERIFY(f.mutations.isEmpty());
        auto r=b.apply(p);QVERIFY2(r.ok,qPrintable(r.message));
        QVERIFY(f.mutations.join('\n').contains("sink=sink.card.focusrite.2"));
        QVERIFY(b.stopManagedRouting().ok);
    }
    void idempotenceAndMonitorDiff() {
        FakeServer f;PulseCompatBackend b([&](const QStringList&a){return f.run(a);});
        auto c=f.config();auto r=b.apply(b.validateConfiguration(c));QVERIFY2(r.ok,qPrintable(r.message));
        QCOMPARE(f.lists["modules"].size(),6);int loads=f.loads;
        QVERIFY(b.apply(b.validateConfiguration(c)).ok);QCOMPARE(f.loads,loads);QCOMPARE(f.lists["modules"].size(),6);
        c.monitor=false;QVERIFY(b.apply(b.validateConfiguration(c)).ok);QCOMPARE(f.lists["modules"].size(),5);
        c.monitor=true;QVERIFY(b.apply(b.validateConfiguration(c)).ok);QCOMPARE(f.lists["modules"].size(),6);
        for(auto v:f.lists["modules"]) {
            QString arg=v.toObject()["argument"].toString();
            QVERIFY(!(arg.contains("source=source.card.focusrite") && arg.contains("sink=sink.card.focusrite")));
        }
        QVERIFY(b.stopManagedRouting().ok);QCOMPARE(f.lists["modules"].size(),0);QCOMPARE(f.defaultSource,QString("source.card.focusrite"));
    }
    void rollbackEachLoad_data() {QTest::addColumn<int>("failure");for(int n=1;n<=6;++n)QTest::newRow(qPrintable(QString::number(n)))<<n;}
    void rollbackEachLoad() {
        QFETCH(int,failure);FakeServer f;f.failLoad=failure;PulseCompatBackend b([&](const QStringList&a){return f.run(a);});
        auto r=b.apply(b.validateConfiguration(f.config()));QVERIFY(!r.ok);QVERIFY(r.message.contains("Injected module failure"));
        QCOMPARE(f.lists["modules"].size(),0);QCOMPARE(f.defaultSource,QString("source.card.focusrite"));QCOMPARE(f.defaultSink,QString("sink.card.jbl"));
    }
    void defaultFailureRollback() {
        FakeServer f;f.failDefault=true;PulseCompatBackend b([&](const QStringList&a){return f.run(a);});
        auto r=b.apply(b.validateConfiguration(f.config()));QVERIFY(!r.ok);QCOMPARE(f.lists["modules"].size(),0);QCOMPARE(f.defaultSource,QString("source.card.focusrite"));
    }
    void cleanupFailureRemainsDegradedUntilStop() {
        FakeServer f; PulseCompatBackend b([&](const QStringList &a) { return f.run(a); });
        auto c = f.config(); QVERIFY(b.apply(b.validateConfiguration(c)).ok);
        c.monitor = false; f.failUnload = true;
        auto r = b.apply(b.validateConfiguration(c)); QVERIFY(!r.ok);
        QVERIFY(!b.status().ok); QVERIFY(!b.apply(b.validateConfiguration(c)).ok);
        QVERIFY(b.stopManagedRouting().ok); QVERIFY(b.status().ok);
        QCOMPARE(f.lists["modules"].size(), 0);
    }
    void failedReapplyKeepsWorkingGraph() {
        FakeServer f; PulseCompatBackend b([&](const QStringList &a) { return f.run(a); });
        auto c = f.config(); QVERIFY(b.apply(b.validateConfiguration(c)).ok);
        const auto before = f.lists["modules"]; const auto source = f.defaultSource;
        c.defaultInput = false; f.failDefault = true;
        auto r = b.apply(b.validateConfiguration(c)); QVERIFY(!r.ok);
        QCOMPARE(f.lists["modules"], before); QCOMPARE(f.defaultSource, source);
        QVERIFY(!r.playbackTarget.isEmpty()); QVERIFY(b.stopManagedRouting().ok);
    }
    void collisionsAndForeignDefaults() {
        FakeServer f;f.lists["sinks"].append(QJsonObject{{"index",99},{"name","VirtualMic"}});
        f.lists["modules"].append(QJsonObject{{"index",99},{"name","module-null-sink"},{"argument","sink_name=VirtualMic"}});
        PulseCompatBackend b([&](const QStringList&a){return f.run(a);});auto r=b.apply(b.validateConfiguration(f.config()));
        QVERIFY2(r.ok,qPrintable(r.message));QVERIFY(r.voiceSource.startsWith("VirtualMic_"));
        f.defaultSource="another.app.source";QVERIFY(b.stopManagedRouting().ok);
        QCOMPARE(f.defaultSource,QString("another.app.source"));QCOMPARE(f.lists["modules"].size(),1);QCOMPARE(f.lists["modules"][0].toObject()["index"].toInt(),99);
    }
    void customVirtualMicName() {
        FakeServer f;
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); });
        auto c = f.config();
        c.virtualMicName = "Discord Soundboard Mic";
        auto plan = b.validateConfiguration(c);
        QVERIFY2(plan.error.isEmpty(), qPrintable(plan.error));
        QVERIFY(plan.changes.join('\n').contains("Discord_Soundboard_Mic"));
        auto r = b.apply(plan);
        QVERIFY2(r.ok, qPrintable(r.message));
        QCOMPARE(r.voiceSource, QString("Discord_Soundboard_Mic"));
        bool foundSink = false;
        for (auto v : f.lists["sinks"]) {
            if (v.toObject()["name"].toString() == "Discord_Soundboard_Mic") foundSink = true;
        }
        QVERIFY(foundSink);
        bool foundSource = false;
        for (auto v : f.lists["sources"]) {
            if (v.toObject()["name"].toString() == "Discord_Soundboard_Mic") foundSource = true;
        }
        QVERIFY(foundSource);
        QVERIFY(b.stopManagedRouting().ok);
    }
    void rejectReusedModuleId() {
        FakeServer f;PulseCompatBackend b([&](const QStringList&a){return f.run(a);});QVERIFY(b.apply(b.validateConfiguration(f.config())).ok);
        auto m=f.lists["modules"][0].toObject();m["argument"]="sink_name=someone_else";f.lists["modules"][0]=m;
        auto r=b.stopManagedRouting();QVERIFY(!r.ok);QVERIFY(r.message.contains("identity changed"));QCOMPARE(f.lists["modules"].size(),1);
    }
    void stopWithoutSettings() {
        FakeServer f;PulseCompatBackend b([&](const QStringList&a){return f.run(a);});QVERIFY(b.stopManagedRouting().ok);QVERIFY(f.mutations.isEmpty());
    }
    void missingAndMalformedServer() {
        PulseCompatBackend missing([](const QStringList&){return CommandResult{false,{},"pactl is missing"};});QVERIFY(missing.probe().error.contains("missing"));
        PulseCompatBackend malformed([](const QStringList&){return CommandResult{true,"not json",{}};});QVERIFY(!malformed.probe().error.isEmpty());
    }
    void incompatibleProfilesDontMutate() {
        FakeServer f;auto cards=f.lists["cards"];
        for(int i=0;i<cards.size();++i) {auto c=cards[i].toObject();if(c["name"]=="card.focusrite") {
            auto profiles=c["profiles"].toObject();
            profiles.remove("output:analog-stereo+input:analog-stereo");
            profiles.remove("pro-audio");
            c["profiles"]=profiles;cards[i]=c;
        }} f.lists["cards"]=cards;
        PulseCompatBackend b([&](const QStringList&a){return f.run(a);});auto p=b.validateConfiguration(f.config());QVERIFY(!p.error.isEmpty());QVERIFY(f.mutations.isEmpty());
    }
    void proAudioDuplexSelection() {
        FakeServer f;auto cards=f.lists["cards"];
        for(int i=0;i<cards.size();++i) {auto c=cards[i].toObject();if(c["name"]=="card.focusrite") {
            auto profiles=c["profiles"].toObject();
            profiles.remove("output:analog-stereo+input:analog-stereo");
            c["profiles"]=profiles;cards[i]=c;
        }} f.lists["cards"]=cards;
        PulseCompatBackend b([&](const QStringList&a){return f.run(a);});
        auto p=b.validateConfiguration(f.config());
        QVERIFY2(p.error.isEmpty(), qPrintable(p.error));
        QCOMPARE(p.metadata["profiles"].toMap()["card.focusrite"].toString(), QString("pro-audio"));
        auto r=b.apply(p);
        QVERIFY2(r.ok, qPrintable(r.message));
        QVERIFY(b.stopManagedRouting().ok);
    }
    void pactl16ModuleIndexFallback() {
        FakeServer f;
        PulseCompatBackend b([&](const QStringList &a) {
            if (a == QStringList({"-f", "json", "list", "modules"})) {
                QJsonArray output{QJsonObject{{"name", "libpipewire-module-rt"}, {"argument", "{\n rt.prio = 88\n}"}}};
                for (auto v : f.lists["modules"]) { auto m = v.toObject(); m.remove("index"); output.append(m); }
                return CommandResult{true, QString::fromUtf8(QJsonDocument(output).toJson()), {}};
            }
            if (a == QStringList({"list", "short", "modules"})) {
                QString output = "1\tlibpipewire-module-rt\t{\n rt.prio = 88\n}\t\n";
                for (auto v : f.lists["modules"]) { auto m = v.toObject(); output += QString::number(m["index"].toInt()) + "\t" + m["name"].toString() + "\t" + m["argument"].toString() + "\n"; }
                return CommandResult{true, output, {}};
            }
            return f.run(a);
        });
        auto c = f.config(); auto r = b.apply(b.validateConfiguration(c)); QVERIFY2(r.ok, qPrintable(r.message));
        auto r2 = b.apply(b.validateConfiguration(c)); QVERIFY2(r2.ok, qPrintable(r2.message)); QCOMPARE(f.lists["modules"].size(), 6);
        QVERIFY(b.stopManagedRouting().ok); QVERIFY(f.lists["modules"].isEmpty());
    }
    void recoveryDiscoveryIsGeneric() {
        FakeServer f;
        auto nodes = f.lists["sinks"];
        for (int i = 0; i < nodes.size(); ++i) { auto n = nodes[i].toObject(); n["state"] = "SUSPENDED"; nodes[i] = n; }
        auto healthy = PulseCompatBackend::decodeDevices(f.lists["cards"], nodes, f.lists["sources"]);
        for (auto e : healthy) QVERIFY(e.healthWarning.isEmpty());
        QJsonArray missing;
        for (auto v : nodes) if (v.toObject()["name"] != "sink.card.focusrite") missing.append(v);
        auto broken = PulseCompatBackend::decodeDevices(f.lists["cards"], missing, f.lists["sources"]);
        bool detected = false;
        for (auto e : broken) if (e.id == f.config().monitorOutput) { detected = true; QVERIFY(e.recoverySupported); QVERIFY(e.healthWarning.contains("missing")); }
        QVERIFY(detected);
        f.outputOnly();
        auto inactive = PulseCompatBackend::decodeDevices(f.lists["cards"], f.lists["sinks"], f.lists["sources"]);
        for (auto e : inactive) if (e.id == f.config().microphone) { QVERIFY(e.recoverySupported); QVERIFY(e.healthWarning.isEmpty()); }
    }
    void disabledInputRecoveryPreservesDuplex() {
        FakeServer f; f.outputOnly();
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); }, [](unsigned long) {});
        auto plan = b.validateRecovery(f.config().microphone);
        QVERIFY2(plan.error.isEmpty(), qPrintable(plan.error)); QVERIFY(f.mutations.isEmpty());
        QCOMPARE(plan.metadata["cycle"].toStringList().last(), QString("output:analog-stereo+input:analog-stereo"));
        auto r = b.recoverDevice(plan); QVERIFY2(r.ok, qPrintable(r.message));
        QCOMPARE(f.defaultSink, QString("sink.card.jbl")); QCOMPARE(f.defaultSource, QString("source.card.focusrite"));
        QCOMPARE(f.loads, 0);
        for (auto command : f.mutations) QVERIFY(command.startsWith("set-card-profile card.focusrite "));
    }
    void disabledUnavailableDeviceRemainsRecoverable() {
        FakeServer f;
        auto cards = f.lists["cards"];
        for (int i = 0; i < cards.size(); ++i) {
            auto c = cards[i].toObject(); if (c["name"] != "card.focusrite") continue;
            c["active_profile"] = "off";
            auto profiles = c["profiles"].toObject();
            for (auto it = profiles.begin(); it != profiles.end(); ++it) { auto p = it.value().toObject(); p["available"] = false; it.value() = p; }
            c["profiles"] = profiles; cards[i] = c;
        }
        f.lists["cards"] = cards;
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); }, [](unsigned long) {});
        auto d = b.probe(); bool seen = false;
        for (auto e : d.endpoints) if (e.id == f.config().microphone) { seen = true; QVERIFY(e.recoverySupported); QVERIFY(e.healthWarning.contains("disabled")); }
        QVERIFY(seen);
        auto p = b.validateRecovery(f.config().microphone); QVERIFY2(p.error.isEmpty(), qPrintable(p.error));
        QCOMPARE(p.metadata["cycle"].toStringList().size(), 1);
        QVERIFY(b.recoverDevice(p).ok);
    }
    void recoveryMissingEndpointNeverClaimsSuccess() {
        FakeServer f;
        PulseCompatBackend b([&](const QStringList &a) {
            if (a == QStringList({"-f", "json", "list", "sources"})) return CommandResult{true, "[]", {}};
            return f.run(a);
        }, [](unsigned long) {});
        auto p = b.validateRecovery(f.config().microphone); QVERIFY(p.error.isEmpty());
        auto r = b.recoverDevice(p); QVERIFY(!r.ok); QVERIFY(r.message.contains("still missing"));
        QVERIFY(r.message.contains("Original card profile restored"));
        for (auto command : f.mutations) QVERIFY(command.startsWith("set-card-profile card.focusrite "));
    }
    void recoveryRejectsStalePreviewAndActiveRouting() {
        FakeServer f; PulseCompatBackend b([&](const QStringList &a) { return f.run(a); }, [](unsigned long) {});
        auto p = b.validateRecovery(f.config().microphone); QVERIFY(p.error.isEmpty()); f.outputOnly();
        QVERIFY(!b.recoverDevice(p).ok); QVERIFY(f.mutations.isEmpty());
        QVERIFY(b.apply(b.validateConfiguration(f.config())).ok);
        auto blocked = b.validateRecovery(f.config().microphone); QVERIFY(blocked.error.contains("Stop Caster routing"));
        QVERIFY(!b.recoverDevice(p).ok); QVERIFY(b.stopManagedRouting().ok);
    }
    void recoveryRespectsConcurrentProfileChanges() {
        FakeServer f; bool changed = false;
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); }, [&](unsigned long) {
            if (changed) return;
            changed = true;
            auto cards = f.lists["cards"];
            for (int i = 0; i < cards.size(); ++i) { auto c = cards[i].toObject(); if (c["name"] == "card.focusrite") { c["active_profile"] = "external-choice"; cards[i] = c; } }
            f.lists["cards"] = cards;
        });
        auto r = b.recoverDevice(b.validateRecovery(f.config().microphone)); QVERIFY(!r.ok);
        QVERIFY(r.message.contains("external state was left untouched")); QCOMPARE(f.mutations.size(), 1);
    }
    void portlessCardIsDiscoverable() {
        QJsonArray cards{QJsonObject{{"index", 1}, {"name", "usb.device"}, {"active_profile", "off"},
            {"profiles", QJsonObject{{"off", QJsonObject{{"sinks", 0}, {"sources", 0}}},
                {"duplex", QJsonObject{{"sinks", 1}, {"sources", 1}, {"available", true}}}}}}};
        auto d = PulseCompatBackend::decodeDevices(cards, {}, {}); QCOMPARE(d.size(), 2);
        for (auto e : d) { QVERIFY(e.recoverySupported); QVERIFY(e.healthWarning.contains("disabled")); }
    }
    void serviceRestartRequiresSeparatePlan() {
        FakeServer f; QStringList calls;
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); }, [](unsigned long) {}, [&](const QStringList &a) {
            calls << a.join(' ');
            return CommandResult{true, a.contains("show") ? "loaded\nloaded\nloaded" : "", {}};
        });
        auto p = b.validateServiceRestart(); QVERIFY2(p.error.isEmpty(), qPrintable(p.error));
        QCOMPARE(calls.size(), 1); QVERIFY(calls[0].contains(" show ")); QVERIFY(f.mutations.isEmpty());
        QVERIFY(!b.restartAudioServices(Plan()).ok);
        for (auto call : calls) QVERIFY(!call.contains(" restart "));
        auto r = b.restartAudioServices(p); QVERIFY2(r.ok, qPrintable(r.message));
        QCOMPARE(calls.count("--user restart wireplumber.service pipewire.service pipewire-pulse.service"), 1);
        QVERIFY(f.mutations.isEmpty());
    }
    void serviceRestartFailureIsNotSuccess() {
        FakeServer f;
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); }, [](unsigned long) {}, [](const QStringList &a) {
            return a.contains("restart") ? CommandResult{false, {}, "injected systemctl failure"} : CommandResult{true, "loaded\nloaded\nloaded", {}};
        });
        auto r = b.restartAudioServices(b.validateServiceRestart()); QVERIFY(!r.ok); QVERIFY(r.message.contains("Some services may have restarted"));
    }
    void nativePulseAudioDoesNotOfferPipeWireRestart() {
        FakeServer f; int serviceCalls = 0;
        PulseCompatBackend b([&](const QStringList &a) {
            auto r = f.run(a); r.output.replace("Fake PipeWire", "pulseaudio"); return r;
        }, [](unsigned long) {}, [&](const QStringList &) { ++serviceCalls; return CommandResult{true, "loaded\nloaded\nloaded", {}}; });
        QVERIFY(!b.probe().capabilities.serviceRestart); QVERIFY(!b.validateServiceRestart().error.isEmpty()); QCOMPARE(serviceCalls, 0);
    }
    void serviceRestartDisabledWhenSystemctlMissing() {
        FakeServer f;
        const auto oldPath = qgetenv("PATH");
        QTemporaryDir emptyDir;
        qputenv("PATH", emptyDir.path().toUtf8());
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); });
        auto d = b.probe();
        auto restartPlan = b.validateServiceRestart();
        qputenv("PATH", oldPath);
        QVERIFY(!d.capabilities.serviceRestart);
        QVERIFY(!restartPlan.error.isEmpty());
        QVERIFY(restartPlan.error.contains("systemctl"));
    }
    void ga104RecoveryCyclesUnavailableProfilesAndResolvesRenamedSink() {
        FakeServer f; f.lists["cards"] = fixture("ga104-cards"); f.lists["sinks"] = {};
        const QString id = "card.ga104|output|hdmi-output-0";
        PulseCompatBackend b([&](const QStringList &a) {
            if (a == QStringList({"set-card-profile", "card.ga104", "output:hdmi-stereo-extra1"})) {
                f.mutations << a.join(' '); return CommandResult{false, {}, "profile unavailable"};
            }
            auto r = f.run(a);
            if (a == QStringList({"set-card-profile", "card.ga104", "output:hdmi-stereo"})) {
                f.lists["sinks"].append(QJsonObject{{"index", 77}, {"name", "hdmi.output.2"}, {"card", 44},
                    {"ports", QJsonArray{QJsonObject{{"name", "hdmi-output-0"}}}}, {"active_port", "hdmi-output-0"}, {"state", "SUSPENDED"}});
            }
            return r;
        }, [](unsigned long) {});
        auto d = b.probe(); QCOMPARE(d.endpoints.size(), 4);
        for (auto e : d.endpoints) { QVERIFY(e.recoverySupported); if (e.id == id) QVERIFY(e.healthWarning.contains("missing")); }
        auto p = b.validateRecovery(id); QVERIFY2(p.error.isEmpty(), qPrintable(p.error));
        QCOMPARE(p.metadata["cycle"].toStringList(), QStringList({"off", "output:hdmi-stereo-extra1", "output:hdmi-stereo-extra2", "output:hdmi-stereo-extra3", "output:hdmi-stereo"}));
        auto r = b.recoverDevice(p); QVERIFY2(r.ok, qPrintable(r.message));
        QVERIFY(r.message.contains("hdmi.output.2")); QVERIFY(r.message.contains("Skipped profile errors"));
        for (auto e : b.probe().endpoints) if (e.id == id) QVERIFY(e.healthWarning.isEmpty());
    }
    void restartRetiresOwnershipAfterServerReset() {
        FakeServer f;
        PulseCompatBackend b([&](const QStringList &a) { return f.run(a); }, [](unsigned long) {}, [&](const QStringList &a) {
            if (a.contains("restart")) {
                // Simulate old module IDs being reused by another application.
                auto module = f.lists["modules"][0].toObject(); module["argument"] = "sink_name=another_app";
                f.lists["modules"] = QJsonArray{module};
                return CommandResult{true, {}, {}};
            }
            return CommandResult{true, "loaded\nloaded\nloaded", {}};
        });
        QVERIFY(b.apply(b.validateConfiguration(f.config())).ok);
        auto p = b.validateServiceRestart(); QVERIFY(p.error.isEmpty()); QVERIFY(p.metadata["managedRouting"].toBool());
        QVERIFY(b.restartAudioServices(p).ok); f.mutations.clear();
        QVERIFY(b.stopManagedRouting().ok); QVERIFY(f.mutations.isEmpty()); QCOMPARE(f.lists["modules"].size(), 1);
    }
    void restartFallbackWhenPulseServerIsUnreachable() {
        bool restarted = false; FakeServer f;
        PulseCompatBackend b([&](const QStringList &a) {
            return restarted ? f.run(a) : CommandResult{false, {}, "Connection refused"};
        }, [](unsigned long) {}, [&](const QStringList &a) {
            if (a.contains("is-active")) return CommandResult{true, "active", {}};
            if (a.contains("restart")) { restarted = true; return CommandResult{true, {}, {}}; }
            return CommandResult{true, "loaded\nloaded\nloaded", {}};
        });
        auto d = b.probe(); QVERIFY(!d.error.isEmpty()); QVERIFY(d.capabilities.serviceRestart);
        auto p = b.validateServiceRestart(); QVERIFY2(p.error.isEmpty(), qPrintable(p.error));
        QVERIFY(b.restartAudioServices(p).ok);
    }
    void isolatedServerIntegration() {
        if (qgetenv("CASTER_PRIVATE_TEST") != "1") QSKIP("Run with isolated-server.py to test real PipeWire commands.");
        QVERIFY(qgetenv("PULSE_SERVER").startsWith("unix:/tmp/caster-private-audio-"));
        auto real = [](const QStringList &args) {
            QProcess p; p.start("pactl", args);
            if (!p.waitForFinished(5000)) return CommandResult{false, {}, "timeout"};
            return CommandResult{p.exitCode() == 0, QString::fromUtf8(p.readAllStandardOutput()).trimmed(), QString::fromUtf8(p.readAllStandardError()).trimmed()};
        };
        QStringList physical;
        for (auto name : {"sink.card.jbl", "sink.card.focusrite", "mic_master"}) {
            auto r = real({"load-module", "module-null-sink", QString("sink_name=") + name}); QVERIFY2(r.ok, qPrintable(r.error)); physical << r.output;
        }
        auto mic = real({"load-module", "module-remap-source", "master=mic_master.monitor", "source_name=source.card.focusrite"});
        QVERIFY2(mic.ok, qPrintable(mic.error)); physical << mic.output;
        QVERIFY(real({"set-default-sink", "sink.card.jbl"}).ok);
        QVERIFY(real({"set-default-source", "source.card.focusrite"}).ok);
        PulseCompatBackend b([&](const QStringList &args) {
            if (args == QStringList({"-f", "json", "list", "cards"})) return CommandResult{true, QString::fromUtf8(QJsonDocument(fixture("cards")).toJson()), {}};
            auto r = real(args);
            if (args.mid(0, 3) == QStringList({"-f", "json", "list"}) && (args[3] == "sinks" || args[3] == "sources")) {
                auto nodes = QJsonDocument::fromJson(r.output.toUtf8()).array(); auto model = fixture(args[3].toUtf8().constData());
                // Test-only physical endpoint facade; routing operations remain real.
                for (int i = 0; i < nodes.size(); ++i) {
                    auto node = nodes[i].toObject();
                    for (auto v : model) { auto e = v.toObject(); if (node["name"] == e["name"]) {
                        node["ports"] = e["ports"]; node["active_port"] = e["active_port"]; node["properties"] = e["properties"];
                    }} nodes[i] = node;
                }
                r.output = QString::fromUtf8(QJsonDocument(nodes).toJson());
            }
            return r;
        });
        FakeServer config;
        auto plan = b.validateConfiguration(config.config()); QVERIFY2(plan.error.isEmpty(), qPrintable(plan.error));
        auto result = b.apply(plan); QVERIFY2(result.ok, qPrintable(result.message));
        QCOMPARE(result.voiceSource, QString("VirtualMic"));
        result = b.apply(b.validateConfiguration(config.config())); QVERIFY2(result.ok, qPrintable(result.message));
        auto c = config.config(); c.monitor = false;
        result = b.apply(b.validateConfiguration(c)); QVERIFY2(result.ok, qPrintable(result.message));
        // Applications using the default input must attach to the mixed source,
        // without Caster moving any of their capture streams.
        QProcess recorder;
        recorder.start("parecord", {"--client-name=CasterCaptureTest", "--raw", "/dev/null"});
        QVERIFY(recorder.waitForStarted());
        auto captureSource = [&]() {
            const auto streams = QJsonDocument::fromJson(real({"-f", "json", "list", "source-outputs"}).output.toUtf8()).array();
            for (auto v : streams) if (v.toObject()["properties"].toObject()["application.name"] == "CasterCaptureTest") return v.toObject()["source"].toInt(-1);
            return -1;
        };
        int mixedIndex = -1;
        for (auto v : QJsonDocument::fromJson(real({"-f", "json", "list", "sources"}).output.toUtf8()).array()) {
            const auto n = v.toObject();
            if (n["name"].toString() == result.voiceSource) mixedIndex = n["index"].toInt();
        }
        QVERIFY(mixedIndex >= 0); QTRY_COMPARE(captureSource(), mixedIndex);
        // Reproduce Qt/GStreamer's ALSA fallback: its PID exists on the client,
        // while its playback stream has no application.process.id property.
        QProcess alsa;
        alsa.start("aplay", {"-D", "pipewire", "-t", "raw", "-f", "S16_LE", "-r", "48000", "-c", "1", "/dev/zero"});
        QVERIFY(alsa.waitForStarted());
        auto alsaStream = [&]() {
            for (auto v : QJsonDocument::fromJson(real({"-f", "json", "list", "sink-inputs"}).output.toUtf8()).array()) {
                const auto stream = v.toObject();
                if (stream["properties"].toObject()["application.name"].toString().contains("ALSA [aplay]")) return stream;
            }
            return QJsonObject();
        };
        QTRY_VERIFY(!alsaStream().isEmpty());
        QVERIFY(!alsaStream()["properties"].toObject().contains("application.process.id"));
        int boardIndex = -1;
        for (auto v : QJsonDocument::fromJson(real({"-f", "json", "list", "sinks"}).output.toUtf8()).array())
            if (v.toObject()["name"] == "SoundboardSink") boardIndex = v.toObject()["index"].toInt();
        QVERIFY(boardIndex >= 0); QVERIFY(alsaStream()["sink"].toInt() != boardIndex);
        const auto route = b.routePlayback(alsa.processId()); QVERIFY2(route.ok, qPrintable(route.message));
        QTRY_COMPARE(alsaStream()["sink"].toInt(), boardIndex);
        alsa.terminate(); QVERIFY(alsa.waitForFinished(3000));
        result = b.stopManagedRouting(); QVERIFY2(result.ok, qPrintable(result.message));
        QCOMPARE(QJsonDocument::fromJson(real({"-f", "json", "info"}).output.toUtf8()).object()["default_source_name"].toString(), QString("source.card.focusrite"));
        recorder.terminate(); QVERIFY(recorder.waitForFinished(3000));
        auto modules = real({"list", "short", "modules"}); QVERIFY(!modules.output.contains("caster.owner"));
        for (auto id : physical) QVERIFY(real({"unload-module", id}).ok);
    }
    void alsaPlaybackUsesClientProcessIdentity() {
        FakeServer f; PulseCompatBackend b([&](const QStringList &a) { return f.run(a); });
        QVERIFY(b.apply(b.validateConfiguration(f.config())).ok);
        f.lists["clients"] = {
            QJsonObject{{"index",0},{"properties",QJsonObject{{"application.process.id","1234"}}}},
            QJsonObject{{"index",408},{"properties",QJsonObject{{"application.process.id","1234"}}}},
            QJsonObject{{"index",415},{"properties",QJsonObject{{"application.process.id",4321}}}}
        };
        auto stream = [](int index, QJsonValue client) { return QJsonObject{{"index",index},{"sink",1},{"client",client},
            {"properties",QJsonObject{{"application.name","PipeWire ALSA [CasterSoundboard]"},{"client.id","415"}}}}; };
        f.lists["sink-inputs"] = {stream(700,"408"), stream(701,415), stream(702,QJsonValue()), stream(703,"missing")};
        auto explicitOther = stream(704,"408"); auto props = explicitOther["properties"].toObject();
        props["application.process.id"] = "9999"; explicitOther["properties"] = props; f.lists["sink-inputs"].append(explicitOther);
        f.mutations.clear(); QVERIFY(b.routePlayback(1234).ok);
        QCOMPARE(f.mutations.size(),1); QVERIFY(f.mutations[0].startsWith("move-sink-input 700 "));
        f.mutations.clear(); QVERIFY(b.routePlayback(4321).ok);
        QCOMPARE(f.mutations.size(),1); QVERIFY(f.mutations[0].startsWith("move-sink-input 701 "));
    }
    void onlyCurrentProcessStreams() {
        FakeServer f;PulseCompatBackend b([&](const QStringList&a){return f.run(a);});QVERIFY(b.apply(b.validateConfiguration(f.config())).ok);
        f.lists["sink-inputs"]=QJsonArray{
            QJsonObject{{"index",700},{"sink",1},{"properties",QJsonObject{{"application.process.id","1234"}}}},
            QJsonObject{{"index",701},{"sink",1},{"properties",QJsonObject{{"application.process.id","4321"},{"application.name","CasterSoundboard"}}}}};
        f.mutations.clear();QVERIFY(b.routePlayback(1234).ok);QCOMPARE(f.mutations.size(),1);QVERIFY(f.mutations[0].startsWith("move-sink-input 700 "));
    }
};
QTEST_GUILESS_MAIN(AudioTests)
#include "tst_AudioBackend.moc"
