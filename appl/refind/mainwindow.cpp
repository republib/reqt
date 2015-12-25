/*
 * mainwindow.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "gui/regui.hpp"
#include "utils.hpp"
#include "textfinder.hpp"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "dialogfileplaceholder.hpp"
#include "dialogglobalplaceholder.hpp"
#include "filefinder.hpp"
#include "aboutdialog.hpp"
#include "dialogoptions.hpp"
#include <QDir>
#include <QFileDialog>
#include <QClipboard>
#include <QLineEdit>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QProcess>
#include <QMessageBox>

const QString VERSION("2015.12.24");

inline QString addEsc(const QString& text) {
   QString rc = text;
#if defined WIN32
   rc.replace("\\", "\\\\");
#endif
   return rc;
}

/**
 * @brief Constructor.
 *
 * @param startDir	the current directory at program start
 * @param homeDir	the base directory for the configuration files
 * @param parent    NULL or the parent widget
 */
MainWindow::MainWindow(const QString& startDir, const QString& homeDir,
                       QWidget* parent) :
   QMainWindow(parent),
   ui(new Ui::MainWindow),
   m_statusMessage(NULL),
   m_stdLabelBackgroundRole(NULL),
   m_textFinder(),
   m_lastBaseDir(),
   m_horizontalHeader(NULL),
   m_lastOrder(Qt::DescendingOrder),
   m_homeDir(homeDir),
   m_storageFile(),
   m_contextHandlers(),
   m_logger(new ReMemoryLogger()),
   m_finder(NULL),
   m_guiQueue(),
   m_guiTimer(new QTimer(this)) {
   ui->setupUi(this);
   initializeHome();
   m_statusMessage = new QLabel(tr("Welcome at refind"));
   if (!startDir.isEmpty())
      ui->comboBoxDirectory->setCurrentText(startDir);
   if (ui->comboBoxDirectory->currentText().isEmpty())
      ui->comboBoxDirectory->setCurrentText(QDir::currentPath());
   ui->tableWidget->setMainWindow(this);
   statusBar()->addWidget(m_statusMessage);
   connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(search()));
   connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(clear()));
   connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(stop()));
   connect(ui->pushButtonSearch, SIGNAL(clicked()), this, SLOT(search()));
   connect(ui->pushButtonSearch2, SIGNAL(clicked()), this, SLOT(search()));
   connect(ui->pushButtonStop, SIGNAL(clicked()), this, SLOT(stop()));
   connect(ui->pushButtonStop2, SIGNAL(clicked()), this, SLOT(stop()));
   connect(ui->pushButtonClear, SIGNAL(clicked()), this, SLOT(clear()));
   connect(ui->actionUp, SIGNAL(triggered()), this, SLOT(up()));
   connect(ui->pushButtonUp, SIGNAL(clicked()), this, SLOT(up()));
   connect(ui->actionSelectDirectory, SIGNAL(triggered()), this,
           SLOT(selectDirectory()));
   connect(ui->actionSaveProgramState, SIGNAL(triggered()), this,
           SLOT(saveState()));
   connect(ui->pushButtonDirectory, SIGNAL(clicked()), this,
           SLOT(selectDirectory()));
   connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
   connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(options()));
   connect(ui->actionGetAbsPath, SIGNAL(triggered()), this,
           SLOT(absPathToClipboard()));
   connect(ui->actionGetFullName, SIGNAL(triggered()), this,
           SLOT(fullNameToClipboard()));
   connect(ui->actionGetFullName, SIGNAL(triggered()), this,
           SLOT(fullNameToClipboard()));
   connect(ui->actionReset, SIGNAL(triggered()), this, SLOT(resetParameters()));
   connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(exportFiles()));
   connect(ui->pushButtonExport, SIGNAL(clicked()), this, SLOT(exportFiles()));
   connect(ui->pushButtonExportFile, SIGNAL(clicked()), this,
           SLOT(selectExportFile()));
   connect(ui->pushButtonFilePlaceholder, SIGNAL(clicked()), this,
           SLOT(filePlaceholder()));
   connect(ui->pushButtonHeaderPlaceholder, SIGNAL(clicked()), this,
           SLOT(headerPlaceholder()));
   connect(ui->pushButtonFooterPlaceholder, SIGNAL(clicked()), this,
           SLOT(footerPlaceholder()));
   connect(ui->actionPreview, SIGNAL(triggered()), this, SLOT(preview()));
   connect(ui->pushButtonPreview, SIGNAL(clicked()), this, SLOT(preview()));
   m_horizontalHeader = ui->tableWidget->horizontalHeader();
   connect(m_horizontalHeader, SIGNAL(sectionClicked ( int ) ),
           this, SLOT(headerClicked ( int ) ));
   connect(m_guiTimer, SIGNAL(timeout()), this, SLOT(guiTimerUpdate()));
   ui->tableWidget->setColumnWidth(TC_NODE, 200);
   ui->tableWidget->setColumnWidth(TC_EXT, 40);
   ui->tableWidget->setColumnWidth(TC_SIZE, 125);
   ui->tableWidget->setColumnWidth(TC_MODIFIED, 175);
   ui->tableWidget->setColumnWidth(TC_TYPE, 75);
   prepareContextMenu();
   m_guiTimer->start(100);
   bool active = m_guiTimer->isActive();
   active = ! active;
}

