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


#include "reidos.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
   QApplication a(argc, argv);
   QString startDir = argc > 1 ? argv[1] : "";
   QString homeDir = argc > 2 ? argv[2] : "";
   IDosMain w(startDir, homeDir);
   w.show();
   return a.exec();
}
