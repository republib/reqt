/*
 * StartPerspective.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef StartPerspective_HPP
#define StartPerspective_HPP

/**
 * Manages the aspects of a standard project.
 */
class StartPerspective: public Perspective {
public:
   static const char* NAME;
public:
   StartPerspective(MainWindow* mainWindow);
public:
   virtual void setDefaultViews();
};

#endif // StartPerspective_HPP
