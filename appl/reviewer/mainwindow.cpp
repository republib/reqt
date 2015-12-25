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
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "aboutdialog.hpp"
#include <QDir>
#include <QFileDialog>
#include <QClipboard>
#include <QLineEdit>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QProcess>
#include <QMessageBox>

/**
 * @brief Constructor.
 *
 * @param file		the file to view
 * @param parent    NULL or the parent widget
 */
MainWindow::MainWindow(const QString& file,
                       QWidget* parent) :
   QMainWindow(parent),
   ui(new Ui::MainWindow),
   m_statusMessage(NULL),
   m_stdLabelBackgroundRole(NULL),
   m_homeDir(),
   m_file(file),
   m_storageFile() {
   ui->setupUi(this);
   initializeHome();
   m_statusMessage = new QLabel(tr("Welcome at reviewer"));
   statusBar()->addWidget(m_statusMessage);
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
      m_homeDir = QDir::home().absoluteFilePath(".review");
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
   AboutDialog dialog;
   dialog.exec();
}

void MainWindow::closing() {
   saveState();
}




/**
 * Handles the push of the button "select directory".
 */
void MainWindow::restoreState() {
   ReStateStorage storage(m_storageFile);
   storage.setForm("main");
   //storage.restore(ui->comboBoxDirectory, "comboBoxDirectory", true);
   storage.close();
}

/**
 * Handles the push of the button "select directory".
 */
void MainWindow::saveState() {
   ReStateStorage storage(m_storageFile);
   storage.setForm("main");
   //storage.store(ui->comboBoxDirectory, "comboBoxDirectory");
   storage.close();
}


/**
 * Writes a text to the status line.
 *
 * @param error     <code>true</code>: the message is an error message
 * @param message   the text to set
 */
void MainWindow::setStatusMessage(bool error, const QString& message) {
   if (m_stdLabelBackgroundRole == NULL)
      m_stdLabelBackgroundRole = new QPalette::ColorRole(
         m_statusMessage->backgroundRole());
   m_statusMessage->setBackgroundRole(
      error ? QPalette::HighlightedText : *m_stdLabelBackgroundRole);
   m_statusMessage->setText(message);
}

