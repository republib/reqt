/*
 * idosmain.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reidos.hpp"
#include "ui_idosmain.h"

IDosMain::IDosMain(const QString& startDir, const QString& homeDir,
                   QWidget* parent) :
   QMainWindow(parent),
   ReGuiValidator(),
   FileCommander(this),
   ui(new Ui::IDosMain),
   m_statusMessage(NULL),
   m_homeDir(homeDir)
   //m_storageFile(),
{
   ui->setupUi(this);
   initializeHome();
   m_statusMessage = new QLabel(tr("Welcome at reidos"));
   ui->statusBar->addWidget(m_statusMessage);
   if (!startDir.isEmpty())
      ui->fileTableTop->comboBoxPath->setCurrentText(startDir);
   ui->fileTableTop->fileSystem = new ReLocalFileSystem("/", m_logger);
   ui->fileTableTop->fillTable();
   ui->fileTableTop->announcer = this;
   ui->fileTableBottom->fileSystem = new ReLocalFileSystem("/", m_logger);
   ui->fileTableBottom->fillTable();
   ui->fileTableTop->announcer = this;
   QString dir(ui->fileTableTop->comboBoxPath->currentText());
   if (dir.isEmpty())
      dir = startDir;
   if (dir.isEmpty())
      dir = QDir::homePath();
   changeDirectory(dir);
}

/**
 * Destructor.
 */
IDosMain::~IDosMain() {
   delete ui;
}

/**
 * initializeHomeializes the program home directory.
 */
void IDosMain::initializeHome() {
   if (m_homeDir.isEmpty()) {
      m_homeDir = QDir::home().absoluteFilePath(".reidos");
   }
   QDir home(m_homeDir);
   if (!home.exists()) {
      if (!home.mkpath(m_homeDir)) {
         m_homeDir = home.tempPath() + "/.redos";
         home.mkpath(m_homeDir);
      }
   }
   if (!m_homeDir.endsWith("/"))
      m_homeDir += "/";
   m_storageFile = m_homeDir + "state.conf";
   restoreState();
}


/**
 * Writes a text to the status line.
 *
 * @param level     the type of the message, e.g. LOG_ERROR
 * @param message   the text to set
 * @return			<code>false</code>: level == LOG_ERROR or LOG_WARNING<br>
 *					<code>true</code>: level == LOG_INFO
 */
bool IDosMain::say(ReLoggerLevel level, const QString& message) {
   bool rc = level == LOG_INFO;
   m_statusMessage->setText(! rc ? "+++ " + message : message);
   return rc;
}

/**
 * Reads the history of the widget values and other parameters and set it.
 */
void IDosMain::restoreState() {
   ReStateStorage storage(m_storageFile, m_logger);
   storage.setForm("main");
   storage.restore(ui->fileTableTop->comboBoxPath, "comboBoxPathTop", true);
   storage.restore(ui->fileTableTop->comboBoxPatterns, "comboBoxPatternTop", false);
   storage.restore(ui->fileTableBottom->comboBoxPath, "comboBoxPathBottom", true);
   storage.restore(ui->fileTableBottom->comboBoxPatterns, "comboBoxPatternBottom", false);
   storage.close();
}

/**
 * Stores the history of the widget values and other parameters.
 */
void IDosMain::saveState() {
   ReStateStorage storage(m_storageFile, m_logger);
   storage.setForm("main");
   storage.store(ui->fileTableTop->comboBoxPath, "comboBoxPathTop");
   storage.store(ui->fileTableTop->comboBoxPatterns, "comboBoxPatternTop");
   storage.store(ui->fileTableBottom->comboBoxPath, "comboBoxPathBottom");
   storage.store(ui->fileTableBottom->comboBoxPatterns, "comboBoxPatternBottom");
   storage.close();
}
