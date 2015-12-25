/*
 * filesearch.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#include "filesearch.hpp"
#include "ui_filesearch.h"

FileSearch::FileSearch(QWidget* parent) :
   QWidget(parent),
   ui(new Ui::FileSearch) {
   ui->setupUi(this);
}

FileSearch::~FileSearch() {
   delete ui;
}
