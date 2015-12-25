/*
 * View.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef VIEW_HPP
#define VIEW_HPP

class MainWindow;
#if ! defined REBASE_HPP
#include "base/rebase.hpp"
#endif
#if ! defined REGUI_HPP
#include "gui/regui.hpp"
#endif
/**
 * Base class of the views.
 *
 * A view is a widget displayed as a dock in the window displaying a perspective.
 */
class View {
public:
   View(const char* name, MainWindow* mainWindow);
public:
   /** Returns the view's name.
    * @return the name
    */
   const QByteArray& name() const {
      return m_name;
   }
   /**
    * Returns the view specific widget.
    * @return the view specific widget
    */
   virtual QWidget* widget() = 0;

   virtual QLayout* rootLayout();
protected:
   QByteArray m_name;
   MainWindow* m_mainWindow;
   QLayout* m_rootLayout;
};

#endif // VIEW_HPP
