/*
 * dialogoptions.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "dialogoptions.hpp"
#include "ui_dialogoptions.h"
#include "math.h"
#include <QFileDialog>
#include <QEvent>
#include <QKeyEvent>

/**
 * Constructor.
 *
 * @param dialog    the option dialog
 */
TableKeyPressEater::TableKeyPressEater(DialogOptions* dialog) :
   QObject(dialog), m_dialog(dialog) {
}

/**
 * Event handler for the up/down keys of the table widget.
 *
 * @param obj       the table widget
 * @param event     the current event
 * @return
 */
bool TableKeyPressEater::eventFilter(QObject* obj, QEvent* event) {
   if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast <QKeyEvent*>(event);
      int key = keyEvent->key();
      if (key == Qt::Key_Up || key == Qt::Key_Down)
         m_dialog->handleKey(key);
   }
   // standard event processing
   return QObject::eventFilter(obj, event);
}
/**
 * Constructor.
 *
 * @param handlers  the current list of the context menu handlers
 * @param parent    the widget parent
 */
DialogOptions::DialogOptions(ContextHandlerList& handlers, QWidget* parent) :
   QDialog(parent),
   ui(new Ui::DialogOptions),
   m_contextHandlers(handlers),
   m_selectedRow(-1) {
   ui->setupUi(this);
   connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
   connect(ui->pushButtonAdd, SIGNAL(clicked()), this, SLOT(add()));
   connect(ui->pushButtonDel, SIGNAL(clicked()), this, SLOT(del()));
   connect(ui->pushButtonDown, SIGNAL(clicked()), this, SLOT(down()));
   connect(ui->pushButtonSelectProgram, SIGNAL(clicked()), this,
           SLOT(selectProgram()));
   connect(ui->pushButtonUp, SIGNAL(clicked()), this, SLOT(up()));
   connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(cellEntered(int,int)));
   connect(ui->tableWidget, SIGNAL(selectionChanged()), this,
           SLOT(selectionChanged()));
   connect(ui->tableWidget, SIGNAL(currentItemChanged()), this,
           SLOT(currentItemChanged()));
   ui->tableWidget->installEventFilter(new TableKeyPressEater(this));
   for (int ix = 0; ix < ui->comboBoxFileType->count(); ix++)
      m_fileTypes.append(ui->comboBoxFileType->itemText(ix));
   for (int ix = 0; ix < ui->comboBoxDirMode->count(); ix++)
      m_dirModes.append(ui->comboBoxDirMode->itemText(ix));
   fillTable();
   if (ui->tableWidget->rowCount() > 0) {
      cellEntered(0, 0);
      selectRow(0);
   }
}

DialogOptions::~DialogOptions() {
   delete ui;
}

void DialogOptions::currentItemChanged(QTableWidgetItem* current,
                                       QTableWidgetItem* previous) {
   QString text = current->text() + " " + previous->text();
}

/**
 * Handles the event "pushed button add".
 */
void DialogOptions::add() {
   QString title = ui->lineEditTitle->text();
   int index = -1;
   int count = ui->tableWidget->rowCount();
   for (int row = 0; row < count; row++) {
      if (ui->tableWidget->item(row, COL_TITLE)->text().compare(title,
            Qt::CaseInsensitive) == 0) {
         index = row;
         break;
      }
   }
   if (index < 0) {
      ui->tableWidget->setRowCount(count + 1);
      selectRow(count);
      index = count;
   }
   currentToTable(index);
}

/**
 * Handles the event "button box accepted".
 */
void DialogOptions::accepted() {
   fromTable();
}

/**
 * Handles the event "cell entered".
 *
 * @param row       the row of the entered cell
 * @param column    the column of the entered cell
 */
void DialogOptions::cellEntered(int row, int column) {
   UNUSED_VAR(column);
   ContextHandler handler;
   m_selectedRow = row;
   fillContextHandler(row, handler);
   ui->comboBoxArgument->setCurrentText(handler.m_arguments);
   ui->comboBoxProgram->setCurrentText(handler.m_program);
   ui->lineEditTitle->setText(handler.m_text);
   ui->comboBoxDirMode->setCurrentIndex(handler.m_directoryMode - 1);
   ui->comboBoxFileType->setCurrentIndex(handler.m_fileType - 1);
}

/**
 * Copies the data of the current entry into the table.
 *
 * @param row   the row number of the table row to set
 */
void DialogOptions::currentToTable(int row) {
   ui->tableWidget->setItem(row, COL_TITLE,
                            new QTableWidgetItem(ui->lineEditTitle->text()));
   ui->tableWidget->setItem(row, COL_PROGRAM,
                            new QTableWidgetItem(ui->comboBoxProgram->currentText()));
   ui->tableWidget->setItem(row, COL_ARGUMENTS,
                            new QTableWidgetItem(ui->comboBoxArgument->currentText()));
   ui->tableWidget->setItem(row, COL_FILE_TYPE,
                            new QTableWidgetItem(ui->comboBoxFileType->currentText()));
   ui->tableWidget->setItem(row, COL_CURRENT_DIR,
                            new QTableWidgetItem(ui->comboBoxDirMode->currentText()));
}

/**
 * Handles the event "pushed button del".
 */
void DialogOptions::del() {
   int count = ui->tableWidget->rowCount() - 1;
   for (int row = m_selectedRow + 1; row <= count; row++) {
      swapRows(row, row - 1);
   }
   ui->tableWidget->setRowCount(count);
}

/**
 * Handles the event "pushed button del".
 */
