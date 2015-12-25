/*
 * workspace.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef WORKSPACE_HPP
#define WORKSPACE_HPP

class Workspace: public ReSettings {
public:
   static const char* KEY_HISTORY_FILES;
   static const char* KEY_HISTORY_PROJECTS;
public:
   Workspace(const QString& path, ReLogger* logger);
};

#endif // WORKSPACE_HPP
