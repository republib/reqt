/*
 * EditorView.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"

const char* EditorView::NAME = "Editor";
/**
 * Constructor.
 *
 * @param mainWindow	the parent (main window)
 */
EditorView::EditorView(MainWindow* mainWindow) :
   View(NAME, mainWindow),
   m_edit(new ReEdit(NULL)),
   m_file(NULL) {
   m_edit->setLines(&m_dummyFile);
}

/**
 * Destructor.
 */
EditorView::~EditorView() {
   delete m_edit;
   m_edit = NULL;
}

/**
 * Opens a file into the editor widget.
 *
 * @param filename	the filename with path
 */
void EditorView::openFile(const QString& filename) {
   delete m_file;
   m_file = new ReFile(filename, false, m_mainWindow->logger());
   m_edit->setLines(m_file);
}

