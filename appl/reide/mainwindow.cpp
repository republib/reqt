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

#include "reide.hpp"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(const char* workspace, const char* project,
                       ReLogger* logger, QWidget* parent) :
   QMainWindow(parent),
   m_workspace(NULL),
   m_logger(logger),
   m_perspectives(this),
   m_timerRuns(false),
   m_timer(this),
   m_delayedStorage(NULL) {
   setLayout(new QVBoxLayout);
   if (workspace == NULL)
      workspace = I18N::s2b(QDir::homePath());
   changeWorkspace(workspace == NULL ? QDir::homePath() : workspace);
   QString proj(project == NULL ? "" : project);
   if (project == NULL) {
      QStringList lastProjects;
      m_workspace->historyAsList("projects", lastProjects);
      int ix = 0;
      while (proj.isEmpty() && ix < lastProjects.size()) {
         QFileInfo dir(lastProjects.at(ix));
         if (dir.isDir() && dir.isWritable())
            proj = lastProjects.at(ix);
      }
      if (proj.isEmpty())
         proj = QDir::homePath();
   }
//	changeProject(proj);
   //connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
   connect(this, SIGNAL(xChanged(int)), this, SLOT(geometryChanged(int)));
   connect(this, SIGNAL(yChanged(int)), this, SLOT(geometryChanged(int)));
   connect(this, SIGNAL(widthChanged(int)), this, SLOT(geometryChanged(int)));
   connect(this, SIGNAL(heightChanged(int)), this, SLOT(geometryChanged(int)));
   connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
   Perspective* mainPerspective = new StartPerspective(this);
   m_perspectives.addPerspective(mainPerspective);
   m_perspectives.change(mainPerspective->name());
   m_perspectives.addPerspective(new ProjectPerspective(proj, this));
   m_delayedStorage = new ReDelayedStorage(m_workspace->historyFile())
}

/**
 * Handles the change of the geometry.
 *
 * @param value	changed x, y, width or height
 */
void MainWindow::geometryChanged(int value) {
   ReUseParameter(value);
   m_delayedStorage->storeWindow(this);
}

void MainWindow::timeout() {
}

/**
 * Destructor.
 */
MainWindow::~MainWindow() {
}

/**
 * Change the current project.
 *
 * @param path	the directory containing the project data
 */
void MainWindow::changeProject(QString path) {
   ProjectPerspective* perspective = perspectives().project(true);
   if (perspective != NULL)
      perspective->changeProject(path);
}

/**
 * Change the workspace.
 *
 * @param path	the directory containing the workspace data
 */
void MainWindow::changeWorkspace(const QString& path) {
   delete m_workspace;
   m_workspace = new Workspace(path, m_logger);
}

/**
 * Calls the file selection dialog.
 */
void MainWindow::openFile(const QString& name) {
   ProjectPerspective* perspective = perspectives().project(true);
   if (perspective != NULL)
      perspective->openFile(name);
}

/**
 * Shows the "open project/file dialog".
 */
void MainWindow::open() {
   ProjectSelection dialog(this);
   dialog.exec();
}
/**
 * Returns the perspective list.
 *
 * @return the perspective list
 */
PerspectiveList& MainWindow::perspectives() {
   return m_perspectives;
}

/**
 * Returns the logger.
 *
 * @return	the logger
 */
ReLogger* MainWindow::logger() const {
   return m_logger;
}

/**
 * Returns the workspace.
 *
 * @return the workspace
 */
Workspace* MainWindow::workspace() const {
   return m_workspace;
}

