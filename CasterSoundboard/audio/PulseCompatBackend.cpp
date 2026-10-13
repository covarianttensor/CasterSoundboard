#include "PulseCompatBackend.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSet>
#include <QThread>
#include <QUuid>
#include <QRegularExpression>
#include <stdexcept>
#include <limits>
#include <algorithm>

namespace Audio {
namespace {
[[noreturn]] void fail(const QString &s) { throw std::runtime_error(s.toUtf8().constData()); }
QString str(const QJsonValue &v) { return v.isString() ? v.toString() : QString::number(v.toDouble(), 'f', 0); }
QJsonObject named(const QJsonValue &value) {
    if (value.isObject()) return value.toObject();
    QJsonObject result;
    for (auto v : value.toArray()) { auto o = v.toObject(); result.insert(o.value("name").toString(), o); }
    return result;
}
bool available(const QJsonValue &v) { return v != QJsonValue(false) && v.toString() != "no" && v.toString() != "not available"; }
QString cardName(const QJsonObject &node, const QJsonArray &cards) {
    auto p = node.value("properties").toObject();
    for (auto v : cards) {
        auto c = v.toObject();
        if ((!node.value("card").isUndefined() && node.value("card") == c.value("index")) ||
            p.value("device.name") == c.value("name")) return c.value("name").toString();
    }
    return {};
}
bool hasPort(const QJsonObject &node, const QString &port) { return port.isEmpty() ? named(node.value("ports")).isEmpty() : named(node.value("ports")).contains(port); }
}
PulseCompatBackend::PulseCompatBackend(CommandRunner r, std::function<void(unsigned long)> wait, CommandRunner service)
    : runner(r), serviceRunner(service), recoveryWait(wait), token(QUuid::createUuid().toString(QUuid::Id128)) {
    if (!recoveryWait) recoveryWait = [](unsigned long ms) { QThread::msleep(ms); };
    if (!serviceRunner) serviceRunner = [](const QStringList &args) {
#ifdef Q_OS_LINUX
        const auto executable = QStandardPaths::findExecutable("systemctl");
        if (executable.isEmpty()) return CommandResult{false, {}, "systemctl is missing; user audio services cannot be restarted here."};
        QProcess p; auto env = QProcessEnvironment::systemEnvironment(); env.insert("LC_ALL", "C"); p.setProcessEnvironment(env);
        p.start(executable, args);
        if (!p.waitForStarted(2000)) return CommandResult{false, {}, p.errorString()};
        if (!p.waitForFinished(15000)) { p.kill(); p.waitForFinished(1000); return CommandResult{false, {}, "Service command timed out; services may still be restarting."}; }
        return CommandResult{p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0,
            QString::fromUtf8(p.readAllStandardOutput()).trimmed(), QString::fromUtf8(p.readAllStandardError()).trimmed()};
#else
        Q_UNUSED(args); return CommandResult{false, {}, "User audio service restart is supported on Linux only."};
#endif
    };
    if (!runner) runner = [](const QStringList &args) {
#ifdef Q_OS_LINUX
        QString executable = QStandardPaths::findExecutable("pactl");
        if (executable.isEmpty()) return CommandResult{false, {}, "pactl is missing. Install pulseaudio-utils."};
        QProcess p;
        auto env = QProcessEnvironment::systemEnvironment(); env.insert("LC_ALL", "C"); p.setProcessEnvironment(env);
        p.start(executable, args);
        if (!p.waitForStarted(2000)) return CommandResult{false, {}, p.errorString()};
        if (!p.waitForFinished(4000)) { p.kill(); p.waitForFinished(1000); return CommandResult{false, {}, "Audio server command timed out."}; }
        return CommandResult{p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0,
            QString::fromUtf8(p.readAllStandardOutput()).trimmed(), QString::fromUtf8(p.readAllStandardError()).trimmed()};
#else
        Q_UNUSED(args);
        return CommandResult{false, {}, "PulseAudio-compatible routing is available on Linux only."};
#endif
    };
}
QString PulseCompatBackend::command(const QStringList &args) {
    auto r = runner(args);
    if (!r.ok) fail("pactl " + args.join(' ') + ": " + r.error);
    return r.output;
}
QJsonArray PulseCompatBackend::list(const QString &kind) {
    QJsonParseError e;
    auto d = QJsonDocument::fromJson(command({"-f", "json", "list", kind}).toUtf8(), &e);
    if (e.error != QJsonParseError::NoError || !d.isArray()) fail("Unsupported or malformed pactl JSON for " + kind + ". Update pulseaudio-utils.");
    // pactl 16 omits module indices from JSON. Its tab-delimited short format
    // retains them; parse only this narrowly defined, integration-tested fallback.
    if (kind == "modules") {
        bool indices = true;
        for (auto v : d.array()) if (!v.toObject().contains("index")) indices = false;
        if (!indices) {
            QJsonArray modules;
            for (auto line : command({"list", "short", "modules"}).split('\n')) {
                auto fields = line.split('\t');
                // Native PipeWire modules contain multiline arguments. They are
                // not owned by this adapter and are deliberately excluded.
                if (fields.size() < 3 || !QRegularExpression("^[0-9]+$").match(fields[0]).hasMatch() ||
                    (fields[1] != "module-null-sink" && fields[1] != "module-loopback")) continue;
                bool matchesJson = false;
                for (auto v : d.array()) {
                    auto m = v.toObject();
                    if (m.value("name").toString() == fields[1] && m.value("argument").toString() == fields[2]) matchesJson = true;
                }
                if (!matchesJson) fail("Module listing changed or has unsupported formatting. Refresh and retry.");
                modules.append(QJsonObject{{"index", fields[0]}, {"name", fields[1]}, {"argument", fields[2]}});
            }
            for (auto v : d.array()) {
                auto m = v.toObject();
                if (m.value("name") != "module-null-sink" && m.value("name") != "module-loopback") continue;
                bool found = false;
                for (auto item : modules) {
                    auto n = item.toObject();
                    if (n.value("name") == m.value("name") && n.value("argument") == m.value("argument")) found = true;
                }
                if (!found) fail("Cannot resolve module IDs safely. Refresh and retry.");
            }
            return modules;
        }
    }
    for (auto v : d.array()) if (!v.isObject() || !v.toObject().contains("index")) fail("Incomplete pactl JSON for " + kind);
    return d.array();
}
QJsonObject PulseCompatBackend::info() {
    auto d = QJsonDocument::fromJson(command({"-f", "json", "info"}).toUtf8());
    if (!d.isObject() || !d.object().contains("default_sink_name") || !d.object().contains("default_source_name"))
        fail("Audio server does not support the required pactl JSON info format.");
    return d.object();
}
Discovery PulseCompatBackend::probe() { return enumerateDevices(); }
QVector<Endpoint> PulseCompatBackend::decodeDevices(const QJsonArray &cards, const QJsonArray &sinks, const QJsonArray &sources) {
    QVector<Endpoint> out;
    for (auto cv : cards) {
        auto c = cv.toObject(); const QString card = c.value("name").toString();
        auto profiles = named(c.value("profiles")); auto ports = named(c.value("ports"));
        const bool portless = ports.isEmpty();
        if (portless) for (bool input : {true, false}) {
            QJsonArray membership;
            for (auto it = profiles.begin(); it != profiles.end(); ++it)
                if (it.value().toObject().value(input ? "sources" : "sinks").toInt() > 0) membership.append(it.key());
            if (!membership.isEmpty()) ports.insert(input ? "@input" : "@output", QJsonObject{
                {"direction", input ? "input" : "output"}, {"description", input ? "Input" : "Output"}, {"profiles", membership}});
        }
        for (auto p = ports.begin(); p != ports.end(); ++p) {
            auto port = p.value().toObject();
            for (bool input : {true, false}) {
                QStringList compatible, advertised;
                // Pulse card ports often omit direction. Infer it from advertised profiles,
                // conventional port direction, and active endpoint membership.
                const QString direction = port.value("direction").toString();
                bool role = direction == (input ? "input" : "output") || p.key().contains(input ? "input" : "output");
                for (auto nv : input ? sources : sinks) {
                    auto n = nv.toObject();
                    if (!n.value("name").toString().endsWith(".monitor") && cardName(n, cards) == card && hasPort(n, portless ? QString() : p.key())) role = true;
                }
                if (!role) continue;
                for (auto pv : port.value("profiles").toArray()) {
                    const QString name = pv.isString() ? pv.toString() : pv.toObject().value("name").toString();
                    auto profile = profiles.value(name).toObject();
                    if (profile.value(input ? "sources" : "sinks").toInt() > 0) {
                        advertised << name;
                        if (available(profile.value("available"))) compatible << name;
                    }
                }
                const auto recoveryProfiles = advertised;
                if (recoveryProfiles.isEmpty()) continue;
                Endpoint e; e.input = input; e.available = available(port.value("availability"));
                e.id = card + "|" + (input ? "input|" : "output|") + p.key();
                e.label = c.value("properties").toObject().value("device.description").toString(card) + " — " + port.value("description").toString(p.key());
                e.recoverySupported = !recoveryProfiles.isEmpty();
                e.metadata = {{"card", card}, {"port", portless ? QString() : p.key()}, {"profiles", compatible}, {"recoveryProfiles", recoveryProfiles}};
                bool realized = false;
                for (auto nv : input ? sources : sinks) {
                    auto n = nv.toObject();
                    if (!n.value("name").toString().endsWith(".monitor") && cardName(n, cards) == card && hasPort(n, portless ? QString() : p.key())) {
                        realized = true;
                        e.active = e.active || n.value("active_port").toString() == p.key();
                    }
                }
                if (e.recoverySupported && advertised.contains(c.value("active_profile").toString()) && !realized)
                    e.healthWarning = "Expected device endpoint is missing. Check the connection or try device recovery.";
                else if (c.value("active_profile").toString() == "off")
                    e.healthWarning = "Device is disabled by its card profile. Recovery can enable it.";
                // SUSPENDED is an ordinary idle state, not evidence of failure.
                out.append(e);
            }
        }
    }
    return out;
}
Discovery PulseCompatBackend::enumerateDevices() {
    discovery = Discovery(); discovery.backend = "PulseAudio-compatible";
    try {
        auto i = info(); discovery.backend = i.value("server_name").toString();
        pipeWireKnown = discovery.backend.contains("PipeWire", Qt::CaseInsensitive);
        cards = list("cards"); sinks = list("sinks"); sources = list("sources");
        discovery.endpoints = decodeDevices(cards, sinks, sources);
        discovery.capabilities = {true, true, true, true, true, false};
        discovery.status = running ? "Routing active (playback redirection experimental)" : "Ready — changes are applied only with Apply";
        discovery.status += "\nCurrent system default input: " + i.value("default_source_name").toString();
        discovery.status += "\nCurrent system default output: " + i.value("default_sink_name").toString();
        int attentionCount = 0;
        for (auto e : discovery.endpoints) if (!e.healthWarning.isEmpty()) ++attentionCount;
        if (attentionCount) discovery.status += QString("\n%1 device endpoint(s) are disabled or missing. Select a device in Device recovery to inspect recovery options.").arg(attentionCount);
        for (auto v : sinks) {
            QString owner = v.toObject().value("properties").toObject().value("caster.owner").toString();
            if (!owner.isEmpty() && owner != token) discovery.status += "\nOther Caster session/crash resources detected; left untouched.";
        }
    } catch (const std::exception &e) { discovery.error = QString::fromUtf8(e.what()); discovery.status = "Unavailable"; }
    if (!discovery.error.isEmpty() && !pipeWireKnown) {
        // A running PipeWire core can identify the restart fallback even when
        // pipewire-pulse is unreachable at application startup.
        const auto active = serviceRunner({"--user", "is-active", "pipewire.service"});
        pipeWireKnown = active.ok && active.output.trimmed() == "active";
    }
    discovery.capabilities.serviceRestart = pipeWireKnown;
    return discovery;
}
Plan PulseCompatBackend::validateRecovery(const QString &endpointId) {
    Plan plan;
    try {
        const auto d = enumerateDevices(); if (!d.error.isEmpty()) fail(d.error);
        const auto e = endpoint(endpointId);
        if (!e.recoverySupported) fail("Select a card-backed device that supports profile recovery.");
        if (running || !owned.isEmpty()) fail("Stop Caster routing before recovering hardware, then Apply your saved choices again afterwards.");
        const auto card = e.metadata.value("card").toString();
        for (auto cv : cards) {
            auto c = cv.toObject(); if (c.value("name").toString() != card) continue;
            const auto original = c.value("active_profile").toString();
            const auto profiles = named(c.value("profiles"));
            const auto targets = e.metadata.value("recoveryProfiles").toStringList();
            QString target;
            int bestPriority = std::numeric_limits<int>::min();
            const auto originalProfile = profiles.value(original).toObject();
            // Preserve the other active direction when enabling a hidden endpoint.
            const auto otherRole = e.input ? "sinks" : "sources";
            for (auto name : targets) {
                const auto candidate = profiles.value(name).toObject();
                if (originalProfile.value(otherRole).toInt() > 0 && candidate.value(otherRole).toInt() == 0) continue;
                const int priority = candidate.value("priority").toInt();
                if (target.isEmpty() || priority > bestPriority) { target = name; bestPriority = priority; }
            }
            if (targets.contains(original)) target = original;
            if (target.isEmpty() || original.isEmpty()) fail("No profile can enable this endpoint while preserving the card's other active direction.");
            QStringList cycle;
            if (original != "off") {
                if (profiles.contains("off")) cycle << "off";
                // Bounded generic equivalent of the reference script's profile cycle.
                // Try up to three alternate profiles for this direction, including
                // profiles marked unavailable (availability may be stale).
                QStringList alternatives;
                for (auto it = profiles.begin(); it != profiles.end(); ++it)
                    if (it.key() != target && it.value().toObject().value(e.input ? "sources" : "sinks").toInt() > 0) alternatives << it.key();
                std::stable_sort(alternatives.begin(), alternatives.end(), [&](const QString &a, const QString &b) {
                    return profiles.value(a).toObject().value("priority").toInt() > profiles.value(b).toObject().value("priority").toInt();
                });
                cycle += alternatives.mid(0, 3);
                if (cycle.isEmpty() && original == target) fail("This card exposes no alternative profile for a reset.");
            }
            cycle << target;
            plan.metadata = {{"recoveryEndpoint", endpointId}, {"card", card}, {"originalProfile", original}, {"cycle", cycle}};
            plan.changes << "Recover: " + e.label;
            plan.changes << "Cycle this card's advertised profiles: " + cycle.join(" → ");
            plan.changes << "Audio on this card will be interrupted. Desktop policy may move its playback streams/defaults; reselect the output afterwards if needed.";
            plan.changes << "Reported availability may be stale. This is an attempted reset, not proof of a hardware fault. No desktop audio services will be restarted.";
            return plan;
        }
        fail("The selected card has disappeared. Refresh devices.");
    } catch (const std::exception &e) { plan.error = QString::fromUtf8(e.what()); }
    return plan;
}
Result PulseCompatBackend::recoverDevice(const Plan &supplied) {
    auto plan = validateRecovery(supplied.metadata.value("recoveryEndpoint").toString());
    if (!plan.error.isEmpty()) return {false, plan.error};
    if (plan.metadata != supplied.metadata) return {false, "Device state changed since the recovery preview. Review recovery again."};
    const auto card = plan.metadata.value("card").toString();
    const auto original = plan.metadata.value("originalProfile").toString();
    const auto cycle = plan.metadata.value("cycle").toStringList();
    QString expected = original;
    QStringList diagnostics;
    bool changed = false;
    auto currentProfile = [&]() {
        for (auto v : list("cards")) if (v.toObject().value("name").toString() == card) return v.toObject().value("active_profile").toString();
        return QString();
    };
    try {
        for (const auto &profile : cycle) {
            if (currentProfile() != expected) fail("Card profile changed externally or the card disconnected; recovery stopped.");
            try {
                command({"set-card-profile", card, profile}); expected = profile; changed = true;
            } catch (const std::exception &e) {
                diagnostics << QString::fromUtf8(e.what());
                if (profile == cycle.last()) throw;
            }
            recoveryWait(1000);
        }
        for (int retry = 0; retry < 10; ++retry) {
            if (currentProfile() != expected) fail("Card profile changed externally or the card disconnected; recovery stopped.");
            auto d = enumerateDevices(); if (!d.error.isEmpty()) fail(d.error);
            const auto selected = endpoint(plan.metadata.value("recoveryEndpoint").toString());
            const auto node = resolve(selected, selected.input ? sources : sinks);
            if (!node.isEmpty() && !selected.active && !selected.metadata.value("port").toString().isEmpty()) {
                command({selected.input ? "set-source-port" : "set-sink-port", node, selected.metadata.value("port").toString()});
            }
            if (!node.isEmpty() && selected.active) {
                return {true, "Device endpoint is exposed again: " + selected.label + " (" + node + "). Test playback or recording to confirm the signal. Reapply Caster routing if needed." +
                    (diagnostics.isEmpty() ? QString() : "\nSkipped profile errors:\n" + diagnostics.join('\n'))};
            }
            recoveryWait(100);
        }
        fail("The selected device endpoint is still missing after profile cycling. Check its connection. You can try the separately confirmed audio-service restart; card-only recovery cannot verify a signal.");
    } catch (const std::exception &e) {
        QString recovery;
        if (changed) {
            try {
                if (currentProfile() == expected) {
                    if (expected != original) command({"set-card-profile", card, original});
                    recovery = currentProfile() == original ? " Original card profile restored." :
                        " The original profile could not be verified after restoration; refresh and inspect the device.";
                } else recovery = " Profile changed externally or card disconnected; the external state was left untouched.";
            } catch (const std::exception &r) { recovery = " Profile restoration failed: " + QString::fromUtf8(r.what()); }
        }
        return {false, QString::fromUtf8(e.what()) + recovery + (diagnostics.isEmpty() ? QString() : "\n" + diagnostics.join('\n'))};
    }
}
Plan PulseCompatBackend::validateServiceRestart() {
    Plan plan;
    try {
        try { pipeWireKnown = info().value("server_name").toString().contains("PipeWire", Qt::CaseInsensitive); }
        catch (const std::exception &) { /* Allow recovery of a previously detected PipeWire server. */ }
        if (!pipeWireKnown) fail("A PipeWire audio server has not been detected. This restart is not supported for native PulseAudio.");
        const QStringList units = {"wireplumber.service", "pipewire.service", "pipewire-pulse.service"};
        QStringList args = {"--user", "show", "--property=LoadState", "--value"}; args += units;
        const auto check = serviceRunner(args);
        if (!check.ok) fail("Cannot inspect user audio services: " + check.error);
        auto states = check.output.split('\n', Qt::SkipEmptyParts);
        if (states.size() != units.size()) fail("Cannot verify all three user audio services; restart cancelled.");
        for (auto state : states) if (state.trimmed() != "loaded") fail("One or more PipeWire/WirePlumber user services are not installed; restart cancelled.");
        plan.metadata = {{"serviceRestart", true}, {"units", units}, {"managedRouting", running || !owned.isEmpty()}};
        if (running || !owned.isEmpty()) plan.changes << "Caster routing will stop. You must Apply your saved choices again after the restart.";
        plan.changes << "Restart WirePlumber, PipeWire and pipewire-pulse for your desktop session.";
        plan.changes << "ALL desktop playback and recording will be interrupted, in all applications. Virtual devices created by other applications or scripts may disappear.";
        plan.changes << "Applications may need to reconnect or reselect devices. Saved Caster choices are retained but are not automatically applied. A restart does not prove the selected device works.";
    } catch (const std::exception &e) { plan.error = QString::fromUtf8(e.what()); }
    return plan;
}
Result PulseCompatBackend::restartAudioServices(const Plan &supplied) {
    const auto plan = validateServiceRestart();
    if (!plan.error.isEmpty()) return {false, plan.error, running ? boardSink : QString()};
    if (plan.metadata != supplied.metadata) return {false, "Service restart preview is stale. Review and confirm again.", running ? boardSink : QString()};
    QStringList args = {"--user", "restart"}; args += plan.metadata.value("units").toStringList();
    const auto restarted = serviceRunner(args);
    // Once a restart was attempted, stream/module state may no longer be valid.
    // On failure retain exact ownership records for conservative cleanup/retry.
    running = false;
    if (!restarted.ok) {
        healthError = "Audio service restart failed or is incomplete: " + restarted.error + "\nSome services may have restarted. Inspect your user audio services before retrying.";
        return {false, healthError};
    }
    owned.clear(); loops.clear(); boardSink.clear(); micSink.clear();
    previousSource.clear(); appliedSource.clear(); healthError.clear();
    for (int retry = 0; retry < 6; ++retry) {
        recoveryWait(500);
        const auto d = enumerateDevices();
        if (d.error.isEmpty() && pipeWireKnown) {
            healthError.clear();
            return {true, "Desktop audio services restarted and PipeWire is responding. Refresh devices, enable/recover the desired device if needed, then Apply saved routing. Test playback/recording to verify recovery."};
        }
    }
    return {false, "The service restart completed, but the audio server is still unavailable. Inspect the user audio service status. No automatic restart will be attempted."};
}
Endpoint PulseCompatBackend::endpoint(const QString &id) const {
    for (auto e : discovery.endpoints) if (e.id == id) return e;
    fail("Selected device is missing/disconnected: " + id); return {};
}
Plan PulseCompatBackend::validateConfiguration(const Configuration &desired) {
    Plan plan; plan.desired = desired;
    try {
        auto d = enumerateDevices(); if (!d.error.isEmpty()) fail(d.error);
        QVector<Endpoint> selected;
        auto output = endpoint(desired.systemOutput); if (output.input) fail("Choose a playback output."); selected << output;
        if (desired.mix) { auto e = endpoint(desired.microphone); if (!e.input) fail("Choose a physical microphone."); selected << e; }
        if (desired.monitor) { auto e = endpoint(desired.monitorOutput); if (e.input) fail("Choose a monitoring output."); selected << e; }
        QMap<QString, QStringList> choices;
        for (auto e : selected) {
            if (!e.available) fail(e.label + " is disconnected.");
            QString card = e.metadata.value("card").toString(); auto profiles = e.metadata.value("profiles").toStringList();
            if (!choices.contains(card)) choices[card] = profiles;
            else { auto previous = choices[card]; choices[card].clear(); for (auto p : previous) if (profiles.contains(p)) choices[card] << p; }
        }
        // Different ports on the same active sink cannot be selected independently.
        for (int a = 0; a < selected.size(); ++a) for (int b = a + 1; b < selected.size(); ++b) {
            if (selected[a].input != selected[b].input || selected[a].id == selected[b].id) continue;
            const auto &nodes = selected[a].input ? sources : sinks;
            QString left = resolve(selected[a], nodes), right = resolve(selected[b], nodes);
            if (!left.isEmpty() && left == right) fail("System and monitoring outputs select different ports of the same device. Choose the same port or separate devices.");
        }
        QVariantMap transitions;
        for (auto cv : cards) {
            auto c = cv.toObject(); auto name = c.value("name").toString(); if (!choices.contains(name)) continue;
            if (choices[name].isEmpty()) fail("No compatible input/output profile for " + name + ". Select another device combination.");
            auto current = c.value("active_profile").toString(); QString best = current; int priority = std::numeric_limits<int>::min();
            if (!choices[name].contains(current)) {
                auto profiles = named(c.value("profiles"));
                for (auto p : choices[name]) if (profiles.value(p).toObject().value("priority").toInt() > priority) {
                    best = p; priority = profiles.value(p).toObject().value("priority").toInt();
                }
                transitions[name] = best;
                plan.changes << name + ": " + current + " → " + best + " (temporarily interrupts this card)";
            }
        }
        plan.metadata["profiles"] = transitions;
        plan.changes << "Global system playback output: " + output.label;
        if (desired.defaultInput && desired.mix) plan.changes << "Global default input: mixed virtual microphone";
        if (desired.mix) plan.changes << "Set the receiving application's input to Default/System Default to use the mixed microphone when it is enabled as default.";
    } catch (const std::exception &e) { plan.error = QString::fromUtf8(e.what()); }
    return plan;
}
QString PulseCompatBackend::resolve(const Endpoint &e, const QJsonArray &nodes) const {
    QString found;
    for (auto nv : nodes) {
        auto n = nv.toObject(); auto name = n.value("name").toString();
        if (name.endsWith(".monitor")) continue;
        if (cardName(n, cards) == e.metadata.value("card").toString() && hasPort(n, e.metadata.value("port").toString())) {
            if (!found.isEmpty()) return {}; // Ambiguous mapping must not select arbitrarily.
            found = name;
        }
    }
    return found;
}
QString PulseCompatBackend::uniqueName(const QString &base) const {
    for (auto v : sinks) if (v.toObject().value("name").toString() == base) return base + "_" + token;
    return base;
}
QString PulseCompatBackend::load(const QString &name, const QStringList &args) {
    QStringList commandArgs = {"load-module", name}; commandArgs += args;
    auto id = command(commandArgs);
    if (!QRegularExpression("^[0-9]+$").match(id).hasMatch()) fail("Invalid module ID returned by " + name);
    owned.append({id, name, args.join(' ')}); return id;
}
void PulseCompatBackend::unload(const QString &id) {
    for (int i = 0; i < owned.size(); ++i) if (owned[i].id == id) {
        for (auto v : list("modules")) {
            auto m = v.toObject();
            if (str(m.value("index")) != id) continue;
            if (m.value("name").toString() != owned[i].name || m.value("argument").toString() != owned[i].arguments)
                fail("Module identity changed; refusing to unload " + id);
            command({"unload-module", id}); break;
        }
        owned.remove(i); return;
    }
}
void PulseCompatBackend::restoreSource() {
    if (!appliedSource.isEmpty() && info().value("default_source_name").toString() == appliedSource && !previousSource.isEmpty())
        command({"set-default-source", previousSource});
    appliedSource.clear(); previousSource.clear();
}
Result PulseCompatBackend::apply(const Plan &supplied) {
    auto plan = validateConfiguration(supplied.desired); if (!plan.error.isEmpty()) return {false, plan.error, boardSink, running && effective.mix ? micSink + ".monitor" : QString()};
    if (plan.metadata != supplied.metadata) return {false, "Devices changed since preview. Review and Apply again.", boardSink, running && effective.mix ? micSink + ".monitor" : QString()};
    if (!healthError.isEmpty()) return {false, healthError + "\nStop routing before applying again.", boardSink, {}};
    const auto desired = plan.desired;
    auto oldOwned = owned; auto oldLoops = loops; const auto oldBoard = boardSink, oldMic = micSink;
    const auto oldPrevious = previousSource, oldApplied = appliedSource;
    QJsonObject before;
    QMap<QString, QString> oldProfiles, setProfiles;
    struct PortChange { QString kind, device, before, after; };
    QVector<PortChange> portChanges;
    QString newDefaultSink, newDefaultSource;
    try {
        auto liveModules = list("modules");
        for (auto m : owned) {
            bool found = false;
            for (auto v : liveModules) {
                auto live = v.toObject();
                if (str(live.value("index")) == m.id && live.value("name").toString() == m.name && live.value("argument").toString() == m.arguments) found = true;
            }
            if (!found) fail("Managed resources changed outside Caster. Stop routing before applying again.");
        }
        before = info(); auto transitions = plan.metadata.value("profiles").toMap();
        for (auto cv : cards) {
            auto c = cv.toObject(); QString name = c.value("name").toString();
            if (transitions.contains(name)) {
                oldProfiles[name] = c.value("active_profile").toString();
                command({"set-card-profile", name, transitions[name].toString()}); setProfiles[name] = transitions[name].toString();
            }
        }
        // Profile changes can complete asynchronously on PipeWire.
        QString input, output, monitor;
        for (int retry = 0; retry < 10; ++retry) {
            auto d = enumerateDevices(); if (!d.error.isEmpty()) fail(d.error);
            output = resolve(endpoint(desired.systemOutput), sinks);
            input = desired.mix ? resolve(endpoint(desired.microphone), sources) : QString();
            monitor = desired.monitor ? resolve(endpoint(desired.monitorOutput), sinks) : QString();
            if (!output.isEmpty() && (!desired.mix || !input.isEmpty()) && (!desired.monitor || !monitor.isEmpty())) break;
            QThread::msleep(100);
        }
        if (output.isEmpty() || (desired.mix && input.isEmpty()) || (desired.monitor && monitor.isEmpty())) fail("Selected endpoint did not appear after profile activation.");
        if (desired.monitor && output == monitor && desired.systemOutput != desired.monitorOutput) fail("The selected output ports share one sink; choose the same port or separate devices.");
        auto setPort = [&](const QString &id, const QString &device, bool isInput) {
            auto e = endpoint(id); const QString port = e.metadata.value("port").toString();
            for (auto v : isInput ? sources : sinks) {
                auto n = v.toObject(); if (n.value("name").toString() != device) continue;
                QString previous = n.value("active_port").toString(); if (previous == port) return;
                QString kind = isInput ? "source" : "sink";
                command({"set-" + kind + "-port", device, port}); portChanges.append({kind, device, previous, port}); return;
            }
        };
        setPort(desired.systemOutput, output, false);
        if (desired.mix) setPort(desired.microphone, input, true);
        if (desired.monitor) setPort(desired.monitorOutput, monitor, false);
        if (boardSink.isEmpty()) {
            boardSink = uniqueName("SoundboardSink");
            load("module-null-sink", {"sink_name=" + boardSink, "sink_properties=\"device.description=Caster_Soundboard caster.owner=" + token + "\""});
        }
        if (desired.mix && micSink.isEmpty()) {
            micSink = uniqueName("VirtualMic");
            load("module-null-sink", {"sink_name=" + micSink, "sink_properties=\"device.description=Virtual_Microphone caster.owner=" + token + "\""});
        }
        QMap<QString, QString> wanted;
        auto connect = [&](const QString &source, const QString &sink) {
            const QRegularExpression safeName("^[A-Za-z0-9_.:-]+$");
            if (!safeName.match(source).hasMatch() || !safeName.match(sink).hasMatch())
                fail("Unsupported audio endpoint name; refusing to interpolate module arguments.");
            QString key = source + "|" + sink;
            wanted[key] = loops.contains(key) ? loops[key] : load("module-loopback", {"source=" + source, "sink=" + sink,
                "latency_msec=30", "source_dont_move=true", "sink_dont_move=true",
                "sink_input_properties=\"caster.owner=" + token + "\"", "source_output_properties=\"caster.owner=" + token + "\""});
        };
        if (desired.mix) { connect(boardSink + ".monitor", micSink); connect(input, micSink); }
        if (desired.monitor) connect(boardSink + ".monitor", monitor);
        if (desired.mix && desired.defaultInput) {
            newDefaultSource = micSink + ".monitor";
            if (appliedSource.isEmpty()) previousSource = before.value("default_source_name").toString();
            command({"set-default-source", newDefaultSource}); appliedSource = newDefaultSource;
        } else {
            if (!appliedSource.isEmpty() && before.value("default_source_name").toString() == appliedSource) newDefaultSource = previousSource;
            restoreSource();
        }
        newDefaultSink = output; command({"set-default-sink", output});
        bool defaultsReady = false;
        for (int retry = 0; retry < 10; ++retry) {
            auto now = info();
            defaultsReady = now.value("default_sink_name").toString() == output &&
                (newDefaultSource.isEmpty() || now.value("default_source_name").toString() == newDefaultSource);
            if (defaultsReady) break;
            QThread::msleep(50);
        }
        if (!defaultsReady) fail("Audio server did not retain the requested defaults.");
        sinks = list("sinks"); sources = list("sources");
        bool boardExists = false, voiceExists = !desired.mix;
        for (auto v : sinks) if (v.toObject().value("name").toString() == boardSink) boardExists = true;
        for (auto v : sources) if (v.toObject().value("name").toString() == micSink + ".monitor") voiceExists = true;
        if (!boardExists || !voiceExists) fail("Virtual endpoints did not appear.");
        // Commit point. Removing superseded connections is cleanup, not rollback:
        // an unload failure leaves the effective state degraded and retryable.
        loops = wanted; effective = desired; running = true;
        QStringList cleanup;
        for (auto it = oldLoops.begin(); it != oldLoops.end(); ++it) if (!wanted.contains(it.key())) {
            try { unload(it.value()); } catch (const std::exception &e) { cleanup << QString::fromUtf8(e.what()); }
        }
        // An unused owned virtual microphone is retained until Stop for reuse.
        if (!cleanup.isEmpty()) {
            healthError = "Routing changed, but old connections could not be removed. Stop routing and retry.\n" + cleanup.join('\n');
            return {false, healthError, boardSink, desired.mix ? micSink + ".monitor" : QString()};
        }
        return {true, "Routing active.", boardSink, desired.mix ? micSink + ".monitor" : QString()};
    } catch (const std::exception &e) {
        QStringList recovery;
        auto attempt = [&](std::function<void()> f) { try { f(); } catch (const std::exception &r) { recovery << QString::fromUtf8(r.what()); } };
        for (auto p : portChanges) attempt([&] {
            for (auto v : list(p.kind == "source" ? "sources" : "sinks")) {
                auto n = v.toObject(); if (n.value("name").toString() == p.device && n.value("active_port").toString() == p.after && !p.before.isEmpty()) command({"set-" + p.kind + "-port", p.device, p.before});
            }
        });
        attempt([&] { for (auto v : list("cards")) {
            auto c = v.toObject(); auto name = c.value("name").toString();
            if (setProfiles.contains(name)) {
                if (c.value("active_profile").toString() == setProfiles[name]) command({"set-card-profile", name, oldProfiles[name]});
                else recovery << "Profile changed externally; not restored: " + name;
            }
        }});
        attempt([&] { auto now = info();
            if (!newDefaultSink.isEmpty() && now.value("default_sink_name").toString() == newDefaultSink) command({"set-default-sink", before.value("default_sink_name").toString()});
            if (!newDefaultSource.isEmpty() && now.value("default_source_name").toString() == newDefaultSource) command({"set-default-source", before.value("default_source_name").toString()});
        });
        auto current = owned;
        for (int i = current.size() - 1; i >= oldOwned.size(); --i) attempt([&] { unload(current[i].id); });
        if (!recovery.isEmpty()) healthError = "Partial recovery; Stop routing and inspect:\n" + recovery.join('\n');
        boardSink = oldBoard; micSink = oldMic; loops = oldLoops; previousSource = oldPrevious; appliedSource = oldApplied;
        return {false, QString::fromUtf8(e.what()) + (recovery.isEmpty() ? "\nChanges rolled back. Refresh devices and retry." : "\nPartial recovery; Stop routing and inspect:\n" + recovery.join('\n')), running ? boardSink : QString(), running && effective.mix ? micSink + ".monitor" : QString()};
    }
}
Result PulseCompatBackend::stopManagedRouting() {
    QStringList errors;
    try { restoreSource(); } catch (const std::exception &e) { errors << QString::fromUtf8(e.what()); }
    auto modules = owned;
    for (int i = modules.size() - 1; i >= 0; --i) {
        try { unload(modules[i].id); } catch (const std::exception &e) { errors << QString::fromUtf8(e.what()); }
    }
    if (owned.isEmpty()) { loops.clear(); boardSink.clear(); micSink.clear(); running = false; healthError.clear(); }
    return {errors.isEmpty(), errors.isEmpty() ? "Routing stopped. Physical default output and card profiles retained." : "Partial cleanup:\n" + errors.join('\n'), {}, {}};
}
Result PulseCompatBackend::status() {
    if (!healthError.isEmpty()) return {false, healthError, boardSink, {}};
    auto d = enumerateDevices();
    if (!d.error.isEmpty()) return {false, d.error, {}, {}};
    if (running) {
        try {
            bool found = false;
            for (auto v : sinks) if (v.toObject().value("name").toString() == boardSink) found = true;
            if (!found) fail("Managed soundboard sink disappeared. Stop routing and Apply again.");
            if (!endpoint(effective.systemOutput).available ||
                (effective.mix && !endpoint(effective.microphone).available) ||
                (effective.monitor && !endpoint(effective.monitorOutput).available) ||
                resolve(endpoint(effective.systemOutput), sinks).isEmpty() ||
                (effective.mix && resolve(endpoint(effective.microphone), sources).isEmpty()) ||
                (effective.monitor && resolve(endpoint(effective.monitorOutput), sinks).isEmpty())) fail("A selected device is disconnected. Stop routing and reconnect, then Apply.");
        } catch (const std::exception &e) { return {false, QString::fromUtf8(e.what()), {}, {}}; }
    }
    return {true, d.status, boardSink, running && effective.mix ? micSink + ".monitor" : QString()};
}
Result PulseCompatBackend::routePlayback(qint64 processId) {
    if (!running || boardSink.isEmpty()) return {true, {}, {}, {}};
    try {
        QString index;
        for (auto v : list("sinks")) if (v.toObject().value("name").toString() == boardSink) index = str(v.toObject().value("index"));
        if (index.isEmpty()) fail("Managed soundboard sink disappeared. Stop routing and Apply again.");
        const auto streams = list("sink-inputs");
        QMap<QString, QString> clientProcesses;
        bool needClients = false;
        for (auto v : streams) {
            const auto s = v.toObject();
            if (s.value("properties").toObject().value("application.process.id").isUndefined() &&
                !s.value("client").isNull() && !s.value("client").isUndefined()) needClients = true;
        }
        // Native PipeWire/ALSA streams expose the PID on their client, not on
        // the stream. Join Pulse client indices (not PipeWire's client.id).
        if (needClients) for (auto v : list("clients")) {
            const auto c = v.toObject();
            const auto pid = c.value("properties").toObject().value("application.process.id");
            if (pid.isString() || pid.isDouble()) clientProcesses.insert(str(c.value("index")), str(pid));
        }
        for (auto v : streams) {
            const auto s = v.toObject();
            const auto pid = s.value("properties").toObject().value("application.process.id");
            const auto client = s.value("client");
            const auto owner = pid.isUndefined()
                ? (client.isString() || client.isDouble() ? clientProcesses.value(str(client)) : QString())
                : (pid.isString() || pid.isDouble() ? str(pid) : QString());
            if (owner == QString::number(processId) && str(s.value("sink")) != index)
                command({"move-sink-input", str(s.value("index")), boardSink});
        }
        return {true, {}, boardSink, {}};
    } catch (const std::exception &e) { return {false, QString::fromUtf8(e.what()), {}, {}}; }
}
std::unique_ptr<Backend> createBackend() { return std::unique_ptr<Backend>(new PulseCompatBackend); }
}
