/*
 * StartPerspective.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"

const char* StartPerspective::NAME = "Start";

/**
 * Constructor.
 *
 * @param mainWindow	the parent (main window)
 */
StartPerspective::StartPerspective(MainWindow* mainWindow) :
   Perspective(NAME, mainWindow) {
   setDefaultViews();
}

void StartPerspective::setDefaultViews() {
   if (m_views.size() == 0) {
      append(new StartView(m_mainWindow), Qt::NoDockWidgetArea);
   }
}