/**
 * @brief Destructor.
 */
MainWindow::~MainWindow() {
   delete ui;
}

/**
 * initializeHomeializes the program home directory.
 */
void MainWindow::initializeHome() {
   if (m_homeDir.isEmpty()) {
      m_homeDir = QDir::home().absoluteFilePath(".refind");
   }
   QDir home(m_homeDir);
   if (!home.exists()) {
      if (!home.mkpath(m_homeDir)) {
         m_homeDir = home.tempPath() + "/.refind";
         home.mkpath(m_homeDir);
      }
   }
   if (!m_homeDir.endsWith("/"))
      m_homeDir += "/";
   m_storageFile = m_homeDir + "state.conf";
   restoreState();
}

/**
 * Starts the about dialog.
 */
void MainWindow::about() {
   AboutDialog dialog(VERSION);
   dialog.exec();
}

/**
 * Starts the options dialog.
 */
void MainWindow::options() {
   DialogOptions dialog(m_contextHandlers, NULL);
   dialog.exec();
}

/**
 * Puts the absolute path of the current (selected) file into the clipboard.
 */
void MainWindow::absPathToClipboard() {
   int row = ui->tableWidget->currentRow();
   if (row >= 0) {
      QClipboard* clipboard = QApplication::clipboard();
      clipboard->setText(buildAbsPath(row));
   }
}

/**
 * Puts the base directory into the clipboard.
 */
void MainWindow::baseDirToClipboard() {
   QClipboard* clipboard = QApplication::clipboard();
   clipboard->setText(m_lastBaseDir.absolutePath());
}

/**
 * Gets the absolute path of the file in the given row.
 *
 * @param row       the row number
 * @param withNode  true: the node will be appended to the result
 * @param uriFormat true: example: file:///u:/tmp/file.txt<br>
 *                  false: example: u:\tmp\file.txt
 * @return          the absolute path of the file given by the row
 */
QString MainWindow::buildAbsPath(int row, bool withNode, bool uriFormat) {
   QString rc = cellAsText(row, TC_PATH);
   if (withNode) {
      ReQStringUtils::ensureLastChar(rc, '/');
      rc += cellAsText(row, TC_NODE);
   }
   if (uriFormat) {
      rc = "file://" + rc;
#if defined WIN32
      rc = rc.replace('\\', '/');
#endif
   }
   return rc;
}

/**
 * Converts the checkbox states to a filetype mask.
 * @return the filetypes selected by the checkboxes
 */
QDir::Filters MainWindow::buildFileTypes() {
   QDir::Filters rc = 0;
   if (ui->checkBoxDirs->isChecked())
      rc |= QDir::Dirs;
   if (ui->checkBoxFiles->isChecked())
      rc |= QDir::Files;
   if (rc == 0)
      rc |= QDir::Dirs | QDir::Files;
   if (!ui->checkBoxLinks->isChecked())
      rc |= QDir::NoSymLinks;
   return rc;
}

/**
 * Gets the content of the given cell as string.
 *
 * @param row   the row number: 0..R-1
 * @param col   the column number: 0..C-1
 * @return      the text of the given cell
 */
QString MainWindow::cellAsText(int row, int col) {
   QTableWidgetItem* widget = ui->tableWidget->item(row, col);
   QString rc;
   if (widget != NULL)
      rc = widget->text();
   return rc;
}

/**
 * Replaces the esc sequences like '\n'.
 *
 * @param text  the text to convert
 * @return      <code>text</code> with the esc sequences replaced
 */
