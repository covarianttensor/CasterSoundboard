#include "ActionsDialog.h"
#include "../CasterBoard.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QGuiApplication>
#include <QLibrary>
#include <QTabWidget>

namespace {
class KeyEdit : public QLineEdit {
public:
    using QLineEdit::QLineEdit;
    void keyPressEvent(QKeyEvent *event) override {
        if (event->isAutoRepeat()) return;
        // Qt's XCB and Wayland/XKB plugins expose evdev codes with an offset of 8.
        // Retain the physical key, including right modifiers and German Y/Z.
        const auto platform = QGuiApplication::platformName();
        if ((platform == "xcb" || platform.startsWith("wayland")) && event->nativeScanCode() >= 8) {
            QLibrary evdev("libevdev.so.2");
            using Name = const char *(*)(unsigned int, unsigned int);
            auto name = reinterpret_cast<Name>(evdev.resolve("libevdev_event_code_get_name"));
            const auto code = event->nativeScanCode() - 8;
            if (name && code > 0 && code < 249) {
                auto value = name(1, code);
                if (value) { setText(QString::fromLatin1(value)); event->accept(); return; }
            }
        }
        int k = event->key(); QString key;
        if ((k >= Qt::Key_A && k <= Qt::Key_Z) || (k >= Qt::Key_0 && k <= Qt::Key_9)) key = QChar(k);
        else if (k >= Qt::Key_F1 && k <= Qt::Key_F24) key = "F" + QString::number(k - Qt::Key_F1 + 1);
        else {
            static const QMap<int, QString> keys = {
                {Qt::Key_Space,"SPACE"}, {Qt::Key_Control,"LEFTCTRL"}, {Qt::Key_Shift,"LEFTSHIFT"},
                {Qt::Key_Alt,"LEFTALT"}, {Qt::Key_Meta,"LEFTMETA"}, {Qt::Key_Return,"ENTER"},
                {Qt::Key_Enter,"KPENTER"}, {Qt::Key_Tab,"TAB"}, {Qt::Key_Escape,"ESC"},
                {Qt::Key_Backspace,"BACKSPACE"}, {Qt::Key_Up,"UP"}, {Qt::Key_Down,"DOWN"},
                {Qt::Key_Left,"LEFT"}, {Qt::Key_Right,"RIGHT"}, {Qt::Key_Insert,"INSERT"},
                {Qt::Key_Delete,"DELETE"}, {Qt::Key_Home,"HOME"}, {Qt::Key_End,"END"},
                {Qt::Key_PageUp,"PAGEUP"}, {Qt::Key_PageDown,"PAGEDOWN"}
            };
            key = keys.value(k);
        }
        if (!key.isEmpty()) setText("KEY_" + key);
        event->accept();
    }
};
}
bool Actions::editProfiles(CasterBoard *board, QWidget *parent, QTabWidget *tabs) {
    QDialog dialog(parent); dialog.setWindowTitle("Actions"); dialog.setMinimumWidth(500);
    QVector<CasterBoard*> boards;
    if (tabs) for (int i=0; i<tabs->count(); ++i) {
        auto b=qobject_cast<CasterBoard*>(tabs->widget(i)); if (b) boards.append(b);
    }
    if (!boards.contains(board)) boards.append(board);
    struct Entry { Profile profile; CasterBoard *owner; CasterBoard *target; bool selected; };
    QVector<Entry> entries;
    int initial = -1;
    for (auto b : boards) for (int i=0; i<b->actionProfiles.size(); ++i) {
        auto p=b->actionProfiles[i];
        if (b==board && (initial<0 || i==b->selectedAction)) initial=entries.size();
        entries.append({p,b,p.attached ? b : nullptr,i==b->selectedAction});
    }
    auto layout=new QVBoxLayout(&dialog);
    auto note=new QLabel("Configure an Action and associate the Action to a tab. "
                         "The Action runs with every tab event (when a sound plays). "
                         "The application window in focus at the time will receive the Action sequence.");
    note->setWordWrap(true); layout->addWidget(note);
    auto row=new QHBoxLayout;
    auto profiles=new QComboBox; profiles->setObjectName("actionProfiles"); row->addWidget(profiles,1);
    auto add=new QPushButton("Add"); add->setObjectName("addAction");
    auto remove=new QPushButton("Remove"); row->addWidget(add); row->addWidget(remove); layout->addLayout(row);
    auto form=new QGridLayout; form->setColumnStretch(1,1); layout->addLayout(form);
    auto target=new QComboBox; target->setObjectName("actionTab"); target->addItem("None");
    for (auto b : boards) target->addItem(*b->soundBoardName);
    form->addWidget(new QLabel("Pin to tab"),1,0); form->addWidget(target,1,1);
    auto name=new QLineEdit; name->setMaxLength(100); name->setObjectName("actionName");
    auto mode=new QComboBox; mode->setObjectName("talkMode");
    mode->addItem("Push To Talk",int(PTT)); mode->addItem("Mute",int(Mute)); mode->addItem("None",int(None));
    auto key=new KeyEdit; key->setObjectName("actionKey"); key->setReadOnly(true);
    key->setToolTip("Click here and press a single key. PTT and Mute remember separate keys.");
    auto buffer=new QSpinBox; buffer->setObjectName("actionBuffer"); buffer->setRange(0,5000); buffer->setSuffix(" ms");
    for (auto field : QVector<QWidget*>{name,mode,target,key,buffer})
        field->setMinimumHeight(field->sizeHint().height());
    auto keyLabel=new QLabel("Key (press to capture)"); auto bufferLabel=new QLabel("Buffer before / after");
    form->addWidget(new QLabel("Name"),0,0); form->addWidget(name,0,1);
    form->addWidget(new QLabel("Talk Mode"),2,0); form->addWidget(mode,2,1);
    form->addWidget(keyLabel,3,0); form->addWidget(key,3,1);
    form->addWidget(bufferLabel,4,0); form->addWidget(buffer,4,1);
    QWidget::setTabOrder(name,target); QWidget::setTabOrder(target,mode);
    auto warning=new QLabel("Mute assumes the game starts muted. Its key toggles it back afterward.");
    warning->setWordWrap(true); layout->addWidget(warning);
    auto buttons=new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel); layout->addWidget(buttons);
    int current=-1;
    Mode displayedMode=None;
    auto storeKey=[&] {
        if (current<0) return;
        if (displayedMode==PTT) entries[current].profile.key=key->text();
        else if (displayedMode==Mute) entries[current].profile.muteKey=key->text();
    };
    auto store=[&] {
        if (current<0) return;
        storeKey(); auto &e=entries[current];
        e.profile.name=name->text().trimmed(); e.profile.mode=Mode(mode->currentData().toInt());
        e.profile.bufferMs=buffer->value();
        e.target=target->currentIndex()>0 ? boards[target->currentIndex()-1] : nullptr;
        e.profile.attached=e.target!=nullptr;
    };
    auto visibility=[&] {
        bool enabled=current>=0;
        name->setEnabled(enabled); mode->setEnabled(enabled); target->setEnabled(enabled); remove->setEnabled(enabled);
        bool input=enabled && displayedMode!=None;
        key->setVisible(input); keyLabel->setVisible(input); buffer->setVisible(input); bufferLabel->setVisible(input);
        warning->setVisible(input && displayedMode==Mute);
        form->invalidate();
        layout->invalidate();
        layout->activate();
        dialog.setMinimumHeight(layout->sizeHint().height());
        dialog.adjustSize();
    };
    auto load=[&](int index) {
        current=index;
        QSignalBlocker modeBlock(mode), targetBlock(target);
        if (index>=0) {
            auto e=entries[index]; name->setText(e.profile.name); buffer->setValue(e.profile.bufferMs);
            displayedMode=e.profile.mode; mode->setCurrentIndex(mode->findData(int(displayedMode)));
            key->setText(e.profile.inputKey()); target->setCurrentIndex(e.target ? boards.indexOf(e.target)+1 : 0);
        } else { name->clear(); displayedMode=None; }
        visibility();
    };
    auto rebuild=[&](int index) {
        QSignalBlocker block(profiles); profiles->clear();
        for (auto e : entries) profiles->addItem(e.profile.name);
        profiles->setCurrentIndex(index); load(index);
    };
    QObject::connect(profiles,QOverload<int>::of(&QComboBox::currentIndexChanged),&dialog,[&](int i){store();load(i);});
    QObject::connect(mode,QOverload<int>::of(&QComboBox::currentIndexChanged),&dialog,[&](int){
        storeKey(); displayedMode=Mode(mode->currentData().toInt());
        if (current>=0) key->setText(displayedMode==Mute ? entries[current].profile.muteKey : entries[current].profile.key);
        visibility();
    });
    QObject::connect(target,QOverload<int>::of(&QComboBox::currentIndexChanged),&dialog,[&](int){visibility();});
    QObject::connect(name,&QLineEdit::textEdited,&dialog,[&](QString text){
        if (current>=0) profiles->setItemText(current,text);
    });
    QObject::connect(add,&QPushButton::clicked,&dialog,[&]{
        store(); Profile p; p.name="Action "+QString::number(entries.size()+1); p.mode=PTT;
        entries.append({p,board,board,false}); rebuild(entries.size()-1);
    });
    QObject::connect(remove,&QPushButton::clicked,&dialog,[&]{
        if (current<0) return;
        entries.remove(current); rebuild(entries.isEmpty() ? -1 : qMin(current,entries.size()-1));
    });
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{
        store(); QMap<CasterBoard*,int> counts;
        for (auto e : entries) if (!valid(e.profile) || ++counts[e.target ? e.target : e.owner]>100) {
            QMessageBox::warning(&dialog,"Actions","Each Action needs a name and valid keys. A Caster profile can store up to 100 Actions."); return;
        }
        dialog.accept();
    });
    rebuild(initial);
    if (dialog.exec()!=QDialog::Accepted) return false;
    QMap<CasterBoard*,QVector<Profile>> saved;
    QMap<CasterBoard*,int> selected;
    for (auto b : boards) selected[b]=-1;
    for (int i=0;i<entries.size();++i) {
        auto e=entries[i]; auto owner=e.target ? e.target : e.owner;
        if (e.selected && e.owner==owner) selected[owner]=saved[owner].size();
        saved[owner].append(e.profile);
    }
    if (current>=0) {
        auto e=entries[current]; auto owner=e.target ? e.target : e.owner;
        // Select the exact entry, even when names/settings are duplicated.
        int index=0;
        for (int i=0;i<current;++i) if ((entries[i].target ? entries[i].target : entries[i].owner)==owner) ++index;
        selected[owner]=index;
        board=owner;
    }
    for (auto b : boards) if (b->actionProfiles!=saved[b] || b->selectedAction!=selected[b]) {
        b->actionProfiles=saved[b]; b->selectedAction=selected[b]; b->actionsDirty=true;
    }
    if (tabs) tabs->setCurrentWidget(board);
    return true;
}
