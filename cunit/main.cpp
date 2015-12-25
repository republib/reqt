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

#include <QCoreApplication>

int main(int argc, char* argv[]) {
   QCoreApplication a(argc, argv);
   void allTests();
   allTests();
   return a.exec();
}
