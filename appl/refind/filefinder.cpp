/*
 * filefinder.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include <QDir>
#include "base/rebase.hpp"
#include "gui/regui.hpp"
#include "utils.hpp"
#include "mainwindow.hpp"
#include "filefinder.hpp"
#include "textfinder.hpp"

/**
 * Constructor.
 */
FileFinder::FileFinder() :
   m_patterns(),
   m_antiPatterns(),
   m_minSize(0),
   m_maxSize(-1),
   m_youngerThan(),
   m_olderThan(),
   m_fileTypes(QDir::NoFilter),
   m_minDepth(0),
   m_maxDepth(512),
   m_baseDir(""),
   m_checkDates(false),
   m_excludedDirs(),
   m_textFinder(NULL),
   m_table(NULL),
   m_statistics(),
   m_guiQueue(NULL),
   m_maxHits(0) {
   m_youngerThan.setMSecsSinceEpoch(0);
   m_olderThan.setMSecsSinceEpoch(0);
}

/**
 * Destructor.
 */
FileFinder::~FileFinder() {
}

/**
 * Resets the instance.
 */
void FileFinder::clear() {
   m_statistics.clear();
}

/**
 * Returns a string representation of the filesize.
 *
 * @param size  the size to convert
 * @return      the filesize as a string
 */
QString fileSize(int64_t size) {
   QString rc;
   rc.sprintf("%12.6f", (double) size / 1000000.0);
   return rc;
}
/**
 * Returns the type of the file.
 *
 * @param info  the file info
 * @return      a string describing the text
 */
QString typeOf(const QFileInfo& info) {
   QString rc;
   if (info.isSymLink()) {
      if (info.isDir())
         rc = QObject::tr("link (dir)");
      else
         rc = QObject::tr("link (file)");
   } else if (info.isDir())
      rc = QObject::tr("dir");
   else
      rc = QObject::tr("file");
   return rc;
}

/**
 * Sets the text finder parameter template.
 *
 * @param textFinder    the text finder instance containing the search parameter
 */
void FileFinder::setTextFinder(TextFinder* textFinder) {
   m_textFinder = textFinder;
}

/**
 * Fills the table with the data of the filtered files of a given directory.
 *
 * This method is recursive over all directories.
 *
 * @param path          the directory to inspect
 * @param depth         the depth of the directory (relative to base directory)
 */
