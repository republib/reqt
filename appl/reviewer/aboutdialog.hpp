/*
 * aboutdialog.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef ABOUTDIALOG_HPP
#define ABOUTDIALOG_HPP

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog: public QDialog {
   Q_OBJECT

public:
   explicit AboutDialog(QWidget* parent = 0);
   ~AboutDialog();

private:
   Ui::AboutDialog* ui;
};

#endif // ABOUTDIALOG_HPP