QString replaceEscSequences(const QString& text) {
   QString rc = text;
   int start = 0;
   while (start < rc.length()) {
      int ix = text.indexOf('\\', start);
      if (ix < 0)
         break;
      start += ix;
      QChar replacement = 0;
      switch (text[start + 1].toLatin1()) {
      case 'n':
         replacement = '\n';
         break;
      case 't':
         replacement = '\t';
         break;
      case 'r':
         replacement = '\r';
         break;
      default:
         replacement = text[start + 1];
         break;
      }
      rc.replace(start, 2, replacement);
      start++;
   }
   return rc;
}

void MainWindow::closing() {
   saveState();
}

/**
 * Clears the table.
 */
void MainWindow::clear() {
   ui->tableWidget->setRowCount(0);
}

/**
 * Handles the click of the "export" button.
 */
void MainWindow::exportFiles() {
   comboText(ui->comboBoxHeader);
   comboText(ui->comboBoxTemplate);
   comboText(ui->comboBoxExportFile);
   comboText(ui->comboBoxFooter);
   if (ui->radioButtonFile->isChecked()) {
      QString fn = ui->comboBoxExportFile->currentText();
      FILE* fp = fopen( I18N::s2b(fn), "w");
      if (fp == NULL)
         guiError(ui->comboBoxExportFile, tr("not a valid file: ") + fn);
      else {
         QTextStream stream(fp);
         exportToStream(stream);
         fclose(fp);
         say(LOG_INFO, tr("result exported to ") + fn);
      }
   } else {
      QString value;
      QTextStream stream(&value);
      m_errors = 0;
      exportToStream(stream);
      QClipboard* clipboard = QApplication::clipboard();
      clipboard->setText(value);
      if (m_errors == 0)
         say(LOG_INFO, tr("result exported to the clipboard"));
   }
}

/**
 * Exports the found files into a stream with header and footer.
 *
 * @param stream    OUT: the stream for the export
 * @param maxRow    -1 or the maximum row to export
 */
void MainWindow::exportToStream(QTextStream& stream, int maxRow) {
   QMap < QString, QString > placeholders;
   buildGlobalPlaceholders (placeholders);
   if (!ui->comboBoxHeader->currentText().isEmpty()) {
      stream << replaceGlobalPlaceholders(ui->comboBoxHeader, placeholders)
             << endl;
   }
   int count = ui->tableWidget->rowCount();
   if (count > 0 && maxRow > 0)
      count = maxRow;
   QString error;
   for (int ii = 0; ii < count; ii++) {
      QString line = ui->comboBoxTemplate->currentText();
      QMap < QString, QString > placeholders;
      QString path = m_lastBaseDir.absoluteFilePath(
                        ReFileUtils::pathAppend(ui->tableWidget->item(ii, TC_PATH)->text(),
                              ui->tableWidget->item(ii, TC_NODE)->text()));
      placeholders.insert("full", addEsc(ReQStringUtils::nativePath(path)));
      path = ReFileUtils::nativePath(ui->tableWidget->item(ii, TC_PATH)->text());
      placeholders.insert("path", addEsc(path));
      placeholders.insert("ext", ui->tableWidget->item(ii, TC_EXT)->text());
      placeholders.insert("node", ui->tableWidget->item(ii, TC_NODE)->text());
      placeholders.insert("modified",
                          ui->tableWidget->item(ii, TC_MODIFIED)->text());
      placeholders.insert("size", ui->tableWidget->item(ii, TC_SIZE)->text());
      if (!ReQStringUtils::replacePlaceholders(line, placeholders, &error)) {
         guiError(ui->comboBoxTemplate, error);
         break;
      }
      stream << replaceEscSequences(line) << endl;
   }
   if (!ui->comboBoxFooter->currentText().isEmpty()) {
      stream << replaceGlobalPlaceholders(ui->comboBoxFooter, placeholders)
             << endl;
   }
}

/**
 * Processes the dragging operation of the selected files in the table widget.
 */