void FileFinder::fillTable(const QString& path, int depth) {
   QFileInfoList entries;
#if defined __linux__
   QDir dir(path);
#elif defined WIN32
   QDir dir(path.length() == 2 && path[1] == ':' ? path + "\\" : path);
#endif
   QDir::Filters filters = m_fileTypes | QDir::NoDotAndDotDot;
   if (m_patterns.count() == 0)
      entries = dir.entryInfoList(filters, QDir::NoSort);
   else
      entries = dir.entryInfoList(m_patterns, filters, QDir::NoSort);
   //QString relativePath = ReQStringUtils::nativePath(
   //path.mid(1 + m_baseDir.length()));
   QString node, ext;
   QList<QRegExp*> antiPatterns;
   QStringList::const_iterator it3;
   for (it3 = m_antiPatterns.begin(); it3 != m_antiPatterns.end(); ++it3) {
      QString pattern(*it3);
      antiPatterns.append(new QRegExp(pattern, Qt::CaseInsensitive, QRegExp::Wildcard));
   }
   QList<QRegExp*>::const_iterator it2;
   QList <QFileInfo>::const_iterator it;
   clock_t nextUpdate = clock() + CLOCKS_PER_SEC;
   QString row, size, type, modified;
   for (it = entries.begin(); it != entries.end(); ++it) {
      node = it->fileName();
      if (node == "." || node == "..")
         continue;
      bool ignore = false;
      for (it2 = antiPatterns.begin(); it2 != antiPatterns.end(); ++it2) {
         if ((*it2)->indexIn(node) >= 0) {
            ignore = true;
            break;
         }
      }
      if (ignore)
         continue;
      if (m_stop)
         break;
      if (depth >= m_minDepth && isValid(*it)) {
         bool isDir = it->isDir();
         if (isDir)
            m_statistics.m_dirs++;
         else
            m_statistics.m_files++;
         row.clear();
         // TC_NODE, TC_EXT, TC_SIZE, TC_MODIFIED, TC_TYPE, TC_PATH
         int ix = node.lastIndexOf('.');
         ext = ix <= 0 ? "" : node.mid(ix + 1).toLower();
         type = typeOf(*it);
         if (isDir)
            size.clear();
         else
            size = fileSize(it->size());
         modified = it->lastModified().toString("yyyy.MM.dd/hh:mm:ss");
         row.reserve(6 + node.length() + ext.length() + size.length()
                     + modified.length() + type.length() + path.length());
         row.append('\t').append(node).append('\t').append(ext);
         row.append('\t').append(size).append('\t').append(modified);
         row.append('\t').append(type).append('\t').append(path);
         if (!isDir)
            m_statistics.m_bytes += it->size();
         m_guiQueue->pushBack(ReGuiQueueItem(ReGuiQueueItem::NewTableRow, m_table, row));
         if (--m_maxHits <= 0)
            break;
      }
      clock_t now = clock();
      if (now > nextUpdate) {
         m_guiQueue->pushBack(ReGuiQueueItem(ReGuiQueueItem::LogMessage, NULL, path));
         nextUpdate = now + CLOCKS_PER_SEC;
      }
   }
   for (it2 = antiPatterns.begin(); it2 != antiPatterns.end(); ++it2) {
      delete *it2;
   }
   if (! m_stop && (depth < m_maxDepth || m_maxDepth < 0) && m_maxHits > 0) {
      entries = dir.entryInfoList(
                   QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::AllDirs, QDir::NoSort);
      bool filtered = m_excludedDirs.length() > 0;
      for (it = entries.begin(); it != entries.end(); ++it) {
         QString node = it->fileName();
         if (!filtered || !isExcludedDir(node))
            fillTable(path + QDir::separator() + node, depth + 1);
      }
   }
}
/**
 * Runs a file search in a second thread.
 */
void FileFinder::run() {
   search();
}

/**
 * Fills the table with the data of the filtered files of a given directory.
 */
void FileFinder::search() {
   clock_t start = clock();
   setStop(false);
   m_statistics.clear();
   QString path = ReFileUtils::nativePath(m_baseDir);
   path = ReQStringUtils::chomp(path, OS_SEPARATOR);
   fillTable(path, 0);
   m_statistics.m_runtimeSeconds = (double) (clock() - start)
                                   / CLOCKS_PER_SEC;
   QString msg;
   msg.sprintf(
      I18N::s2b(QObject::tr(
                   "Found: %d dir(s) and %d file(s) with %.6f MByte. Duration of the search: %.3f sec")),
      m_statistics.m_dirs, m_statistics.m_files,
      m_statistics.m_bytes / 1000000.0, m_statistics.m_runtimeSeconds);
   m_guiQueue->pushBack(ReGuiQueueItem(ReGuiQueueItem::ReadyMessage, NULL, msg));
}

/**
 * Checks whether a filename matches the list of excluded directories.
 *
 * @param node  filename to check
 * @return      <code>true</code>: the node is part of the excluded dirs
 */
bool FileFinder::isExcludedDir(const QString& node) {
   bool rc = false;
   QList <QString>::iterator it;
   for (it = m_excludedDirs.begin(); it != m_excludedDirs.end(); ++it) {
      if (QString::compare(node, *it, Qt::CaseInsensitive) == 0) {
         rc = true;
         break;
      }
   }
   return rc;
}

/**
 * Tests whether a file matches the filter conditions.
 *
 * @param file  the file to inspect
 * @return      <code>true</code>: the file matches the filter condition
 */
bool FileFinder::isValid(const QFileInfo& file) {
   int64_t size = file.size();
   bool rc = size >= m_minSize && (m_maxSize < 0 || size <= m_maxSize);
   bool checkYounger;
   if (rc
         && ((checkYounger = m_youngerThan.toMSecsSinceEpoch() > 0)
             || m_olderThan.toMSecsSinceEpoch() > 0)) {
      QDateTime date = file.lastModified();
      rc = !checkYounger || date >= m_youngerThan;
      if (rc)
         rc = m_olderThan.toMSecsSinceEpoch() == 0 || date <= m_olderThan;
   }
   if (rc && m_textFinder != NULL) {
      QString full = file.absoluteFilePath();
      TextFinder textFinder(full, file.size());
      textFinder.getSearchParameter(*m_textFinder);
      rc = textFinder.contains();
   }
   return rc;
}
/**
 * Sets the stop flag.
 *
 * @param stop  <code>true</code>: the file search must be stopped
 */
