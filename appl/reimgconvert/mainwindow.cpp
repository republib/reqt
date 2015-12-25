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

const QString VERSION("2015.05.31");

/** @class MainWindow mainwindow.hpp "mainwindow.hpp"
 *
 * @brief Implements the main window of the application.
 *
 * Administrates a form with all input data and a listbox for logging output.
 *
 * The button "convert" stars the conversion with the given data.
 */

/**
 * @brief Constructor.
 *
 * @param parent        NULL or the parent (who destroys the objects at the end)
 */
MainWindow::MainWindow(const QString& homeDir, QWidget* parent) :
   QMainWindow(parent),
   m_homeDir(homeDir),
   m_storageFile(),
   ui(new Ui::MainWindow),
   m_converter(NULL) {
   ui->setupUi(this);
   initializeHome();
   switchRun(true);
   m_statusMessage = new QLabel(tr("Welcome at reimgconvert"));
   statusBar()->addWidget(m_statusMessage);
   connect(ui->actionSelectDestination, SIGNAL(triggered()), this,
           SLOT(selectDestination()));
   connect(ui->pushButtonSelectDest, SIGNAL(clicked()), this,
           SLOT(selectDestination()));
   connect(ui->actionSelectSource, SIGNAL(triggered()), this,
           SLOT(selectSource()));
   connect(ui->pushButtonSelectDest, SIGNAL(clicked()), this,
           SLOT(selectDestination()));
   connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
   restoreState();
   connect(ui->comboBoxTemplate, SIGNAL(currentIndexChanged(const QString&)),
           this, SLOT(on_templateChangeIndex(const QString&)));
   connect(ui->pushButtonActivate, SIGNAL(clicked()), this, SLOT(activate()));
   restoreState();
}

/**
 * @brief Destructor
 */
MainWindow::~MainWindow() {
   delete ui;
   delete m_converter;
}

/**
 * Slot when the pushbutton "activate" is clicked.
 */
void MainWindow::activate() {
   int width = comboInt(ui->comboBoxMaxWidth, 0);
   int height = comboInt(ui->comboBoxMaxHeight, 0);
   setMaxDimensions(width, height);
}

/**
 * Sets the maximal dimensions (from the "simple" interface).
 *
 * @param maxWidth  the maximal width
 * @param maxHeight the maximal height
 */
void MainWindow::setMaxDimensions(int maxWidth, int maxHeight) {
   if (maxWidth > maxHeight) {
      ui->comboBoxLandscapeX->setCurrentText(QString::number(maxWidth));
      ui->comboBoxLandscapeY->setCurrentText("*");
      ui->comboBoxPortraitX->setCurrentText("*");
      ui->comboBoxPortraitY->setCurrentText(QString::number(maxHeight));
      ui->comboBoxSquareX->setCurrentText(QString::number(maxHeight));
   } else {
      ui->comboBoxLandscapeY->setCurrentText(QString::number(maxHeight));
      ui->comboBoxLandscapeX->setCurrentText("*");
      ui->comboBoxPortraitY->setCurrentText("*");
      ui->comboBoxPortraitX->setCurrentText(QString::number(maxWidth));
      ui->comboBoxSquareX->setCurrentText(QString::number(maxWidth));
   }
}

/**
 * Selects the destination directory with a dialog.
 */
void MainWindow::selectDestination() {
   QString dir = QFileDialog::getExistingDirectory(this,
                 tr("Select Destination Directory"), ui->comboBoxTarget->currentText(),
                 QFileDialog::ShowDirsOnly);
   if (!dir.isEmpty())
      ui->comboBoxTarget->setCurrentText(dir);
}

/**
 * Selects the destination directory with a dialog.
 */
void MainWindow::selectSource() {
   QString dir = QFileDialog::getExistingDirectory(this,
                 tr("Select Source Directory"), ui->comboBoxSourceDir->currentText(),
                 QFileDialog::ShowDirsOnly);
   if (!dir.isEmpty())
      ui->comboBoxSourceDir->setCurrentText(dir);
}

/**
 * Starts the about dialog.
 */
void MainWindow::about() {
   AboutDialog dialog(VERSION);
   dialog.exec();
}

/**
 * initializeHomeializes the program home directory.
 */
void MainWindow::initializeHome() {
   if (m_homeDir.isEmpty()) {
      m_homeDir = QDir::home().absoluteFilePath(".reimgconvert");
   }
   QDir home(m_homeDir);
   if (!home.exists()) {
      if (!home.mkpath(m_homeDir)) {
         m_homeDir = home.tempPath() + "/.reimgconvert";
         home.mkpath(m_homeDir);
      }
   }
   if (!m_homeDir.endsWith("/"))
      m_homeDir += "/";
   m_storageFile = m_homeDir + "state.conf";
   restoreState();
}

/**
 * @brief Handles the click on the button "stop".
 */
void MainWindow::on_pushButtonStop_clicked() {
   m_converter->stop();
   ui->pushButtonConvert->show();
   ui->pushButtonStop->hide();
}

/**
 * @brief Handles the click on the button "file select".
 *
 * Shows a selection dialog for directories and sets the source directory
 * onto the selected directory.
 */
void MainWindow::on_pushButtonFileSelect_clicked() {
   QFileDialog selection;
   selection.setFileMode(QFileDialog::DirectoryOnly);
   QString dir = ui->comboBoxSourceDir->currentText();
   if (!dir.isEmpty())
      selection.setDirectory(dir);
   if (selection.exec())
      ui->comboBoxSourceDir->setCurrentText(selection.selectedFiles().at(0));
}

/**
 * Slot when the value of the template combobox has been changed.
 *
 * @param text  the new selected combobox text
 */
