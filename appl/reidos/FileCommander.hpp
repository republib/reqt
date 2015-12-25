/*
 * FileCommander.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef FILECOMMANDER_HPP
#define FILECOMMANDER_HPP

class IDosMain;

class FileCommander {
public:
   enum TableColumns {
      TC_MODIFIED, TC_SIZE, TC_EXT, TC_NODE,
   };
public:
   FileCommander(IDosMain* m_main);
public:
   void buildFs(const QString& url, ReFileSystem& filesystem);
   bool changeDirectory(const QString& path);
   void fillTable(QTableWidget* table);
protected:
   IDosMain* m_main;
   ReFileSystem* m_topFS;
   ReFileSystem* m_bottomFS;
   ReIncludeExcludeMatcher m_topMatcher;
   ReIncludeExcludeMatcher m_bottomMatcher;
   ReLogger* m_logger;
   bool m_topIsActive;
   QString m_dateFormat;
};

#endif // FILECOMMANDER_HPP
