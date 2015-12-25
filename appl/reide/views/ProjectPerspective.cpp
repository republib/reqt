/*
 * ProjectPerspective.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"

const char* ProjectPerspective::NAME = "Project";

const char* ProjectPerspective::KEY_HISTORY_OPEN_FILES = "openFiles";
/**
 * Constructor.
 *
 * @param path			home path (for the settings)
 * @param mainWindow	the parent (main window)
 */
ProjectPerspective::ProjectPerspective(const QString& path,
                                       MainWindow* mainWindow) :
   Perspective(NAME, mainWindow),
   ReSettings(path, ".reditor.proj", mainWindow->logger()),
   m_editorView(NULL) {
   setDefaultViews();
   //QString filename = topOfHistory(KEY_HISTORY_OPEN_FILES);
   //QFileInfo info(filename);
   //if (!filename.isEmpty() && info.exists() && !info.isDir()) {
   //	openFile(filename);
   //}
}

/**
 * Change the project.
 *
 * @param path	the path of the new current project
 */
void ProjectPerspective::changeProject(const QString& path) {
   setPath(path);
   int maxEntries = m_mainWindow->workspace()->intValue(
                       "history.max_projects");
   m_mainWindow->workspace()->addHistoryEntry(Workspace::KEY_HISTORY_PROJECTS,
         m_path, ';', maxEntries);
   /*<pre>
    if (m_fileTree == NULL) {
    m_fileTree = new ReFileTree(path, m_logger, this);
    m_dockProjectTree = new QDockWidget("", this);
    m_dockProjectTree->setWidget(m_fileTree);
    addDockWidget(Qt::LeftDockWidgetArea, m_dockProjectTree);
    }
    m_fileTree->setPath(path);
    m_dockProjectTree->setWindowTitle(
    tr("Project") + " " + ReQStringUtils::nodeOf(path));
    </pre>
    */
}

/**
 * Opens a file in the project directory.
 *
 * @param filename	the filename relative to the project directory
 */
void ProjectPerspective::openFile(const QString& filename) {
   QString full;
   if (ReFileUtils::isAbsolutPath(filename)) {
      full = filename;
   } else {
      full = m_path + OS_SEPARATOR_STR + filename;
   }
   m_editorView->openFile(full);
   addHistoryEntry(KEY_HISTORY_OPEN_FILES, full, ';', 1);
}

/**
 * Sets the name of the project directory.
 *
 * @param path	the directory name
 */
void ProjectPerspective::setPath(const QString& path) {
   if (!path.endsWith(OS_SEPARATOR_STR))
      ReSettings::setPath(path);
   else
      ReSettings::setPath(m_path.mid(0, path.size() - 1));
}

/**
 * Sets the default views of the project perspective.
 */
void ProjectPerspective::setDefaultViews() {
   if (m_views.size() == 0) {
      m_editorView = new EditorView(m_mainWindow);
      append(m_editorView, Qt::NoDockWidgetArea);
      append(new FileTreeView(QDir::homePath(), m_mainWindow),
             Qt::LeftDockWidgetArea);
   }
}