void MainWindow::fileDragging() {
   QDrag* drag = new QDrag(this);
   QMimeData* mimeData = new QMimeData;
   QList < QUrl > urls;
   QList < QTableWidgetSelectionRange > ranges =
      ui->tableWidget->selectedRanges();
   QList <QTableWidgetSelectionRange>::iterator it;
   int files = 0;
   int dirs = 0;
   bool isDir = false;
   for (it = ranges.begin(); it != ranges.end(); ++it) {
      for (int row = (*it).topRow(); row <= (*it).bottomRow(); row++) {
         isDir = cellAsText(row, TC_SIZE).isEmpty();
         QUrl url(buildAbsPath(row, true, true));
         urls.append(url);
         if (isDir)
            dirs++;
         else
            files++;
      }
   }
   if (urls.size() > 0) {
      mimeData->setUrls(urls);
      drag->setMimeData(mimeData);
      QPixmap image(200, 30);
      QPainter painter(&image);
      QString msg;
      if (urls.size() == 1)
         msg = tr("copy ") + ReFileUtils::nodeOf(urls.at(0).toString());
      else if (files > 0 && dirs > 0)
         msg = tr("copy %1 file(s) and %2 dir(s)").arg(files).arg(dirs);
      else if (files > 0)
         msg = tr("copy %1 file(s)").arg(files);
      else
         msg = tr("copy %1 dirs(s)").arg(dirs);
      painter.fillRect(image.rect(), Qt::white);
      painter.drawText(10, 20, msg);
      drag->setPixmap(image);
      Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
   }
}

/**
 * Handles the push of "select file placeholder".
 */
void MainWindow::filePlaceholder() {
   DialogFilePlaceholder dialog;
   dialog.exec();
   if (!dialog.var().isEmpty()) {
      QComboBox* target = ui->comboBoxTemplate;
      target->setCurrentText(target->currentText() + dialog.var());
   }
}

/**
 * Handles the push of "select placeholder for the footer".
 */
void MainWindow::footerPlaceholder() {
   handlePlaceholder(ui->comboBoxFooter);
}

/**
 * Puts the absolute full name of the current (selected) file into the clipboard.
 */
void MainWindow::fullNameToClipboard() {
   int row = ui->tableWidget->currentRow();
   if (row >= 0) {
      QClipboard* clipboard = QApplication::clipboard();
      QString path = buildAbsPath(row);
      ReQStringUtils::ensureLastChar(path, OS_SEPARATOR);
      path += cellAsText(row, TC_NODE);
      clipboard->setText(path);
   }
}

/**
 * Callback method of the GUI timer.
 */
void MainWindow::guiTimerUpdate() {
   int count = m_guiQueue.count();
   while(count-- > 0) {
      ReGuiQueueItem item = m_guiQueue.popFront();
      if (item.m_type == ReGuiQueueItem::Undef)
         break;
      if (! item.apply()) {
         switch (item.m_type) {
         case ReGuiQueueItem::ReadyMessage:
            say(LOG_INFO, item.m_value);
            m_statistics = m_finder->statistics();
            startStop(false);
            break;
         case ReGuiQueueItem::LogMessage:
            say(LOG_INFO, item.m_value);
            break;
         default:
            say(LOG_ERROR, "unknown item type: " + QString::number(item.m_type)
                + " " + item.m_value);
            break;
         }
      }
   }
}

/**
 * Handle the "copy to clipboard" entry from the context menu.
 *
 * @param currentRow    the row where the context menu is called
 * @param full          the full name of the current row
 */
void MainWindow::handleCopyToClipboard(int currentRow, const QString& full) {
   QMimeData* mimeData = new QMimeData;
   QList < QUrl > urls;
   bool isInSelection = false;
   QList < QTableWidgetSelectionRange > ranges =
      ui->tableWidget->selectedRanges();
   QList <QTableWidgetSelectionRange>::iterator it;
   QString textList;
   textList.reserve(ui->tableWidget->rowCount() * 80);
   for (it = ranges.begin(); it != ranges.end(); ++it) {
      for (int row = (*it).topRow(); row <= (*it).bottomRow(); row++) {
         isInSelection = isInSelection || row == currentRow;
         QString name(buildAbsPath(row, true));
         QUrl url(name);
         textList += name + '\n';
         urls.append(url);
      }
   }
   if (!isInSelection) {
      urls.clear();
      urls.append(QUrl(full));
      textList = full;
   }
   mimeData->setUrls(urls);
   mimeData->setText(textList);
   QClipboard* clipboard = QApplication::clipboard();
   clipboard->setMimeData(mimeData);
   say(LOG_INFO,
       tr("%1 entry/entries copied to clipboard").arg(urls.length()));
}

/**
 * Starts an external command given by the context menu for the selected file.
 *
 * @param handler   the info about the external program_invocation_short_name
 * @param parent    the path of the file
 * @param full      the full file name
 * @param node      the file name without path
 */
