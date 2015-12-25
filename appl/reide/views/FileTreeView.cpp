/*
 * FileTreeView.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"

const char* FileTreeView::NAME = "FileTree";
/**
 * Constructor.
 *
 * @param directory		the base directory of the tree
 * @param mainWindow	the parent (main window)
 */
FileTreeView::FileTreeView(const QString& directory, MainWindow* mainWindow) :
   View(NAME, mainWindow),
   m_fileTree(new ReFileTree(directory, mainWindow->logger())) {
}

FileTreeView::~FileTreeView() {
   delete m_fileTree;
}
