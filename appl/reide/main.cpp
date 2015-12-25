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
#include "reide.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
   const char* workspace = NULL;
   const char* project = NULL;
   for (int ix = 1; ix < argc; ix++) {
      if (argv[ix][0] == '-') {
         if (strcmp(argv[ix], "-w") == 0 && ix < argc - 1) {
            workspace = argv[++ix];
         } else if (strncmp(argv[ix], "--workspace=", 12) == 0) {
            workspace = argv[ix] + 12;
         } else if (strcmp(argv[ix], "-p") == 0 && ix < argc - 1) {
            project = argv[++ix];
         } else if (strncmp(argv[ix], "--project=", 10) == 0) {
            workspace = argv[ix] + 10;
         }
      }
   }
   QApplication a(argc, argv);
   ReLogger logger;
   ReDebugAppender appender;
   appender.setAutoDelete(false);
   logger.addAppender(&appender);
   MainWindow w(workspace, project, &logger);
   logger.log(LOG_INFO, 1, "start");
   w.show();
   return a.exec();
}
