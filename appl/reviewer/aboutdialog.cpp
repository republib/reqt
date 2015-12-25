/*
 * aboutdialog.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "aboutdialog.hpp"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget* parent) :
   QDialog(parent), ui(new Ui::AboutDialog) {
   ui->setupUi(this);
}

AboutDialog::~AboutDialog() {
   delete ui;
}
