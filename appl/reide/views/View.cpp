/*
 * View.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"

/**
 * Constructor.
 *
 * @param name			the name of the view
 * @param mainWindow	the parent (main window)
 */
View::View(const char* name, MainWindow* mainWindow) :
   m_name(name),
   m_mainWindow(mainWindow),
   m_rootLayout(NULL) {
}

/**
 * Returns the outermost widget of the view which must be a layout.
 *
 * @return return m_rootLayout;
 */
QLayout* View::rootLayout() {
   if (m_rootLayout == NULL) {
      m_rootLayout = new QVBoxLayout;
      m_rootLayout->addWidget(widget());
   }
   return m_rootLayout;
}
