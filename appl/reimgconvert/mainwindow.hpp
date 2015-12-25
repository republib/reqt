/*
 * mainwindow.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileDialog>
#include <QResizeEvent>
#include <QLabel>
#include "converter.hpp"
#include "gui/regui.hpp"
namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow,
   public ReGuiValidator,
   public ConvertLogger {
   Q_OBJECT

public:
   explicit MainWindow(const QString& homeDir, QWidget* parent = 0);
   ~MainWindow();

public:
   bool error(const QString& message) {
      log("+++ " + message);
      return false;
   }
   bool log(const QString& message);
   bool logAppendLast(const QString& message);
   void setStatusMessage(bool error, const QString& message);
   void switchRun(bool runActive);
private:
   void initializeHome();
   void restoreState();
   void saveState();
   void setMaxDimensions(int maxWidth, int maxHeight);
public slots:
   void on_threadStateChanged(Converter::State state, const QString& info);
private slots:
   void activate();
   void about();
   void on_pushButtonFileSelect_clicked();
   void on_pushButtonStop_clicked();
   void on_pushButtonConvert_clicked();
   void on_templateChangeIndex(const QString& text);
   void selectDestination();
   void selectSource();
private:
   QString m_homeDir;
   QString m_storageFile;
   Ui::MainWindow* ui;
   Converter* m_converter;
   QLabel* m_statusMessage;
};

#endif // MAINWINDOW_HPP
