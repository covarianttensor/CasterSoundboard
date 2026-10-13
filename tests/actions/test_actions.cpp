#include "actions/ActionEngine.h"
#include "actions/ActionsDialog.h"
#include "CasterBoard.h"
#include "CasterPlayer.h"
#include "CasterPlayerState.h"
#include "MainWindow.h"
#include <QTest>
#include <QBuffer>
#include <QTimer>
#include <QApplication>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QFile>
#include <QSignalBlocker>

class FakeInput : public Actions::InputChannel {
public:
    int starts = 0, opens = 0, closes = 0;
    Actions::Mode lastMode = Actions::None;
    void start(const QString &) override { ++starts; }
    void open(Actions::Mode mode) override { ++opens; lastMode = mode; }
    void close() override { ++closes; }
};
static Actions::Profile profile(Actions::Mode mode = Actions::PTT) {
    Actions::Profile p; p.name = "Game"; p.mode = mode; p.bufferMs = 5000; return p;
}
class ActionTests : public QObject {
    Q_OBJECT
private slots:
    void roundTripAndLegacy_data() {
        QTest::addColumn<int>("streamVersion");
        QTest::newRow("preserved-qt5-profile") << int(QDataStream::Qt_5_15);
        QTest::newRow("qt6-profile") << int(QDataStream::Qt_DefaultCompiledVersion);
    }
    void roundTripAndLegacy() {
        QFETCH(int, streamVersion);
        CasterBoard board; board.actionProfiles = {profile(), profile(Actions::Mute)}; board.selectedAction = 1;
        board.actionProfiles[1].muteKey="KEY_M";
        board.actionProfiles[0].attached=false;
        QByteArray bytes; QDataStream out(&bytes,QIODevice::WriteOnly);
        out.setVersion(streamVersion); out << board;
        CasterBoard loaded; QDataStream in(bytes); in >> loaded;
        QCOMPARE(in.status(),QDataStream::Ok); QCOMPARE(loaded.actionProfiles.size(),2);
        QCOMPARE(loaded.selectedAction,1); QCOMPARE(loaded.activeAction().mode,Actions::Mute);
        QCOMPARE(loaded.activeAction().key,QString("KEY_V"));
        QCOMPARE(loaded.activeAction().inputKey(),QString("KEY_M")); QVERIFY(!loaded.actionProfiles[0].attached);
        QByteArray versionOne; QDataStream v1(&versionOne,QIODevice::WriteOnly);
        v1 << quint32(0x43414354) << quint32(1) << quint32(1) << qint32(0)
           << QString("Old") << qint32(Actions::Mute) << QString("KEY_T") << qint32(150);
        QDataStream oldActions(versionOne); QVector<Actions::Profile> migrated; int active;
        Actions::read(oldActions,migrated,active);
        QCOMPARE(oldActions.status(),QDataStream::Ok); QCOMPARE(migrated[0].inputKey(),QString("KEY_T"));
        QVERIFY(migrated[0].attached);
        QByteArray legacy; QDataStream old(&legacy,QIODevice::WriteOnly);
        old.setVersion(streamVersion); old << *board.soundBoardName;
        for (auto key : board.players->keys()) old << *board.players->value(key)->playerState;
        QDataStream legacyIn(legacy); legacyIn >> loaded;
        QCOMPARE(legacyIn.status(),QDataStream::Ok); QVERIFY(loaded.actionProfiles.isEmpty()); QCOMPARE(loaded.selectedAction,-1);
        // Truncation and future extension versions cannot be silently ignored.
        bytes.chop(1); QDataStream bad(bytes); bad >> loaded; QVERIFY(bad.status()!=QDataStream::Ok);
        QByteArray future; QDataStream f(&future,QIODevice::WriteOnly); f << quint32(0x43414354) << quint32(99) << quint32(0) << qint32(-1);
        QDataStream fi(future); QVector<Actions::Profile> ps; int selected; Actions::read(fi,ps,selected);
        QCOMPARE(fi.status(),QDataStream::ReadCorruptData);
    }
    void pendingCancelAndOverlap() {
        FakeInput *input = nullptr;
        Actions::Engine engine(nullptr,[&] { input = new FakeInput; return input; });
        CasterPlayerWidget a,b;
        engine.start(&a,profile(),false); engine.start(&b,profile(),false);
        QCOMPARE(input->starts,1); QVERIFY(a.actionPending); QCOMPARE(input->opens,0);
        emit input->ready(); QCOMPARE(input->opens,1);
        emit input->opened();
        engine.cancel(&a); QCOMPARE(input->closes,0); QVERIFY(!a.actionPending);
        engine.cancel(&b); QCOMPARE(input->closes,1); QVERIFY(!b.actionPending);
        emit input->closed();
        engine.start(&a,profile(Actions::Mute),false); QCOMPARE(input->starts,1);
        QCOMPARE(input->lastMode,Actions::Mute); QCOMPARE(input->opens,2);
        engine.cancel(&a); emit input->opened(); QCOMPARE(input->closes,2);
    }
    void failureStopsAllAndCanRetry() {
        FakeInput *input = nullptr;
        Actions::Engine engine(nullptr,[&] { input = new FakeInput; return input; });
        CasterPlayerWidget a,b;
        engine.start(&a,profile(),false); engine.start(&b,profile(),false);
        emit input->failed("permission denied");
        QVERIFY(!a.actionPending); QVERIFY(!b.actionPending);
        QCOMPARE(a.actionError,QString("permission denied")); QVERIFY(!engine.contains(&a));
        engine.start(&a,profile(),false); QVERIFY(a.actionPending); QCOMPARE(input->starts,1);
        engine.cancel(&a);
    }
    void modeConflictAndSnapshot() {
        FakeInput *input = nullptr;
        Actions::Engine engine(nullptr,[&] { input = new FakeInput; return input; });
        CasterPlayerWidget a,b; auto p = profile();
        engine.start(&a,p,false); p.mode = Actions::Mute;
        engine.start(&b,p,false); QVERIFY(!engine.contains(&b)); QVERIFY(!b.actionError.isEmpty());
        emit input->ready(); QCOMPARE(input->lastMode,Actions::PTT);
        engine.cancel(&a); emit input->opened(); QCOMPARE(input->closes,1);
    }
    void destroyedPlayerCancelsPending() {
        FakeInput *input = nullptr;
        Actions::Engine engine(nullptr,[&] { input = new FakeInput; return input; });
        auto a = new CasterPlayerWidget;
        engine.start(a,profile(),false); emit input->ready(); delete a;
        emit input->opened(); QCOMPARE(input->closes,1);
    }
    void playbackCompletionAndPause() {
        QTemporaryDir dir; QFile wav(dir.filePath("silent.wav")); QVERIFY(wav.open(QIODevice::WriteOnly));
        QDataStream out(&wav); out.setByteOrder(QDataStream::LittleEndian);
        wav.write("RIFF",4); out << quint32(8036); wav.write("WAVEfmt ",8);
        out << quint32(16) << quint16(1) << quint16(1) << quint32(8000) << quint32(16000) << quint16(2) << quint16(16);
        wav.write("data",4); out << quint32(8000); wav.write(QByteArray(8000,0)); wav.close();
        FakeInput *input = nullptr;
        Actions::Engine engine(nullptr,[&] { input = new FakeInput; return input; });
        CasterPlayerWidget p; p.player->audioOutput()->setMuted(true);
        *p.playerState->filePath = wav.fileName();
        {
            // Defer metadata notifications until after the first play request.
            QSignalBlocker block(p.player);
            p.reloadFromPlayerState();
            QTRY_COMPARE(p.player->mediaStatus(), QMediaPlayer::LoadedMedia);
        }
        auto settings=profile(); settings.bufferMs=30;
        engine.start(&p,settings,false); emit input->ready(); emit input->opened();
        QVERIFY(p.actionPending); QVERIFY(p.player->state()!=QMediaPlayer::PlayingState);
        QTRY_COMPARE(p.player->state(),QMediaPlayer::PlayingState);
        p.playerMetaDataChanged();
        QCOMPARE(p.player->state(),QMediaPlayer::PlayingState);
        QTRY_VERIFY(!engine.contains(&p)); QTRY_COMPARE(input->closes,1);
        emit input->closed();
        engine.start(&p,settings,false); emit input->opened();
        QTRY_COMPARE(p.player->state(),QMediaPlayer::PlayingState);
        p.player->pause(); QTRY_VERIFY(!engine.contains(&p)); QCOMPARE(input->closes,2);
        emit input->closed();
        engine.start(&p,settings,true); emit input->opened();
        QTRY_COMPARE(p.player->state(),QMediaPlayer::PlayingState);
        QTRY_COMPARE(input->closes,3);
    }
    void dialogSelectionAndCancel() {
        CasterBoard board; board.actionProfiles = {profile(),profile(Actions::Mute)}; board.selectedAction = 0;
        QTimer::singleShot(0,[] {
            auto d = QApplication::activeModalWidget(); QVERIFY(d);
            d->findChild<QComboBox*>("actionProfiles")->setCurrentIndex(1);
            d->findChild<QLineEdit*>("actionName")->setText("Team chat");
            d->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
        });
        QVERIFY(Actions::editProfiles(&board,nullptr)); QCOMPARE(board.selectedAction,1);
        QCOMPARE(board.activeAction().name,QString("Team chat")); QVERIFY(board.actionsDirty);
        QTimer::singleShot(0,[] {
            auto d = QApplication::activeModalWidget();
            d->findChild<QComboBox*>("actionProfiles")->setCurrentIndex(0);
            d->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Cancel)->click();
        });
        QVERIFY(!Actions::editProfiles(&board,nullptr)); QCOMPARE(board.selectedAction,1);
    }
    void explicitTabAssignment() {
        QTabWidget tabs;
        auto first = new CasterBoard; auto second = new CasterBoard;
        *first->soundBoardName="First"; *second->soundBoardName="Second";
        tabs.addTab(first,"First"); tabs.addTab(second,"Second");
        first->actionProfiles={profile()}; first->selectedAction=0;
        QTimer::singleShot(0,[] {
            auto d=QApplication::activeModalWidget();
            d->findChild<QPushButton*>("addAction")->click();
            d->findChild<QComboBox*>("actionTab")->setCurrentIndex(2);
            d->findChild<QLineEdit*>("actionName")->setText("Second tab voice");
            d->grab().save("/tmp/caster-actions-dialog.png");
            d->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
        });
        QVERIFY(Actions::editProfiles(first,nullptr,&tabs));
        QCOMPARE(first->activeAction().name,QString("Game")); QVERIFY(!first->actionsDirty);
        QCOMPARE(second->activeAction().name,QString("Second tab voice")); QVERIFY(second->actionsDirty);
        QCOMPARE(tabs.currentWidget(),static_cast<QWidget*>(second));
    }
    void unattachedAndModeKeys() {
        CasterBoard board; board.actionProfiles={profile()}; board.selectedAction=0;
        QTimer::singleShot(0,[] {
            auto d=QApplication::activeModalWidget();
            auto target=d->findChild<QComboBox*>("actionTab");
            QCOMPARE(target->itemText(0),QString("None")); target->setCurrentIndex(0);
            auto mode=d->findChild<QComboBox*>("talkMode"); auto key=d->findChild<QLineEdit*>("actionKey");
            key->setText("KEY_T");
            mode->setCurrentIndex(mode->findData(int(Actions::Mute))); key->setText("KEY_M");
            mode->setCurrentIndex(mode->findData(int(Actions::PTT))); QCOMPARE(key->text(),QString("KEY_T"));
            mode->setCurrentIndex(mode->findData(int(Actions::None)));
            mode->setCurrentIndex(mode->findData(int(Actions::Mute))); QCOMPARE(key->text(),QString("KEY_M"));
            QTest::qWait(50); // Let posted layout requests from mode changes settle.
            d->adjustSize();
            d->grab().save("/tmp/caster-actions-unattached.png");
            d->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
        });
        QVERIFY(Actions::editProfiles(&board,nullptr));
        QCOMPARE(board.actionProfiles.size(),1); QVERIFY(!board.hasAttachedActions());
        QCOMPARE(board.activeAction().mode,Actions::None);
        QCOMPARE(board.actionProfiles[0].key,QString("KEY_T")); QCOMPARE(board.actionProfiles[0].muteKey,QString("KEY_M"));
        QByteArray bytes; QDataStream out(&bytes,QIODevice::WriteOnly);
        out << board;
        CasterBoard loaded; QDataStream in(bytes); in >> loaded;
        QVERIFY(!loaded.hasAttachedActions()); QCOMPARE(loaded.actionProfiles.size(),1);
        QCOMPARE(loaded.actionProfiles[0].muteKey,QString("KEY_M"));
        QTimer::singleShot(0,[] {
            auto d=QApplication::activeModalWidget();
            QCOMPARE(d->findChild<QComboBox*>("actionTab")->currentIndex(),0);
            d->findChild<QComboBox*>("actionTab")->setCurrentIndex(1);
            d->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
        });
        QVERIFY(Actions::editProfiles(&loaded,nullptr)); QVERIFY(loaded.hasAttachedActions());
        QCOMPARE(loaded.activeAction().inputKey(),QString("KEY_M"));
    }
    void unclutteredPanelAndCloseGuard() {
        QTemporaryDir dir; QString path=dir.filePath("board.caster");
        { CasterBoard b; b.actionProfiles={profile()}; b.selectedAction=0;
          QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); QDataStream stream(&file); stream << b; }
        MainWindow window; QString error; QVERIFY2(window.loadProfile(path,error),qPrintable(error));
        auto tabs=window.findChild<QTabWidget*>(); auto board=qobject_cast<CasterBoard*>(tabs->currentWidget());
        window.mainTabContainerTabClosedRequested(0); QCOMPARE(tabs->count(),1);
        QTimer::singleShot(0,[] {
            auto d=QApplication::activeModalWidget();
            d->findChild<QLineEdit*>("actionName")->setText("Updated");
            d->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
        });
        window.findChild<QPushButton*>("actionsButton")->click();
        QVERIFY(board->actionsDirty);
        QVERIFY(!window.findChild<QLabel*>("actionsSummary"));
        QVERIFY(!window.findChild<QLabel*>("actionsError"));
        window.saveTab(); QVERIFY(!board->actionsDirty);
        QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); QDataStream stream(&file); CasterBoard loaded; stream >> loaded;
        QCOMPARE(loaded.activeAction().name,QString("Updated"));
        QVERIFY(!window.findChild<QLabel*>("actionsSummary"));
    }
};
QTEST_MAIN(ActionTests)
#include "test_actions.moc"
