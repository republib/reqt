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


#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
   QApplication a(argc, argv);
   MainWindow w;
   w.show();
   return a.exec();
}