void MainWindow::on_templateChangeIndex(const QString& text) {
   QRegularExpression rexpr("(\\d+)x(\\d+)");
   QRegularExpressionMatch match = rexpr.match(text);
   int width = match.captured(1).toInt();
   int height = match.captured(2).toInt();
   setMaxDimensions(width, height);
}

/**
 * @brief Handles the button click on "convert".
 */
void MainWindow::on_pushButtonConvert_clicked() {
   switchRun(false);
   delete m_converter;
   m_errors = 0;
   int landscapeX = comboInt(ui->comboBoxLandscapeX, 0, "*", 0);
   int landscapeY = comboInt(ui->comboBoxLandscapeY, 0, "*", 0);
   int portraitX = comboInt(ui->comboBoxPortraitX, 0, "*", 0);
   int portraitY = comboInt(ui->comboBoxPortraitY, 0, "*", 0);
   int squareX = comboInt(ui->comboBoxSquareX, 0);
   int quality = comboInt(ui->comboBoxQuality, 70);
   if (m_errors == 0) {
      m_converter = new Converter(ui->comboBoxSourceDir->currentText(),
                                  ui->comboBoxLandscapeX->currentText(),
                                  ui->comboBoxSourcePattern->currentText(),
                                  ui->comboBoxDestType->currentText(), landscapeX, landscapeY, portraitX,
                                  portraitY, squareX, quality, this);
      // start the thread:
      m_converter->start();
   }
}

/**
 * @brief Handles the event "thread changed".
 *
 * @param state     the new state of the thread
 * @param info      info about the new state. Not used
 */
void MainWindow::on_threadStateChanged(Converter::State state, const QString&) {
   switch (state) {
   case Converter::STATE_READY:
      ui->pushButtonConvert->show();
      ui->pushButtonStop->hide();
      //ui->statusBar->showMessage(info);
      break;
   case Converter::STATE_SUB_TASK_STOPPED:
      //ui->statusBar->showMessage(info);
      break;
   case Converter::STATE_STARTING:
   default:
      break;
   }
}

/**
 * @brief Logs a message
 *
 * @param message   the message to log
 * @return          <code>true</code>
 */
bool MainWindow::log(const QString& message) {
   ui->listWidget->insertItem(0, message);
   return true;
}

/**
 * @brief Logs a message
 *
 * @param message   the message to log
 * @return          <code>true</code>
 */
bool MainWindow::logAppendLast(const QString& message) {
   QListWidgetItem* item = ui->listWidget->item(0);
   item->setText(item->text() + " " + message);
   return true;
}

/**
 * Reads the history of the widget values and other parameters and set it.
 */
void MainWindow::restoreState() {
   ReStateStorage storage(m_storageFile);
   storage.setForm("main");
   storage.restore(ui->comboBoxMaxHeight, "comboBoxMaxHeight", true);
   storage.restore(ui->comboBoxMaxWidth, "comboBoxMaxWidth", true);
   storage.restore(ui->comboBoxSourcePattern, "comboBoxSourcePattern", true);
   storage.restore(ui->comboBoxLandscapeX, "comboBoxLandscapeX", true);
   storage.restore(ui->comboBoxLandscapeY, "comboBoxLandscapeY");
   storage.restore(ui->comboBoxPortraitX, "comboBoxPortraitX", true);
   storage.restore(ui->comboBoxPortraitY, "comboBoxPortraitY", true);
   storage.restore(ui->comboBoxQuality, "comboBoxQuality", true);
   storage.restore(ui->comboBoxSourceDir, "comboBoxSourceDir", true);
   storage.restore(ui->comboBoxSquareX, "comboBoxSquareX", true);
   storage.restore(ui->comboBoxTarget, "comboBoxTarget", true);
   storage.close();
}

/**
 * Stores the history of the widget values and other parameters.
 */
void MainWindow::saveState() {
   ReStateStorage storage(m_storageFile);
   storage.setForm("main");
   storage.store(ui->comboBoxMaxHeight, "comboBoxMaxHeight");
   storage.store(ui->comboBoxMaxWidth, "comboBoxMaxWidth");
   storage.store(ui->comboBoxSourcePattern, "comboBoxSourcePattern");
   storage.store(ui->comboBoxLandscapeX, "comboBoxLandscapeX");
   storage.store(ui->comboBoxLandscapeY, "comboBoxLandscapeY");
   storage.store(ui->comboBoxPortraitX, "comboBoxPortraitX");
   storage.store(ui->comboBoxPortraitY, "comboBoxPortraitY");
   storage.store(ui->comboBoxQuality, "comboBoxQuality");
   storage.store(ui->comboBoxSourceDir, "comboBoxSourceDir");
   storage.store(ui->comboBoxSquareX, "comboBoxSquareX");
   storage.store(ui->comboBoxTarget, "comboBoxTarget");
   storage.close();
}

/**
 * Writes a text to the status line.
 *
 * @param error     <code>true</code>: the message is an error message
 * @param message   the text to set
 */
void MainWindow::setStatusMessage(bool error, const QString& message) {
   m_statusMessage->setText(message);
}

/**
 * Enables/disables the buttons/actions relevant for running.
 *
 * @param runActive <code>true</code>: the conversion is possible
 */
void MainWindow::switchRun(bool runActive) {
   if (runActive) {
      ui->pushButtonConvert->show();
      ui->pushButtonStop->hide();
   } else {
      ui->pushButtonConvert->hide();
      ui->pushButtonStop->show();
   }
   ui->actionConvert->setEnabled(runActive);
   ui->actionStop->setEnabled(not runActive);
}

