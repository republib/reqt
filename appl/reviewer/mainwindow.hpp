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
#ifndef REBASE_HPP
#include "base/rebase.hpp"
#endif
#include <QHeaderView>
#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QDir>
#if ! defined GUI_REGUI_HPP_
#include "gui/regui.hpp"
#endif
namespace Ui {
class MainWindow;
}
enum TableColumns {
   TC_NODE, TC_EXT, TC_SIZE, TC_MODIFIED, TC_TYPE, TC_PATH
};

class MainWindow: public QMainWindow, public ReGuiValidator {

   Q_OBJECT

public:
   explicit MainWindow(const QString& file, QWidget* parent = 0);
   ~MainWindow();

private slots:
   void about();
   void closing();
   void saveState();

private:
   void initializeHome();
   void restoreState();
   virtual void setStatusMessage(bool error, const QString& message);
private:
   Ui::MainWindow* ui;
   QLabel* m_statusMessage;
   QPalette::ColorRole* m_stdLabelBackgroundRole;
   QString m_homeDir;
   QString m_file;
   QString m_storageFile;
};

#endif // MAINWINDOW_HPP