void MainWindow::handleExternalCommand(ContextHandler* handler,
                                       const QString& parent, const QString& full, const QString& node) {
   QString arguments = handler->m_arguments;
   QString dir;
   switch (handler->m_directoryMode) {
   case ContextHandler::DM_TO_PARENT:
      dir = parent;
      break;
   case ContextHandler::DM_TO_FILE:
      dir = full;
      break;
   default:
      dir = ui->comboBoxDirectory->currentText();
      break;
   }
   QMap < QString, QString > placeholders;
   placeholders.insert("full", full);
   placeholders.insert("node", node);
   placeholders.insert("path", parent);
   placeholders.insert("ext", ReFileUtils::extensionOf(node));
   QString error;
   QStringList args = arguments.split(' ');
   bool hasErrors = false;
   for (int ix = 0; ix < args.size(); ix++) {
      QString arg = args.at(ix);
      if (!ReQStringUtils::replacePlaceholders(arg, placeholders, &error)) {
         guiError(NULL, error);
         hasErrors = true;
         break;
      }
      args.replace(ix, arg);
   }
   if (!hasErrors) {
      QProcess::startDetached(handler->m_program, args, dir, NULL);
      say(LOG_INFO,
          tr("started:") + " " + handler->m_program + " " + arguments);
   }
}

/**
 * Handles the global placeholder selection dialog.
 *
 * @param target    OUT: the combobox where the result is appended
 */
void MainWindow::handlePlaceholder(QComboBox* target) {
   DialogGlobalPlaceholder dialog;
   dialog.exec();
   if (!dialog.var().isEmpty())
      target->setCurrentText(target->currentText() + dialog.var());
}

/**
 * Count the selected rows of a tableWidget and tests whether a given row is in the selection.
 *
 * @param table         the table to test
 * @param currentRow    -1 or the row which will be tested whether it is a member of the selection
 * @param isInSelection OUT: true: currentRow is a member of the selected rows
 *
 * @return  the number of selected rows
 */
static int countSelectedRows(QTableWidget* table, int currentRow,
                             bool& isInSelection) {
   int rc = 0;
   isInSelection = false;
   QList < QTableWidgetSelectionRange > ranges = table->selectedRanges();
   QList <QTableWidgetSelectionRange>::iterator it;
   for (it = ranges.begin(); it != ranges.end(); ++it) {
      for (int row = (*it).topRow(); row <= (*it).bottomRow(); row++) {
         isInSelection = isInSelection || row == currentRow;
         rc++;
      }
   }
   return rc;
}

/**
 * Handles the request of a context menu of the result table.
 *
 * @param position  the position of a mouse click
 */
void MainWindow::handleTableContextMenu(const QPoint& position) {
   int currentRow = ui->tableWidget->rowAt(position.y());
   if (currentRow >= 0) {
      QMenu menu;
      QString node = ui->tableWidget->item(currentRow, TC_NODE)->text();
      QString parent = buildAbsPath(currentRow);
      QString full = ReFileUtils::pathAppend(parent, node);
      QFileInfo file(full);
      bool isDir = file.isDir();
      QList <ContextHandler*>::const_iterator it;
      QMap <QAction*, ContextHandler*> actions;
      ContextHandler* handler;
      bool hasSeparator = false;
      for (it = m_contextHandlers.list().begin();
            it != m_contextHandlers.list().end(); ++it) {
         handler = *it;
         if ((isDir && handler->m_fileType == ContextHandler::FT_FILE)
               || (!isDir && handler->m_fileType == ContextHandler::FT_DIR))
            continue;
         QString text = handler->m_text + " " + node;
         if (handler->intrinsicType() != ContextHandler::IT_UNDEF) {
            if (!hasSeparator) {
               hasSeparator = true;
               menu.addSeparator();
            }
            bool inSelection = false;
            int count = countSelectedRows(ui->tableWidget, currentRow,
                                          inSelection);
            if (inSelection)
               text = handler->m_text + tr(" %1 object(s)").arg(count);
         }
         actions.insert(menu.addAction(text), handler);
      }
      QAction* selectedItem = menu.exec(
                                 ui->tableWidget->viewport()->mapToGlobal(position));
      if (selectedItem != NULL) {
         handler = *actions.find(selectedItem);
         switch (handler->intrinsicType()) {
         case ContextHandler::IT_COPY:
            handleCopyToClipboard(currentRow, full);
            break;
         case ContextHandler::IT_UNDEF:
         default:
            handleExternalCommand(handler, parent, full, node);
            break;
         }
      }
   }
}
/**
 * Handles the event "header column clicked".
 *
 * @param col   the col
 */
