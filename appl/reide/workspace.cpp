/*
 * workspace.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "reide.hpp"

const char* Workspace::KEY_HISTORY_FILES = "files";
const char* Workspace::KEY_HISTORY_PROJECTS = "projecs";

/**
 * Constructor.
 *
 * @param path		the directory containing the configuration
 * @param logger	the logger
 */
Workspace::Workspace(const QString& path, ReLogger* logger) :
   ReSettings(path, ".reditor.ws", logger) {
   insertProperty(
      new ReProperty("editor.tabwidth", QObject::tr("Tabulator width"),
                     QObject::tr("Maximal length of the gap displaying a tabulator"),
                     "4", PT_INT, "[1,16]"));
   insertProperty(
      new ReProperty("history.max_projects",
                     QObject::tr("Maximal project entries"),
                     QObject::tr(
                        "Maximal number of projects in the 'last opened projects'"),
                     "20", PT_INT, "[1,100]"));
   insertProperty(
      new ReProperty("history.max_files", QObject::tr("Maximal file entries"),
                     QObject::tr("Maximal number of files in the 'last opened files'"),
                     "20", PT_INT, "[1,100]"));
}

