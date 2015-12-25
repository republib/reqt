/*
 * ReFileTree.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REFILETREE_HPP
#define REFILETREE_HPP
#include <QTreeView>
#include <QFileSystemModel>

/**
 * Displays a directory with its files and subdirectories in a tree view.
 */
class ReFileTree: public QTreeView {
public:
   ReFileTree(const QString& path, ReLogger* logger, QWidget* parent = NULL);

public:
   void setPath(const QString& path);

protected:
   QString m_path;
   ReLogger* m_logger;
   QFileSystemModel m_model;
};

#endif // REFILETREE_HPP