void MainWindow::headerClicked(int col) {
   m_lastOrder =
      m_lastOrder == Qt::AscendingOrder ?
      Qt::DescendingOrder : Qt::AscendingOrder;
   ui->tableWidget->sortItems(col, m_lastOrder);
   m_horizontalHeader->setSortIndicatorShown(true);
   m_horizontalHeader->setSortIndicator(col, m_lastOrder);
}

/**
 * Handles the push of "select placeholder for the header".
 */
void MainWindow::headerPlaceholder() {
   handlePlaceholder(ui->comboBoxHeader);
}

/**
 * Informs the instance about some state changes.
 *
 * @param message	description of the state change
 * @return			SUCCESS
 */
ReObserver::ReturnCode MainWindow::notify(const char* message) {
   return ReObserver::SUCCESS;
}

/**
 * Prepares the context menu of the result table.
 *
 */
void MainWindow::prepareContextMenu() {
   ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(ui->tableWidget,
           SIGNAL(customContextMenuRequested(const QPoint&)),
           SLOT(handleTableContextMenu(const QPoint&)));
}

/**
 * Prepares the text search.
 */
void MainWindow::prepareTextFind() {
   m_textFinder.setSearchParameter(ui->comboBoxTextPattern->currentText(),
                                   ui->checkBoxTextIgnoreCase->isChecked(), ui->checkBoxRegExpr->isChecked(),
                                   !ui->checkBoxBinaryFiles->isChecked());
   QString error = m_textFinder.regExprError();
   if (!error.isEmpty()) {
      guiError(ui->comboBoxTextPattern, error);
   }
}

/**
 * Shows a preview of the result export.
 */
void MainWindow::preview() {
   QString value;
   QTextStream stream(&value);
   exportToStream(stream, 1);
   QMessageBox msgBox;
   msgBox.setText(value);
   msgBox.setInformativeText(tr("Preview of the Export"));
   msgBox.setStandardButtons(QMessageBox::Cancel);
   msgBox.setDefaultButton(QMessageBox::Save);
   msgBox.exec();
}

/**
 * Builds the hash with the global placeholders with their current values.
 *
 * @param hash  IN/OUT: the placeholders will be appended here
 */
void MainWindow::buildGlobalPlaceholders(QMap <QString, QString>& hash) {
   hash.insert("filepatterns", ui->comboBoxFilePatterns->currentText());
   hash.insert("base", m_lastBaseDir.absolutePath());
   hash.insert("textpattern", addEsc(ui->comboBoxTextPattern->currentText()));
   hash.insert("dirs", QString::number(m_statistics.m_dirs));
   hash.insert("files", QString::number(m_statistics.m_files));
   hash.insert("runtime", QString::number(m_statistics.m_runtimeSeconds, 'g', 3));
   hash.insert("bytes", QString::number(m_statistics.m_bytes));
   hash.insert("megabytes",
               QString::number((double) m_statistics.m_bytes / 1000000));
   hash.insert("datetime",
               QDateTime::currentDateTime().toLocalTime().toString("yyyy.MM.dd/hh:mm:ss"));
}
/**
 * Replaces the placeholders valid in header and footer.
 *
 * @param text  the text to convert
 * @return      <code>text</code> with the esc sequences replaced
 */
QString MainWindow::replaceGlobalPlaceholders(QComboBox* combo,
      QMap <QString, QString>& placeholders) {
   QString rc = combo->currentText();
   QString error;
   if (!ReQStringUtils::replacePlaceholders(rc, placeholders, &error))
      guiError(combo, error);
   return replaceEscSequences(rc);
}

/**
 * @brief Handles the action "reset parameters".
 *
 * Most of the filter parameters will be set to the default.
 */
void MainWindow::resetParameters() {
   ui->comboBoxFilePatterns->setCurrentText("");
   ui->comboBoxMaxDepth->setCurrentText("");
   ui->comboBoxMaxSize->setCurrentText("");
   ui->comboBoxMinDepth->setCurrentText("");
   ui->comboBoxMinSize->setCurrentText("");
   ui->comboBoxOlder->setCurrentText("");
   ui->comboBoxYounger->setCurrentText("");
   ui->comboBoxTextPattern->setCurrentText("");
   ui->checkBoxBinaryFiles->setChecked(false);
   ui->checkBoxDirs->setChecked(true);
   ui->checkBoxFiles->setChecked(true);
   ui->checkBoxLinks->setChecked(true);
   ui->checkBoxRegExpr->setChecked(false);
   ui->checkBoxTextIgnoreCase->setChecked(false);
}