void FileFinder::setStop(bool stop) {
   m_stop = stop;
}


/**
 * Sets the maximal hit count.
 *
 * @param maxHits	the maximal hit count
 */
void FileFinder::setMaxHits(int maxHits) {
   m_maxHits = maxHits;
}

/**
 * Sets the gui queue for exchanging data to the main tread.
 *
 * @param guiQueue	the gui queue
 */
void FileFinder::setGuiQueue(ReGuiQueue* guiQueue) {
   m_guiQueue = guiQueue;
}

/**
 * Sets the base directory.
 *
 * @param baseDir   the directory where the search starts
 */
void FileFinder::setBaseDir(const QString& baseDir) {
   m_baseDir = baseDir;
}

/**
 * Sets the list of excluded directories.
 * @param excludedDirs  each entry of this list will not be entered for search
 */
void FileFinder::setExcludedDirs(const QStringList& excludedDirs) {
   m_excludedDirs = excludedDirs;
}

/**
 * Sets the file types.
 *
 * @param filetypes the file types to set
 */
void FileFinder::setFiletypes(const QDir::Filters& filetypes) {
   m_fileTypes = filetypes;
}

/**
 * Sets the maximum subdirectory depth.
 *
 * @param maxDepth  0: search only in the base directory<br>
 *                  otherwise: the maximum depth
 */
void FileFinder::setMaxDepth(int maxDepth) {
   m_maxDepth = maxDepth;
}

/**
 * Sets the maximum size.
 *
 * @param maxSize   the maximum size to set
 */
void FileFinder::setMaxSize(const int64_t& maxSize) {
   m_maxSize = maxSize;
}

/**
 * Sets the minimum subdirectory depth.
 *
 * @param minDepth  0: search in the base directory too<br>
 *                  otherwise: the minimum depth
 */
void FileFinder::setMinDepth(int minDepth) {
   m_minDepth = minDepth;
}

/**
 * Sets the minimum size.
 *
 * @param minSize   the minimum size to set
 */
void FileFinder::setMinSize(const int64_t& minSize) {
   m_minSize = minSize;
}

/**
 * Sets the observer object. Will be notified about the search exit.
 * @param observer
 */
void FileFinder::setObserver(ReObserver* observer) {
   m_observer = observer;
}

/**
 * Sets the date time which is the lower bound.
 *
 * @param olderThan the lower bound (including)
 */
void FileFinder::setOlderThan(const QDateTime& olderThan) {
   m_olderThan = olderThan;
   if (m_olderThan.toMSecsSinceEpoch() > 0)
      m_checkDates = true;
}

/**
 * Sets the file patterns.
 *
 * @param patterns  the patterns to set
 */
void FileFinder::setPatterns(const QStringList& patterns) {
   m_patterns.clear();
   m_antiPatterns.clear();
   QStringList::const_iterator it;
   for (it = patterns.begin(); it != patterns.end(); ++it) {
      QString pattern = *it;
      if (pattern.startsWith("-")) {
         if (pattern.length() > 1)
            m_antiPatterns.append(pattern.mid(1));
      } else {
         if (pattern.length() > 0)
            m_patterns.append(pattern);
      }
   }
}

/**
 * Sets the table to fill.
 * @param table		the table containing the found files
 */
void FileFinder::setTable(QTableWidget* table) {
   m_table = table;
}

/**
 * Sets the date time which is the upper bound.
 *
 * @param youngerThan   the upper bound (including)
 */
void FileFinder::setYoungerThan(const QDateTime& youngerThan) {
   m_youngerThan = youngerThan;
   if (youngerThan.toMSecsSinceEpoch() > 0)
      m_checkDates = true;
}


/**
 * Returns the statistics about the search.
 * @return the statistics
 */
const Statistics& FileFinder::statistics() const {
   return m_statistics;
}
