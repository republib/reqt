/*
 * project.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

class MainWindow;

class Project: public ReSettings {
public:
   static const char* KEY_HISTORY_OPEN_FILES;
public:
   Project(const QString& path, MainWindow* mainWindow);
public:
   void openFile(const QString& filename);

private:
   MainWindow* m_mainWindow;
};

#endif // PROJECT_HPP
