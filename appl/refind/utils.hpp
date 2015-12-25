/*
 * utils.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef UTILS_HPP
#define UTILS_HPP
#ifndef REBASE_HPP
#include "base/rebase.hpp"
#endif
#include <QTableWidget>
#include "gui/ReStateStorage.hpp"
enum {
   COL_FILE_TYPE, COL_TITLE, COL_PROGRAM, COL_ARGUMENTS, COL_CURRENT_DIR
};
class ContextHandler {
public:
   enum IntrinsicType {
      IT_UNDEF, IT_COPY
   };

   enum DirMode {
      DM_UNDEF, DM_TO_PARENT, DM_TO_FILE
   };
   enum FileType {
      FT_UNDEF, FT_FILE, FT_DIR, FT_ALL
   };
public:
   ContextHandler();
   ContextHandler(IntrinsicType intrinsicType, const QString& text);
   IntrinsicType intrinsicType() const;
public:
   QString m_text;
   QString m_program;
   QString m_arguments;
   DirMode m_directoryMode;
   FileType m_fileType;
   IntrinsicType m_intrinsicType;
   void setIntrinsicType(const IntrinsicType& intrinsicType);
};

class ContextHandlerList {
public:
   ContextHandlerList();
   ~ContextHandlerList();
   ContextHandlerList(const ContextHandlerList& source);
   ContextHandlerList& operator =(const ContextHandlerList& source);
public:
   void addIntrinsics();
   void clear();
   ContextHandlerList& copy(const ContextHandlerList& source);
   QList <ContextHandler*>& list() {
      return m_list;
   }
   void save(ReStateStorage& storage);
   void restore(ReStateStorage& storage);
private:
   QList <ContextHandler*> m_list;
};

class Statistics {
public:
   Statistics();
public:
   void clear();
   void add(Statistics& source);
public:
   int m_dirs;
   int m_files;
   int64_t m_bytes;
   double m_runtimeSeconds;
};

#endif // UTILS_HPP
