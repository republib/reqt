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

#include "base/rebase.hpp"
#include "gui/regui.hpp"
#include "mainwindow.hpp"
#include <QApplication>
char** g_argv;
int main(int argc, char* argv[]) {
   g_argv = argv;
   QString homeDir = argc > 1 ? argv[1] : "";
   QApplication a(argc, argv);
   MainWindow w(homeDir);
   w.show();
   return a.exec();
}
