/*
 * filesearch.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef FILESEARCH_HPP
#define FILESEARCH_HPP

#include <QWidget>

namespace Ui {
class FileSearch;
}

class FileSearch : public QWidget {
   Q_OBJECT

public:
   explicit FileSearch(QWidget* parent = 0);
   ~FileSearch();

private:
   Ui::FileSearch* ui;
};

#endif // FILESEARCH_HPP
