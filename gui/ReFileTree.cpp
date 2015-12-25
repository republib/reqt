/*
 * ReFileTree.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "gui/regui.hpp"

enum {
   LOC_BOOL_VALUE_1 = LOC_FIRST_OF(LOC_FILETREE), // 11901
};

/**
 * Constructor.
 *
 * @param path
 * @param logger
 * @param parent
 */
ReFileTree::ReFileTree(const QString& path, ReLogger* logger, QWidget* parent) :
   QTreeView(parent),
   m_path(path),
   m_logger(logger),
   m_model() {
   setPath(path);
   setModel(&m_model);
}

/**
 * Sets the directory path of the widget.
 *
 * @param path	the path of the base directory shown in the tree widget
 */
void ReFileTree::setPath(const QString& path) {
   m_model.setRootPath(path);
   QModelIndex idx = m_model.index(path);
   setRootIndex(idx);
}
