/*
 * ReTraverser.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef OS_RETRAVERSER_HPP_
#define OS_RETRAVERSER_HPP_

#include "base/ReMatcher.hpp"
#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>

typedef DIR* FindFileHandle_t;
#endif
/** Returns whether a filetime is undefined.
 * @param time	the filetime to test
 * @return 		<code>true</code>: the given filetime is undefined
 */
inline bool filetimeIsUndefined(ReFileTime_t& time) {
#if defined __linux__
   return time.tv_sec == 0 && time.tv_nsec == 0;
#elif defined __WIN32__
   return time.dwHighDateTime == 0 && time.dwLowDateTime == 0;
#endif
}
/** Sets the filetime to undefined.
 * @param time	the filetime to clear
 */
inline void setFiletimeUndef(ReFileTime_t& time) {
#if defined __linux__
   time.tv_sec = time.tv_nsec = 0;
#elif defined __WIN32__
   time.dwHighDateTime = time.dwLowDateTime = 0;
#endif
}

class ReDirStatus_t {
public:
   enum Type_t {
      TF_UNDEF = 0,
      // single property flags:
      TF_SUBDIR = 1 << 0,
      TF_REGULAR = 1 << 1,
      TF_LINK = 1 << 2,
      TF_LINK_DIR = 1 << 3,
      TF_BLOCK = 1 << 4,
      TF_PIPE = 1 << 5,
      TF_CHAR = 1 << 6,
      TF_SOCKET = 1 << 7,
      TF_OTHER = 1 << 8,
      // collections:
      TC_SPECIAL = (TF_BLOCK | TF_CHAR | TF_SOCKET | TF_PIPE | TF_OTHER),
      TC_NON_DIR = (TC_SPECIAL | TF_LINK | TF_REGULAR),
      TC_ALL = (TF_SUBDIR | TC_NON_DIR | TF_LINK_DIR)
   };

public:
   ReDirStatus_t(ReLogger* logger);
public:
   const ReFileTime_t* accessed();
   ReFileSize_t fileSize();
   const char* filetimeAsString(QByteArray& buffer);
   bool findFirst();
   bool findNext();
   void freeEntry();
   const char* fullName();
   bool isDirectory();
   bool isDotDir() const;
   bool isLink();
   bool isRegular();
   const ReFileTime_t* modified();
   const char* node() const;
   const char* rightsAsString(QByteArray& buffer, bool numerical,
                              int ownerWidth);
   Type_t type();
   char typeAsChar();
public:
   static const char* filetimeToString(const ReFileTime_t* time,
                                       QByteArray& buffer);
   static time_t filetimeToTime(const ReFileTime_t* time);
#if defined __WIN32__
   static bool getFileOwner(HANDLE handle, const char* file, QByteArray& name,
                            ReLogger* logger = NULL);
   static bool getPrivilege(const char* privilege, ReLogger* logger);
#endif
   static void timeToFiletime(time_t time, ReFileTime_t& filetime);
public:
   QByteArray m_path;
   QByteArray m_fullName;
   int m_passNo;
   ReLogger* m_logger;
#ifdef __linux__
   DIR* m_handle;
   struct dirent* m_data;
   struct stat m_status;
public:
   struct stat* getStatus();
#elif defined WIN32
   HANDLE m_handle;
   WIN32_FIND_DATAA m_data;
   bool m_getPrivilege;
#endif
};
class ReDirEntryFilter {
public:
   ReDirEntryFilter();
   ~ReDirEntryFilter();
public:
   virtual void deserialize(const uint8_t*& sequence, size_t& length);
   bool match(ReDirStatus_t& entry);
   virtual QByteArray& serialize(QByteArray& sequence);
public:
   ReDirStatus_t::Type_t m_types;
   RePatternList* m_nodePatterns;
   RePatternList* m_pathPatterns;
   ReFileSize_t m_minSize;
   ReFileSize_t m_maxSize;
   ReFileTime_t m_minAge;
   ReFileTime_t m_maxAge;
   int m_minDepth;
   int m_maxDepth;
   bool m_allDirectories;
private:
   static int m_serialId;
};

