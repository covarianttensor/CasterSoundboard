#pragma once
#include <QDialog>
class CasterBoard;
class QTabWidget;
namespace Actions {
// Edits a working copy. Cancel leaves the board untouched.
bool editProfiles(CasterBoard *board, QWidget *parent, QTabWidget *tabs = nullptr);
}