/**
 * Reads the history of the widget values and other parameters and set it.
 */
void MainWindow::restoreState() {
   ReStateStorage storage(m_storageFile, m_logger);
   storage.setForm("main");
   storage.restore(ui->comboBoxDirectory, "comboBoxDirectory", true);
   storage.restore(ui->comboBoxExcludedDirs, "comboBoxExcludedDirs", true);
   storage.restore(ui->comboBoxExportFile, "comboBoxExportFile", true);
   storage.restore(ui->comboBoxFilePatterns, "comboBoxFilePatterns");
   storage.restore(ui->comboBoxFooter, "comboBoxFooter", true);
   storage.restore(ui->comboBoxHeader, "comboBoxHeader", true);
   storage.restore(ui->comboBoxMaxDepth, "comboBoxMaxDepth");
   storage.restore(ui->comboBoxMaxSize, "comboBoxMaxSize");
   storage.restore(ui->comboBoxMinDepth, "comboBoxMinDepth");
   storage.restore(ui->comboBoxMinSize, "comboBoxMinSize");
   storage.restore(ui->comboBoxOlder, "comboBoxOlder");
   storage.restore(ui->comboBoxTemplate, "comboBoxTemplate", true);
   storage.restore(ui->comboBoxTextPattern, "comboBoxTextPattern");
   storage.restore(ui->comboBoxYounger, "comboBoxYounger");
   m_contextHandlers.restore(storage);
   storage.close();
}

/**
 * Stores the history of the widget values and other parameters.
 */
void MainWindow::saveState() {
   ReStateStorage storage(m_storageFile, m_logger);
   storage.setForm("main");
   storage.store(ui->comboBoxDirectory, "comboBoxDirectory");
   storage.store(ui->comboBoxExcludedDirs, "comboBoxExcludedDirs");
   storage.store(ui->comboBoxExportFile, "comboBoxExportFile");
   storage.store(ui->comboBoxFilePatterns, "comboBoxFilePatterns");
   storage.store(ui->comboBoxFooter, "comboBoxFooter");
   storage.store(ui->comboBoxHeader, "comboBoxHeader");
   storage.store(ui->comboBoxMaxDepth, "comboBoxMaxDepth");
   storage.store(ui->comboBoxMaxSize, "comboBoxMaxSize");
   storage.store(ui->comboBoxMinDepth, "comboBoxMinDepth");
   storage.store(ui->comboBoxMinSize, "comboBoxMinSize");
   storage.store(ui->comboBoxOlder, "comboBoxOlder");
   storage.store(ui->comboBoxTemplate, "comboBoxTemplate");
   storage.store(ui->comboBoxTextPattern, "comboBoxTextPattern");
   storage.store(ui->comboBoxYounger, "comboBoxYounger");
   m_contextHandlers.save(storage);
   storage.close();
}

/**
 * Sets the properties of the file finder.
 *
 * @param finder	OUT: the finder to populate
 */
void MainWindow::populateFinder(FileFinder& finder) {
   if (! ui->checkBoxAppend->isChecked()) {
      ui->tableWidget->setRowCount(0);
      m_statistics.clear();
   }
   finder.setObserver(this);
   finder.setGuiQueue(&this->m_guiQueue);
   finder.setBaseDir(comboText(ui->comboBoxDirectory));
   finder.setTable(ui->tableWidget);
   m_lastBaseDir.cd(comboText(ui->comboBoxDirectory));
   finder.setMaxSize(comboSize(ui->comboBoxMaxSize));
   finder.setMinSize(comboSize(ui->comboBoxMinSize));
   finder.setOlderThan(comboDate(ui->comboBoxOlder));
   finder.setYoungerThan(comboDate(ui->comboBoxYounger));
   finder.setMinDepth(comboInt(ui->comboBoxMinDepth, 0));
   finder.setMaxDepth(comboInt(ui->comboBoxMaxDepth, -1));
   finder.setMaxHits(comboInt(ui->comboBoxMaxHits, 0x7fffffff));
   finder.setFiletypes(buildFileTypes());
   QStringList patterns;
   QString value = comboText(ui->comboBoxFilePatterns);
   if (!value.isEmpty())
      patterns = value.split(",");
   finder.setPatterns(patterns);
   value = comboText(ui->comboBoxExcludedDirs);
   if (value.indexOf('/') >= 0 || value.indexOf('\\') >= 0)
      guiError(ui->comboBoxExcludedDirs, tr("no path delimiter allowed"));
   else if (value.indexOf('*') >= 0)
      guiError(ui->comboBoxExcludedDirs,
               tr("no patterns allowed. Do not use '*"));
   else if (!value.isEmpty())
      patterns = value.split(",");
   finder.setExcludedDirs(patterns);
   if (m_errors == 0) {
      prepareTextFind();
      if (!comboText(ui->comboBoxTextPattern).isEmpty())
         finder.setTextFinder(&m_textFinder);
   }
}

