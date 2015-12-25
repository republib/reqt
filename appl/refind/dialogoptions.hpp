/*
 * dialogoptions.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef DIALOGOPTIONS_HPP
#define DIALOGOPTIONS_HPP
#include "utils.hpp"
#include <QDialog>

namespace Ui {
class DialogOptions;
}

class DialogOptions;
class TableKeyPressEater: public QObject {
   Q_OBJECT
public:
   TableKeyPressEater(DialogOptions* dialog);
protected:
   bool eventFilter(QObject* obj, QEvent* event);
private:
   DialogOptions* m_dialog;
};

class DialogOptions: public QDialog {
   Q_OBJECT

public:
   explicit DialogOptions(ContextHandlerList& handlers, QWidget* parent = 0);
   ~DialogOptions();
private slots:
   void accepted();
   void add();
   void cellEntered(int row, int column);
   void del();
   void down();
   void selectProgram();
   void selectionChanged();
   void up();
public:
   void handleKey(int key);
private:
   void currentToTable(int row);
   void currentItemChanged(QTableWidgetItem* current,
                           QTableWidgetItem* previous);
   void fillContextHandler(int row, ContextHandler& handler);
   void fillTable();
   void fromTable();
   void selectRow(int row);
   void swapRows(int row1, int row2);
private:
   Ui::DialogOptions* ui;
   ContextHandlerList& m_contextHandlers;
   int m_selectedRow;
   QList <QString> m_fileTypes;
   QList <QString> m_dirModes;

};

#endif // DIALOGOPTIONS_HPP
