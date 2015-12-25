/*
 * Perspective.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef PERSPECTIVE_HPP
#define PERSPECTIVE_HPP

class MainWindow;

class ViewInfo {
public:
   View* m_view;
   Qt::DockWidgetArea m_position;
   QDockWidget* m_dockWidget;
};

/**
 * Abstract base class for perspectives.
 *
 * A perspective is a collection of views displayed as docks of a window.
 */
class Perspective {
public:
   Perspective(const char* name, MainWindow* mainWindow);
   ~Perspective();
public:
   void activate();
   void append(View* view, Qt::DockWidgetArea location);
   /** Returns the perspective's name.
    * @return	the name
    */
   const QByteArray& name() const {
      return m_name;
   }
   /**
    * Prepares the <code>m_map</code> with the views.
    */
   virtual void setDefaultViews() = 0;
   void deactivate();

protected:
   QByteArray m_name;
   MainWindow* m_mainWindow;
   QList<ViewInfo*> m_views;
};
class ProjectPerspective;
/**
 * Manages a collection of <code>Perspective</code>s.
 */
class PerspectiveList {
public:
   PerspectiveList(MainWindow* mainWindow);
public:
   void addPerspective(Perspective* perspective);
   Perspective* change(const QByteArray& name);
   Perspective* get(const QByteArray& name);
   ProjectPerspective* project(bool activate = false);
private:
   Perspective* m_current;
   QMap<QByteArray, Perspective*> m_map;
   MainWindow* m_mainWindow;
};

#endif // PERSPECTIVE_HPP
