/*
 * StartView.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_startview.h"

/**
 * Constructor.
 *
 * @param mainWindow	the main window, the parent
 */
StartView::StartView(MainWindow* mainWindow) :
   QWidget(NULL),
   View("StartView", mainWindow),
   ui(new Ui::StartView) {
   ui->setupUi(this);
   connect(ui->toolButtonSelectFile, SIGNAL(clicked()), this,
           SLOT(selectFile()));
   connect(ui->toolButtonSelectProject, SIGNAL(clicked()), this,
           SLOT(selectDir()));
   connect(ui->pushButtonOpen, SIGNAL(clicked()), this, SLOT(open()));
   connect(ui->lineEditFilterLastFile, SIGNAL(textChanged(QString)), this,
           SLOT(textChangedFilterFiles(QString)));
   connect(ui->tableWidgetFiles, SIGNAL(cellEntered(int,int)), this,
           SLOT(cellEnteredFiles(int, int)));
   connect(ui->tableWidgetProjects, SIGNAL(cellEntered(int,int)), this,
           SLOT(cellEnteredProjects(int, int)));
   Workspace* workspace = mainWindow->workspace();
   buildTableInfo(workspace, Workspace::KEY_HISTORY_FILES, true, m_files);
   buildTableInfo(workspace, Workspace::KEY_HISTORY_PROJECTS, false,
                  m_projects);
   buildTable("", m_files, ui->tableWidgetFiles);
   buildTable("", m_projects, ui->tableWidgetProjects);
}

/**
 * Destructor.
 */
StartView::~StartView() {
   delete ui;
}

/**
 * Handles the event cellEntered for the last opened files.
 *
 * @param row	the row of the entered cell
 * @param col	the column of the entered cell
 */
void StartView::cellEnteredFiles(int row, int col) {
   ReUseParameter(col);
   QString file = fileOfTable(ui->tableWidgetFiles, row);
   ui->lineEditOpen->setText(file);
}

/**
 * Handles the event cellEntered for the last opened projects.
 *
 * @param row	the row of the entered cell
 * @param col	the column of the entered cell
 */
void StartView::cellEnteredProjects(int row, int col) {
   ReUseParameter(col);
   QString file = fileOfTable(ui->tableWidgetProjects, row);
   ui->lineEditOpen->setText(file);
}

/**
 * Builds the table from the table using a filter expression.
 *
 * @param filter	a filter expression with wildcards '*'
 * @param lines		the full table info
 * @param table		OUT: will be filled with all lines matching the filter
 */
void StartView::buildTable(const QString& filter, const QStringList& lines,
                           QTableWidget* table) {
   QStringList::const_iterator it;
   int rowCount = 0;
   ReMatcher matcher(filter, Qt::CaseInsensitive, true);
   for (it = lines.cbegin(); it != lines.cend(); ++it) {
      if (matcher.matches(*it))
         rowCount++;
   }
   table->setRowCount(rowCount);
   int row = -1;
   for (it = lines.cbegin(); it != lines.cend(); ++it) {
      if (matcher.matches(*it)) {
         row++;
         QStringList cols = it->split('\t');
         for (int col = 0; col < cols.size(); col++) {
            QTableWidgetItem* item = table->item(row, col);
            if (item != NULL)
               item->setText(cols.at(col));
            else {
               item = new QTableWidgetItem(cols.at(col));
               table->setItem(row, col, item);
            }
         }
      }
   }
}

/**
 * Build the info for a table (last opened files or last opened projects).
 *
 * Note: the table shows a filtered part of this info.
 *
 * @param settings		the history container
 * @param key			the name of the history entry
 * @param withDate		the file's name is part of the info
 * @param tableContent	OUT: the list containing the table info
 */
void StartView::buildTableInfo(ReSettings* settings, const char* key,
                               bool withDate, QStringList& tableContent) {
   QStringList files;
   settings->historyAsList(key, files);
   QStringList::const_iterator it;
   for (it = files.cbegin(); it != files.cend(); ++it) {
      QFileInfo file(*it);
      if (file.exists()) {
         QString info = file.fileName();
         if (withDate)
            info.append("\t").append(
               file.lastModified().toString("yyyy.mm.dd/HH:MM:SS"));
         info.append("\t").append(file.path());
         tableContent.append(info);
      }
   }
}

/**
 * Shows an error message.
 *
 * @param message	message to show
 */
void StartView::error(const QString& message) {
   QMessageBox dialog(QMessageBox::Critical, "Error", message,
                      QMessageBox::Close);
   dialog.exec();
}

/**
 * Extracts the full filename of a given table.
 *
 * The node is the first column, the path the last.
 *
 * @param table	the table from which the filename is taken
 * @param row	the row where the filename is
 * @return	the full name of the file in the given row
 */
QString StartView::fileOfTable(QTableWidget* table, int row) {
   int colPath = table->columnCount() - 1;
   QString file = table->item(row, colPath)->text() + OS_SEPARATOR_STR
                  + table->item(row, 0)->text();
   return file;
}
/**
 * Opens a file or a directory (project directory).
 */
void StartView::open() {
   QString name = ui->lineEditOpen->text();
   if (name.isEmpty())
      error("missing filename/project directory");
   else {
      QFileInfo file(name);
      if (!file.exists())
         error("does not exists: " + name);
      else {
         if (file.isDir())
            m_mainWindow->perspectives().project(true)->openFile(name);
         else
            m_mainWindow->openFile(name);
         close();
      }
   }
}

/**
 * Selects a directory (project directory) with an open dialog.
 */
void StartView::selectDir() {
   QString name = ui->lineEditOpen->text();
   if (name.isEmpty())
      name = m_mainWindow->perspectives().project()->path();
   name = QFileDialog::getExistingDirectory(this,
          tr("Select Project Directory"), name);
   if (!name.isEmpty()) {
      ui->lineEditOpen->setText(name);
      open();
   }
}

/**
 * Selects a file with a file open dialog.
 */
void StartView::selectFile() {
   QString name = ui->lineEditOpen->text();
   name = QFileDialog::getOpenFileName(this, tr("Select File"), name);
   if (!name.isEmpty()) {
      ui->lineEditOpen->setText(name);
      open();
   }
}

/**
 * Handles the filter text change for a given table.
 *
 * @param text	the filter text
 * @param table	the table which will be filled
 * @param lines	the full (unfiltered) table info
 */
void StartView::textChanged(const QString& text, QTableWidget* table,
                            const QStringList& lines) {
   buildTable(text, lines, table);
   if (table->rowCount() > 0) {
      QString file = fileOfTable(table, 0);
      ui->lineEditOpen->setText(file);
   }
}

/**
 * Handles the event "text changed" of the last opened files.
 *
 * @param text	the new text
 */
void StartView::textChangedFilterFiles(const QString& text) {
   textChanged(text, ui->tableWidgetFiles, m_files);
}

/**
 * Handles the event "text changed" of the last opened projects.
 *
 * @param text	the new text
 */
void StartView::textChangedFilterProjects(const QString& text) {
   textChanged(text, ui->tableWidgetProjects, m_projects);
}
/**
 * Returns the view specific widget.
 *
 * @return	the view specific widget
 */
QWidget* StartView::widget() {
   return this;
}
