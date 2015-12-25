/*
 * Perspective.cpp
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
 * @param name			the name of the perspective
 * @param mainWindow	the parent (main window)
 */
Perspective::Perspective(const char* name, MainWindow* mainWindow) :
   m_name(name),
   m_mainWindow(mainWindow) {
}

/**
 * Destructor.
 */
Perspective::~Perspective() {
   for (int ix = 0; ix < m_views.size(); ix++) {
      ViewInfo* info = m_views.at(ix);
      delete info->m_dockWidget;
      delete info;
   }
   m_views.clear();
}

/**
 * Sets the views as docks in the main window.
 */
void Perspective::activate() {
   // the first view is the central widget, not a dock
   QWidget* root = m_views.at(0)->m_view->widget();
   m_mainWindow->setCentralWidget(root);
   for (int ix = 1; ix < m_views.size(); ix++) {
      ViewInfo* info = m_views.at(ix);
      m_mainWindow->addDockWidget(info->m_position, info->m_dockWidget);
   }
}

/**
 * Appends a view to the view list.
 *
 * @param view		the view to append
 * @param position	the positon of the dock
 */
void Perspective::append(View* view, Qt::DockWidgetArea position) {
   ViewInfo* info = new ViewInfo();
   info->m_position = position;
   info->m_view = view;
   info->m_dockWidget = NULL;
   if (position != Qt::NoDockWidgetArea) {
      info->m_dockWidget = new QDockWidget(NULL);
      info->m_dockWidget->setWidget(view->widget());
   }
   m_views.append(info);
}

/**
 * Removes the views (as docks) from the main window.
 */
void Perspective::deactivate() {
   // the first view is the central widget, not a dock
   for (int ix = 1; ix < m_views.size(); ix++) {
      ViewInfo* info = m_views.at(ix);
      m_mainWindow->removeDockWidget(info->m_dockWidget);
   }
}

/**
 * Constructor.
 */
PerspectiveList::PerspectiveList(MainWindow* mainWindow) :
   m_current(NULL),
   m_map(),
   m_mainWindow(mainWindow) {
}

/**
 * Adds a perspective to the collection.
 *
 * @param perspective	the perspective to add
 */
void PerspectiveList::addPerspective(Perspective* perspective) {
   m_map.insert(perspective->name(), perspective);
}

/**
 * Deactivates the current perspective and activates another.
 *
 * @param name	name of the new active perspective
 */
Perspective* PerspectiveList::change(const QByteArray& name) {
   if (m_current == NULL || m_current->name() != name) {
      if (m_current != NULL)
         m_current->deactivate();
      m_current = get(name);
      if (m_current != NULL)
         m_current->activate();
   }
   return m_current;
}

/**
 * Returns the perspective given by name.
 *
 * @param name	the name of the wanted perspective
 * @return	NULL: not known<br>
 *			otherwise: the wanted perspective
 */
Perspective* PerspectiveList::get(const QByteArray& name) {
   Perspective* rc = m_map.value(name);
   return rc;
}

/**
 * Returns the project perspective.
 *
 * @param activate	<code>true</code>: the project perspective will be activated
 * @return			the project perspective
 */
ProjectPerspective* PerspectiveList::project(bool activate) {
   ProjectPerspective* rc;
   if (activate)
      rc = reinterpret_cast<ProjectPerspective*>(change(
               ProjectPerspective::NAME));
   else
      rc =
         reinterpret_cast<ProjectPerspective*>(get(ProjectPerspective::NAME));
   return rc;
}
