/*
 * main.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include <QApplication>
#include "base/rebase.hpp"
#include "gui/regui.hpp"
#include "utils.hpp"
#include "mainwindow.hpp"

int main(int argc, char* argv[]) {
   QApplication app(argc, argv);
   QString startDir = argc > 1 ? argv[1] : "";
   QString homeDir = argc > 2 ? argv[2] : "";
   MainWindow w(startDir, homeDir);
   QObject::connect(&app, SIGNAL(aboutToQuit()), &w, SLOT(closing()));
   w.show();
   return app.exec();
}
