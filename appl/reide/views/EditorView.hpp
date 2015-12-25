/*
 * EditorView.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef EDITORVIEW_HPP
#define EDITORVIEW_HPP

class MainWindow;

/**
 * Abstract base class of the views.
 *
 * A view is a widget displayed as a dock in the window displaying a perspective.
 */
class EditorView: public View {
public:
   static const char* NAME;
public:
   EditorView(MainWindow* mainWindow);
   ~EditorView();
public:
   void openFile(const QString& filename);

public:
   virtual QWidget* widget() {
      return m_edit;
   }
protected:
   ReEdit* m_edit;
   ReFile* m_file;
   ReLines m_dummyFile;
};

#endif // EDITORVIEW_HPP
