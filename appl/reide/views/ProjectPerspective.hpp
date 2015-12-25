/*
 * ProjectPerspective.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef PROJECTPERSPECTIVE_HPP
#define PROJECTPERSPECTIVE_HPP

/**
 * Manages the aspects of a standard project.
 */
class ProjectPerspective: public Perspective, public ReSettings {
public:
   static const char* NAME;
public:
   static const char* KEY_HISTORY_OPEN_FILES;
public:
   ProjectPerspective(const QString& path, MainWindow* mainWindow);
public:
   virtual void setDefaultViews();
public:
   void openFile(const QString& filename);
   void changeProject(const QString& path);
   void setPath(const QString& path);

private:
   EditorView* m_editorView;
};

#endif // PROJECTPERSPECTIVE_HPP
