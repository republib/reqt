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
#ifndef REBASE_HPP
#include "reide.hpp"
#endif

class MainWindow: public QMainWindow {
   Q_OBJECT

public:
   explicit MainWindow(const char* workspace, const char* project,
                       ReLogger* logger, QWidget* parent = 0);
   ~MainWindow();
   void changeProject(QString path);
   void changeWorkspace(const QString& path);
   void openFile(const QString& name);
   Workspace* workspace() const;
   ReLogger* logger() const;
   PerspectiveList& perspectives();

public slots:
   void open();
   void geometryChanged(int value);
protected slots:
   void timeout();
private:
   Workspace* m_workspace;
   ReLogger* m_logger;
   PerspectiveList m_perspectives;
   bool m_timerRuns;
   QTimer m_timer;
   ReDelayedStorage* m_delayedStorage;
};

#endif // MAINWINDOW_HPP
