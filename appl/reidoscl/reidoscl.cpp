/*
 * reidoscl.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "reidoscl.hpp"
#include <QApplication>

class UpdateHandler {
public:
   UpdateHandler(ReProgramArgs* globalArgs);
public:
   int run();
   void help() {
      m_globalArgs->help(NULL);
      m_updateArgs->help(NULL);
   }
private:
   ReProgramArgs* m_globalArgs;
   ReProgramArgs* m_updateArgs;
};
/**
 * Constructor.
 *
 * @param globalArgs	the global arguments (useful for all modes)
 */
UpdateHandler::UpdateHandler(ReProgramArgs* globalArgs) :
   m_globalArgs(globalArgs),
   m_updateArgs(NULL) {
   m_updateArgs = new ReProgramArgs(
      I18N::s2b(
         QObject::tr("usage:") + "$0 <global_opts> sy(nchronize) <options> <source> <target>\n"
         + QObject::tr("Copies newer and missing files from <source> to <target>") + "\n"
         + QObject::tr("<source>: the source directory. This directory is never changed") + "\n"
         + QObject::tr("If <source> ends with the path separator the files of <source> will be copied directly to <target>") + "\n"
         + QObject::tr("Otherwise the node of <source> will be a subdirectory of <target>") + "\n"
         + QObject::tr("Examples:") + "\n"
         + "<source>: /home/ <target>: "
         + QObject::tr("/trg /home/abc.txt will be copied to /trg/abc.txt") + "\n"
         + "<source>: /home <target>:"
         + QObject::tr("/trg /home/abc.txt will be copied to /trg/home/abc.txt") + "\n"
         + "<target>: "
         + QObject::tr("the target directory")
      ),
      "$0 up -p *,*.txt,*.odt --dir-patterns=*,.git,.* /home /backup\n"
      "$0 update --file-pattern=*,-*.bak,-*~ -P *,-*cache* /work crypt:/media/nas"
   );
   m_updateArgs->addString("filePattern", I18N::s2b(
                              QObject::tr("a comma separated list of file patterns to include/exclude files to copy.") + "\n"
                              + QObject::tr("An exclude pattern begins with '-'. Placeholder is '*' (for any string)") + "\n"
                              + QObject::tr("Examples:")
                           ) + "\n"
                           "--file-pattern=*,-*.bak\n"
                           "-p *.txt,*.doc,*.odt",
                           'p', "file-pattern", false, "*");
   m_updateArgs->addString("dirPattern", I18N::s2b(
                              QObject::tr("a comma separated list of directory patterns to include/exclude directories from processing.")
                              + "\n"
                              + QObject::tr("An exclude pattern begins with '-'. Placeholder is '*' (for any string)") + "\n"
                              + QObject::tr("Examples:") + "\n"
                              + "--dir-pattern=;*;-.cache\n"
                              + "-P *,-.git,.*"),
                           'P', "dir-pattern", false, "*");
   m_updateArgs->setProgramName(globalArgs->programName());
   m_updateArgs->init(globalArgs->argCount(), globalArgs->args(), false);
}

int UpdateHandler::run() {
   int rc = 0;
   if (m_updateArgs->argCount() < 2)
      m_updateArgs->help(I18N::s2b(QObject::tr("too few arguments")).constData());
   else {
   }
   //ReFileSystem*
   return rc;
}

/**
 * Handles the mode "help".
 *
 * @param args	the program arguments
 */
void handleHelp(ReProgramArgs& args) {
   QByteArray arg0 = args.shift();
   args.help(NULL);
   if (QByteArray("update").startsWith(arg0)) {
      UpdateHandler handler(&args);
      handler.help();
   } else if (QByteArray("help").startsWith(arg0)) {
      printf("%s help <mode>\n%s%s\n", args.programName(),
             ReProgramArgs::PREFIX_LINE_OPTION,
             I18N::s2b(QObject::tr("prints a description of the usage of <mode>")).constData());
   } else {
      printf("+++ %s\n",
             I18N::s2b(QObject::tr("unknown <mode>:") + " " + arg0).constData());
   }
}

int main(int argc, char* argv[]) {
   QApplication a(argc, argv);
   ReProgramArgs args(
      QObject::tr("Usage:") + " $0 <opts> <mode> <mode_opts> <mode_params>\n"
      + "<mode>:\n"
      + "c(ompare)     " + QObject::tr("compares two directories") + "\n"
      + "h(elp)        " + QObject::tr("shows a help text and examples") + "\n"
      + "sm(ooth)      " + QObject::tr("superflous files on target will be removed") + "\n"
      + "u(pdate)      " + QObject::tr("newer or missing files will be copied from source to target"),
      "$0 --verbose-mode=summary c /home /opt/backup/home\n"
      "$0 help update\n"
      "$0 -v upd --dir-pattern=;*;-cache --file-pattern=;*.txt;*.doc /home /opt /media/backup"
   );
   int rc = 0;
   try {
      args.init(argc, argv);
      QByteArray arg0;
      if (args.argCount() < 1) {
         args.help(I18N::s2b(QObject::tr("missing <mode>")).constData());
         rc = 1;
      } else if (QByteArray("help").startsWith(arg0 = args.shift())) {
         handleHelp(args);
      } else if (QByteArray("update").startsWith(arg0)) {
         UpdateHandler handler (&args);
         rc = handler.run();
      } else if (QByteArray("smooth").startsWith(arg0)) {
         UpdateHandler handler (&args);
         rc = handler.run();
      } else {
         args.help((I18N::s2b(QObject::tr("unknown <mode>:")) + " " + arg0).constData());
         rc = 1;
      }
   } catch( ReOptionException e) {
      args.help(NULL, true, stdout);
      rc = 2;
   }
   // a.exec();
   return rc;
}