class ReTraceUnit {
public:
   ReTraceUnit(int m_triggerCount = 10, int interval = 60);
   virtual ~ReTraceUnit();
public:
   /** Returns whether the the instance is triggered.
    * To avoid too the not cheap call of time() the trace unit uses a counter.
    * If the counter reaches a given level the time check should be done.
    * @return <code>true</code>: the counter has reached <code>m_triggerCount</code>
    */
   inline bool isCountTriggered() {
      bool rc = ++m_count % m_triggerCount == 0;
      return rc;
   }
   /** Tests whether a given waiting time has been gone since the last trace.
    * @return	<code>true</code>: the last trace has been done after
    *			at least <code>m_interval</code> seconds
    */
   inline bool isTimeTriggered() {
      time_t now = time(NULL);
      bool rc = now - m_lastTrace > m_interval;
      if (rc) {
         m_lastTrace = now;
      }
      return rc;
   }
   virtual bool trace(const char* message);
protected:
   int m_count;
   int m_triggerCount;
   time_t m_lastTrace;
   int m_interval;
   time_t m_startTime;
};

class ReDirTreeStatistic {
public:
   ReDirTreeStatistic();
public:
   const char* statisticAsString(QByteArray& buffer, bool append = false,
                                 const char* formatFiles = "%8d ", const char* formatSizes = "%12.6f",
                                 const char* formatDirs = "%7d ");
   /**
    * Resets the counters.
    */
   inline void clear() {
      m_files = m_directories = 0;
      m_sizes = 0ll;
   }
public:
   int m_directories;
   int m_files;
   int64_t m_sizes;
};

#define MAX_ENTRY_STACK_DEPTH 256
class ReTraverser: public ReDirTreeStatistic {
public:
   ReTraverser(const char* base, ReTraceUnit* tracer = NULL, ReLogger* logger =
                  NULL);
   virtual ~ReTraverser();
public:
   void changeBase(const char* base);
   /**
    * Return the number of entered directories .
    * @return	the number of directories entered until now
    */
   inline int directories() const {
      return m_directories;
   }
   /**
    * Return the number of found files.
    * @return	the number of files found until now
    */
   inline int files() const {
      return m_files;
   }
   /** Returns whether the current directory has changed since the last call.
    * @param state		IN/OUT: stored info about the current directory.
    * 					The value has no interest for the caller
    * @return			<code>true</code>: the path has been changed
    */
   inline bool hasChangedPath(int& state) {
      bool rc = m_directories > state;
      state = m_directories;
      return rc;
   }
   ReDirStatus_t* rawNextFile(int& level);
   ReDirStatus_t* nextFile(int& level, ReDirEntryFilter* filter = NULL);
   /** Sets the tree traversal algorithm.
    * @param depthFirst	<code>true</code>: files of the subdirectories will
    *                      be returned earlier
    */
   void setDepthFirst(bool depthFirst) {
      m_passNoForDirSearch = depthFirst ? 1 : 2;
   }
   /** Sets directory filter (pattern list).
    * @param pattern 	pattern list for the subdirs to be entered
    */
   inline void setDirPattern(RePatternList* pattern) {
      m_dirPatterns = pattern;
      if (pattern != NULL)
         m_dirPatterns->setIgnoreCase(true);
   }
   /** Sets the maximal depth.
    * @param value     the value to set
    */
   inline void setMaxLevel(int value) {
      m_maxLevel = value;
   }
   /** Sets the minimal depth.
    * @param value     the value to set
    */
   inline void setMinLevel(int value) {
      m_minLevel = value;
   }
   void setPropertiesFromFilter(ReDirEntryFilter* filter);
   /**
    * Return the sum of file lengths of the found files.
    * @return	the sum of file lengths of the files found until now
    */
   inline int64_t sizes() const {
      return m_sizes;
   }
   ReDirStatus_t* topOfStack(int offset = 0);
protected:
   void destroy();
   void freeEntry(int level);
   bool initEntry(const QByteArray& parent, const char* node, int level);
   /**
    * Tests whether a directory should be processed.
    * @param node		the base name of the subdir
    * @return 			<code>true</code>: the subdir will be processed<br>
    * 					<code>false</code>: do not enter this subdir
    */
   inline bool isAllowedDir(const char* node) {
      bool rc = m_dirPatterns->match(node);
      return rc;
   }
protected:
   int m_minLevel;
   int m_maxLevel;
   int m_level;
   QByteArray m_base;
   ReDirStatus_t* m_dirs[MAX_ENTRY_STACK_DEPTH];
   /// each directory will be passed twice: for all files + for directories only
   /// 1: depth first 2: breadth first
   int m_passNoForDirSearch;
   /// a subdirectory will be entered only if this pattern list matches
   /// if NULL any directory will be entered
   RePatternList* m_dirPatterns;
   ReDirTreeStatistic m_statistic;
   ReTraceUnit* m_tracer;
   ReLogger* m_logger;
};

#endif /* OS_RETRAVERSER_HPP_ */
