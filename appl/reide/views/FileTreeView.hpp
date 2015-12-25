/*
 * FileTreeView.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef FILETREEVIEW_HPP
#define FILETREEVIEW_HPP

/**
 * Base class of the views.
 *
 * A view is a widget displayed as a dock in the window displaying a perspective.
 */
class FileTreeView: public View {
public:
   static const char* NAME;
public:
   FileTreeView(const QString& directory, MainWindow* mainWindow);
   ~FileTreeView();
public:
   /** Returns the view specific widget.
    * @return the view specific widget
    */
   virtual QWidget* widget() {
      return m_fileTree;
   }
protected:
   ReFileTree* m_fileTree;
};

#endif // FILETREEVIEW_HPP
