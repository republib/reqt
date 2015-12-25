/*
 * filefinder.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef FILEFINDER_HPP
#define FILEFINDER_HPP
#include <QTableWidget>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

class TextFinder;
class FileFinder : public QThread {
public:
   FileFinder();
   ~FileFinder();
public:
   void clear();
   void fillTable(const QString& path, int depth);
   void run();
   void search();
   void setAppend(bool append);
   void setAnnouncer(ReAnnouncer* announcer);
   void setBaseDir(const QString& baseDir);
   void setFiletypes(const QDir::Filters& filetypes);
   void setExcludedDirs(const QStringList& excludedDirs);
   void setGuiQueue(ReGuiQueue* guiQueue);
   void setMaxDepth(int maxDepth);
   void setMaxHits(int maxHits);
   void setMaxSize(const int64_t& maxSize);
   void setMinDepth(int minDepth);
   void setMinSize(const int64_t& minSize);
   void setObserver(ReObserver* observer);
   void setOlderThan(const QDateTime& olderThan);
   void setPatterns(const QStringList& patterns);
   void setStop(bool stop);
   void setTable(QTableWidget* table);
   void setTextFinder(TextFinder* textFinder);
   void setYoungerThan(const QDateTime& youngerThan);
   const Statistics& statistics() const;

private:
   bool isExcludedDir(const QString& node);
   bool isValid(const QFileInfo& file);
private:
   QStringList m_patterns;
   QStringList m_antiPatterns;
   int64_t m_minSize;
   int64_t m_maxSize;
   QDateTime m_youngerThan;
   QDateTime m_olderThan;
   QDir::Filters m_fileTypes;
   int m_minDepth;
   int m_maxDepth;
   QString m_baseDir;
   bool m_checkDates;
   QStringList m_excludedDirs;
   // Only used to hold the search parameters:
   TextFinder* m_textFinder;
   QTableWidget* m_table;
   Statistics m_statistics;
   ReObserver* m_observer;
   ReGuiQueue* m_guiQueue;
   int m_maxHits;
   bool m_stop;
};

#endif // FILEFINDER_HPP
