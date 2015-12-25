/*
 * ReProcess.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#include "base/rebase.hpp"

ReProcess::ReProcess() {
}

/**
 * Executes an external program and return its output.
 *
 * @param program	the program to execute
 * @param args		the program arguments
 * @param timeout	the maximal count of seconds waiting for program's end
 * @return			the output (stdout) of the command
 */
QByteArray ReProcess::executeAndRead(const QString& program,
                                     const QStringList& args, int timeout) {
   QProcess process;
   process.start(program, args, QIODevice::ReadOnly);
   process.waitForFinished(timeout * 1000);
   QByteArray rc = process.readAllStandardOutput();
   return rc;
}

/**
 * Executes an external program and return its output.
 *
 * @param command	command to execute
 * @param timeout	the maximal count of seconds waiting for program's end
 * @return			the output (stdout) of the command
 */
QByteArray ReProcess::executeAndRead(const QByteArray& command, int timeout) {
   QStringList args = QString(command).split(' ');
   QString program = args.at(0);
   args.removeAt(0);
   QByteArray rc = executeAndRead(program, args, timeout);
   return rc;
}

