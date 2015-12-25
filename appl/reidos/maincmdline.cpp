/*
 * maincmdline.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#include "reidoscmdline.hpp"
#include <QApplication>

class SyncHandler {
public:
   SyncHandler(ReProgramArgs* globalArgs);
private:
   ReProgramArgs* m_globalArgs;
   ReProgramArgs* m_syncArgs;
};
/**
 * Constructor.
 *
 * @param globalArgs	the global arguments (useful for all modes)
 */
SyncHandler::SyncHandler(ReProgramArgs* globalArgs) :
   m_globalArgs(globalArgs),
   m_syncArgs(NULL) {
   m_syncArgs = new ReProgramArgs(
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
      "$0 sy -p *,*.txt,*.odt --dir-patterns=*,.git,.* /home /backup\n"
      "$0 sync --file-pattern=*,-*.bak,-*~ -P *,-*cache* /work crypt:/media/nas"
   );
   m_syncArgs->addString("filePattern", I18N::s2b(
                            QObject::tr("a comma separated list of file patterns to include/exclude files to copy.") + "\n"
                            + QObject::tr("An exclude pattern begins with '-'. Placeholder is '*' (for any string)") + "\n"
                            + QObject::tr("Examples:")
                         ) + "\n"
                         "--file-pattern=*,-*.bak\n"
                         "-p *.txt,*.doc,*.odt",
                         'p', "file-pattern", false, "*");
   m_syncArgs->addString("dirPattern", I18N::s2b(
                            QObject::tr("a comma separated list of directory patterns to include/exclude directories from processing.")
                            + "\n"
                            + QObject::tr("An exclude pattern begins with '-'. Placeholder is '*' (for any string)") + "\n"
                            + QObject::tr("Examples:") + "\n"
                            + "--dir-pattern=;*;-.cache\n"
                            + "-P *,-.git,.*"),
                         'P', "dir-pattern", false, "*");
}

int main(int argc, char* argv[]) {
   QApplication a(argc, argv);
   ReProgramArgs args(
      QObject::tr("Usage:") + " $0 <opts> <mode> <mode_opts> <mode_params>\n"
      + "<mode>:\n"
      + "co(mpare)     " + QObject::tr("compares two directories") + "\n"
      + "sm(ooth)      " + QObject::tr("superflous files on target will be removed") + "\n"
      + "sy(nchronize) " + QObject::tr("newer or missing files will be copied from source to target"),
      "$0 help sync\n"
      "$0 -v sync --dir-pattern=;*;-cache --file-pattern=;*.txt;*.doc /x /y"
   );
   try {
      args.init(argc, argv);
      args.help(NULL, false, stdout);
   } catch( ReOptionException e) {
      args.help(NULL, true, stdout);
   }
   return a.exec();
}
