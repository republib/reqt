/*
 * project.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"

const char* Project::KEY_HISTORY_OPEN_FILES = "openFiles";
/**
 * Constructor.
 *
 * @param path			the directory containing the configuration
 * @param mainWindow	the instance of the main window
 */
Project::Project(const QString& path, MainWindow* mainWindow) :
   ReSettings(path, ".reditor.proj", mainWindow->logger()),
   m_mainWindow(mainWindow) {
   QString filename = topOfHistory(KEY_HISTORY_OPEN_FILES);
   QFileInfo info(filename);
   if (!filename.isEmpty() && info.exists() && !info.isDir()) {
      m_mainWindow->openFile(filename);
   }
}

/**
 * Opens a file in the project directory.
 *
 * @param filename	the filename relative to the project directory
 */
void Project::openFile(const QString& filename) {
   QString full = m_path + OS_SEPARATOR_STR + filename;
   addHistoryEntry(KEY_HISTORY_OPEN_FILES, filename, ';', 1);
   m_mainWindow->openFile(full);
}

