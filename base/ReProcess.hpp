/*
 * ReProcess.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef REPROCESS_HPP
#define REPROCESS_HPP


class ReProcess {
public:
   ReProcess();

public:
   static QByteArray executeAndRead(const QString& program,
                                    const QStringList& args, int timeout = 60);
   static QByteArray executeAndRead(const QByteArray& command, int timeout = 60);
   static QByteArray executeAndFilter(const char* command, const QString& regExpr);

};

#endif // REPROCESS_HPP