/**
 * Handles the "search" button.
 */
void MainWindow::search() {
   m_errors = 0;
   QString path = comboText(ui->comboBoxDirectory);
   QFileInfo dir(path);
   if (!dir.exists())
      guiError(ui->comboBoxDirectory, tr("directory not found: ") + path);
   else if (!dir.isDir())
      guiError(ui->comboBoxDirectory, tr("not a directory: ") + path);
   else {
      QApplication::setOverrideCursor (QCursor(Qt::WaitCursor));
      ui->pushButtonSearch->setEnabled(false);
      ui->pushButtonSearch2->setEnabled(false);
      if (m_finder == NULL)
         m_finder = new FileFinder;
      populateFinder(*m_finder);
      if (! ui->checkBoxAppend->isChecked())
         clear();
      startStop(true);
      m_finder->start();
      QApplication::restoreOverrideCursor();
   }
}
/**
 * Handles the push of the button "select directory".
 */
void MainWindow::selectDirectory() {
   QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                 ui->comboBoxDirectory->currentText(), QFileDialog::ShowDirsOnly);
   if (!dir.isEmpty())
      ui->comboBoxDirectory->setCurrentText(ReFileUtils::nativePath(dir));
}

/**
 * Calls the file selection dialog.
 */
void MainWindow::selectExportFile() {
   QString name = QFileDialog::getSaveFileName(this, tr("Select Export File"),
                  ui->comboBoxExportFile->currentText());
   if (!name.isEmpty())
      ui->comboBoxExportFile->setCurrentText(name);
}

/**
 * Issues a message in the status line.
 *
 * @param level		type of the message, e.g. LOG_INFO or LOG_ERROR
 * @param message	the message to issue. Use QString::arg() to format,
 *					e.g. <code>say(QString("name: %1 no: %2).arg(name).arg(no)</code>
 * @return			<code>false</code>: level == LOG_ERROR or LOG_WARNING<br>
 *					<code>true</code>: level >= LOG_INFO
 */
bool MainWindow::say(ReLoggerLevel level, const QString& message) {
   bool rc = level >= LOG_INFO;
   if (m_stdLabelBackgroundRole == NULL)
      m_stdLabelBackgroundRole = new QPalette::ColorRole(
         m_statusMessage->backgroundRole());
   m_statusMessage->setBackgroundRole(
      ! rc ? QPalette::HighlightedText : *m_stdLabelBackgroundRole);
   m_statusMessage->setText(message);
   return rc;
}
/**
 * Starts or stops the search.
 *
 * @param start	<code>true</code>: the search should start
 */
void MainWindow::startStop(bool start) {
   ui->actionStart->setEnabled(!start);
   ui->actionStop->setEnabled(start);
   ui->pushButtonSearch->setEnabled(! start);
   ui->pushButtonSearch2->setEnabled(! start);
   ui->pushButtonStop->setEnabled(start);
   ui->pushButtonStop2->setEnabled(start);
}

/**
 * Handles the "search" button.
 */
void MainWindow::stop() {
   m_finder->setStop(true);
   startStop(false);
}

/**
 * @brief Handles the "up" button: go to the parent directory.
 */
void MainWindow::up() {
   QString path = ui->comboBoxDirectory->currentText();
   QDir dir(path);
   if (dir.exists()) {
      dir.cdUp();
      if (dir.exists()) {
         path = ReFileUtils::nativePath(dir.absolutePath());
         ui->comboBoxDirectory->setEditText(path);
         setInHistory(ui->comboBoxDirectory, path);
      }
   }
}