void DialogOptions::down() {
   if (m_selectedRow >= 0 && m_selectedRow < ui->tableWidget->rowCount() - 1) {
      swapRows(m_selectedRow, m_selectedRow + 1);
      selectRow(m_selectedRow + 1);
   }
}

/**
 * Fills the table with the data from the list.
 */
void DialogOptions::fillTable() {
   int count = m_contextHandlers.list().size();
   int rowCount = 0;
   for (int row = 0; row < count; row++) {
      ContextHandler* handler = m_contextHandlers.list().at(row);
      if (handler->intrinsicType() == ContextHandler::IT_UNDEF)
         rowCount++;
   }
   ui->tableWidget->setRowCount(rowCount);
   QString value;
   QList < QString > dirMode;
   for (int row = 0; row < rowCount; row++) {
      ContextHandler* handler = m_contextHandlers.list().at(row);
      if (handler->intrinsicType() == ContextHandler::IT_UNDEF) {
         ui->tableWidget->setItem(row, COL_TITLE,
                                  new QTableWidgetItem(handler->m_text));
         ui->tableWidget->setItem(row, COL_PROGRAM,
                                  new QTableWidgetItem(handler->m_program));
         ui->tableWidget->setItem(row, COL_ARGUMENTS,
                                  new QTableWidgetItem(handler->m_arguments));
         value = ui->comboBoxFileType->itemText(handler->m_fileType - 1);
         ui->tableWidget->setItem(row, COL_FILE_TYPE,
                                  new QTableWidgetItem(value));
         value = ui->comboBoxDirMode->itemText(handler->m_directoryMode - 1);
         ui->tableWidget->setItem(row, COL_CURRENT_DIR,
                                  new QTableWidgetItem(value));
      }
   }
}

/**
 * Fills a handler with the data from a table row.
 *
 * @param row       the row number
 * @param handler   OUT: the handler to fill
 */
void DialogOptions::fillContextHandler(int row, ContextHandler& handler) {
   handler.m_text = ui->tableWidget->item(row, COL_TITLE)->text();
   handler.m_program = ui->tableWidget->item(row, COL_PROGRAM)->text();
   handler.m_arguments = ui->tableWidget->item(row, COL_ARGUMENTS)->text();
   int nType = 1
               + m_fileTypes.indexOf(ui->tableWidget->item(row, COL_FILE_TYPE)->text());
   handler.m_fileType = ContextHandler::FileType(nType);
   nType = 1
           + m_dirModes.indexOf(ui->tableWidget->item(row, COL_CURRENT_DIR)->text());
   handler.m_directoryMode = ContextHandler::DirMode(nType);
}

/**
 * Fills the list from the data of the table.
 */
void DialogOptions::fromTable() {
   m_contextHandlers.clear();
   int count = ui->tableWidget->rowCount();
   for (int row = 0; row < count; row++) {
      ContextHandler* handler = new ContextHandler;
      fillContextHandler(row, *handler);
      m_contextHandlers.list().append(handler);
   }
}

/**
 * Handles the up/down key press event.
 *
 * @param key   the key (up or down)
 */
void DialogOptions::handleKey(int key) {
   if (key == Qt::Key_Down) {
      if (m_selectedRow < ui->tableWidget->rowCount() - 1)
         m_selectedRow++;
   } else if (key == Qt::Key_Up) {
      if (m_selectedRow > 0)
         m_selectedRow--;
   }
}

/**
 * Deselects the current row and selects another.
 * @param row   the number of the new selected row
 */
void DialogOptions::selectRow(int row) {
   int last = ui->tableWidget->columnCount() - 1;
   QTableWidgetSelectionRange range(m_selectedRow, 0, m_selectedRow, last);
   ui->tableWidget->setRangeSelected(range, false);
   QTableWidgetSelectionRange range2(row, 0, row, last);
   ui->tableWidget->setRangeSelected(range2, true);
   m_selectedRow = row;
}

/**
 * Swaps two rows of the table.
 *
 * @param row1  the number of the first row to swap
 * @param row2  the number of the 2nd row to swap
 */
void DialogOptions::swapRows(int row1, int row2) {
   QString value1;
   QString value2;
   for (int col = COL_FILE_TYPE; col <= COL_CURRENT_DIR; col++) {
      value1 = ui->tableWidget->item(row1, col)->text();
      value2 = ui->tableWidget->item(row2, col)->text();
      ui->tableWidget->item(row1, col)->setText(value2);
      ui->tableWidget->item(row2, col)->setText(value1);
   }
}

/**
 * Handles the event "pushed button select program file".
 */
void DialogOptions::selectProgram() {
   QString file = QFileDialog::getOpenFileName(NULL, tr("Select Program File"),
                  ui->comboBoxProgram->currentText());
   if (!file.isEmpty())
      ui->comboBoxProgram->setCurrentText(file);
}

void DialogOptions::selectionChanged() {
   QList < QTableWidgetSelectionRange > selections =
      ui->tableWidget->selectedRanges();
   QList <QTableWidgetSelectionRange>::const_iterator it = selections.begin();
   if (it != selections.end()) {
      int row = (*it).topRow();
      if (row != m_selectedRow)
         selectRow(row);
   }
}
/**
 * Handles the event "pushed button up".
 */
void DialogOptions::up() {
   if (m_selectedRow > 0 && m_selectedRow < ui->tableWidget->rowCount()) {
      swapRows(m_selectedRow, m_selectedRow - 1);
      selectRow(m_selectedRow - 1);
   }
}

