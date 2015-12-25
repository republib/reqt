/*
 * ReFileUtils.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"

enum {
   LOC_DELETE_TREE_1 = LOC_FIRST_OF(LOC_FILE), // 11801
   LOC_DELETE_TREE_2,		// 11802
   LOC_DELETE_TREE_3,		// 11803
   LOC_SET_TIMES_1,		// 11804
};

QDateTime ReFileUtils::m_undefinedTime;

/**
 * Constructor.
 */
ReTreeStatistic::ReTreeStatistic() :
   m_files(0),
   m_directories(0),
   m_fileSizes(0L) {
}

/**
 * Appends a relative path to base directory name (absolute or relative).
 *
 * @param base	the base directory, relative or absolute
 * @param path	a relative path (relative to <code>base</code>)
 * @return      <code>path</code> if it is an absolute path<br>
 *				otherwise: the combined path
 */
QString ReFileUtils::pathAppend(const QString& base, const QString& path) {
   QString rc;
   if (isAbsolutPath(path))
      rc = path;
   else if (!base.isEmpty())
      rc = QDir::cleanPath(base + OS_SEPARATOR + path);
   else {
      rc = path;
      rc.replace("\\", "/");
      if (path.startsWith("/"))
         rc.remove(0, 1);
   }
   return rc;
}

/**
 * Appends a relative path to base directory name (absolute or relative).
 *
 * @param base	the base directory, relative or absolute
 * @param path	a relative path (relative to <code>base</code>)
 * @return      <code>path</code> if it is an absolute path<br>
 *				otherwise: the combined path
 */
QByteArray ReFileUtils::pathAppend(const char* base, const char* path) {
   QByteArray rc;
   if (base[0] != '\0') {
      rc.append(base).append(OS_SEPARATOR).append(path);
   } else {
      rc = path;
   }
   return cleanPath(rc.constData());
}

/**
 * Delete a directory tree.
 *
 * @param path		the directory to delete
 * @param withBase	<code>true</code>: the directory itself will be deleted<br>
 *					<code>false</code>: only all files/dirs inside will be
 *					deleted
 * @param logger	NULL or error logger
 * @return			<code>true</code>:  all files deleted<br>
 *					<code>false</code>: at least one deletion failed
 */

bool ReFileUtils::deleteTree(const QString& path, bool withBase,
                             ReLogger* logger) {
   bool rc = true;
   QDir dir(path);
   if (dir.exists(path)) {
      QFileInfo info;
      QStringList names = dir.entryList(
                             QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs
                             | QDir::Files);
      QStringList::const_iterator it;
      for (it = names.constBegin(); it != names.constEnd(); ++it) {
         QString full(path);
         full.append(OS_SEPARATOR_STR).append(*it);
         QFileInfo info(full);
         if (info.isDir()) {
            if (!deleteTree(full, false, logger))
               rc = false;
            else if (_rmdir(I18N::s2b(full)) != 0) {
               rc = false;
               if (logger != NULL)
                  logger->logv(LOG_ERROR, LOC_DELETE_TREE_1,
                               "cannot delete directory (%d): %s", errno,
                               I18N::s2b(full).constData());
            }
         } else {
            if (!QFile::remove(full)) {
               rc = false;
               if (logger != NULL)
                  logger->logv(LOG_ERROR, LOC_DELETE_TREE_2,
                               "cannot delete file (%d): %s", errno,
                               I18N::s2b(full).constData());
            }
         }
      }
   }
   if (withBase && (_rmdir(I18N::s2b(path))) != 0) {
      rc = false;
      logger->logv(LOG_ERROR, LOC_DELETE_TREE_3,
                   "cannot delete directory (%d): %s", errno, I18N::s2b(path).constData());
   }
   return rc;
}

/**
 * Extracts the extension of a filename.
 *
 * The extension is the last part of a node separated by a '.'.
 * The separator '.' in part of the extension.
 * If the node starts with the only dot the extension is empty.
 *
 * Works with '/' and with '\\'.
 *
 * @param filename  the filename (with or without path)
 * @return          "": no extension found<br>
 *                  otherwise: the extension of <code>filename</code>
 */
QString ReFileUtils::extensionOf(const QString& filename) {
   QString rc;
   int ix = max(0, filename.size() - 1);
   QChar cc;
   while (ix > 0) {
      if ((cc = filename.at(ix)) == QChar('.')) {
         if ((cc = filename.at(ix - 1)) != '/' && cc != '\\')
            rc = filename.mid(ix);
         break;
      } else if (cc == QChar('\\') || cc == QChar('/'))
         break;
      ix--;
   }
   return rc;
}

/**
 * Extracts the extension of a filename.
 *
 * The extension is the last part of a node separated by a '.'.
 * The separator '.' in part of the extension.
 * If the node starts with the only dot the extension is empty.
 *
 * Works with '/' and with '\\'.
 *
 * @param filename  the filename (with or without path)
 * @return          "": no extension found<br>
 *                  otherwise: the extension of <code>filename</code>
 */
QByteArray ReFileUtils::extensionOf(const char* filename) {
   QByteArray rc;
   if (filename[0] != '\0') {
      size_t ix = strlen(filename) - 1;
      char cc;
      while (ix > 0) {
         if ((cc = filename[ix]) == '.') {
            if ((cc = filename[ix - 1]) != '/' && cc != '\\')
               rc.append(filename + ix);
            break;
         } else if (cc == '\\' || cc == '/')
            break;
         ix--;
      }
   }
   return rc;
}

/**
 * Returns whether a path is an absolute path.
 *
 * @param path	the path to test
 * @return		<code>true</code>: the path is absolute<br>
 *				<code>false</code>: the path is relative
 */
bool ReFileUtils::isAbsolutPath(const QString& path) {
   bool rc;
#ifdef __linux__
   rc = path.startsWith(OS_SEPARATOR);
#else
   rc = path.length() > 3 && path.at(1) == ':' && path.at(2) == OS_SEPARATOR
        || path.startsWith("\\\\");
#endif
   return rc;
}

/**
 * Returns whether a path is an absolute path.
 *
 * @param path	the path to test
 * @return		<code>true</code>: the path is absolute<br>
 *				<code>false</code>: the path is relative
 */
bool ReFileUtils::isAbsolutPath(const char* path) {
   bool rc;
#ifdef __linux__
   rc = path[0] == OS_SEPARATOR;
#else
   rc = isalpha(path[0]) && path[1] == ':' && path[2] == OS_SEPARATOR
        || path[0] == OS_SEPARATOR && path[1] == OS_SEPARATOR;
#endif
   return rc;
}

/**
 * Extracts the node of a filename.
 *
 * The node is the filename without path.
 *
 * @param filename  the filename (with or without path)
 * @return          the node of <code>filename</code>
 */
QString ReFileUtils::nodeOf(const QString& filename) {
   QString rc;
   int ix = filename.size() - 1;
   while (ix >= 0) {
      if (filename[ix] == '/' || filename[ix] == '\\') {
         rc = filename.mid(ix + 1);
         break;
      }
      ix--;
   }
   if (ix < 0)
      rc = filename;
   return rc;
}

/**
 * Extracts the node of a filename.
 *
 * The node is the filename without path.
 *
 * @param filename  the filename (with or without path)
 * @return          the node of <code>filename</code>
 */
QByteArray ReFileUtils::nodeOf(const char* filename) {
   QByteArray rc;
   int ix = (int) strlen(filename) - 1;
   while (ix >= 0) {
      if (filename[ix] == '/' || filename[ix] == '\\') {
         rc.append(filename + ix + 1);
         break;
      }
      ix--;
   }
   if (ix < 0)
      rc = filename;
   return rc;
}

/**
 * Extracts the path of a full filename.
 *
 * @param filename  the filename (with or without path)
 * @return          "": no path available<br>
 *					"/": filename = "/"<br>
 *					otherwise: the path of <code>filename</code> including
 *					trailing separator
 */
QString ReFileUtils::parentOf(const QString& filename) {
   QString rc;
   int ix = filename.size() - 1;
   while (ix >= 0) {
      if (filename[ix] == '/' || filename[ix] == '\\') {
         rc = filename.mid(ix + 1);
         break;
      }
      ix--;
   }
   if (ix >= 0)
      rc = filename.mid(0, ix + 1);
   return rc;
}

/** Normalizes a file path.
 *
 * Removes duplicated slashes and "." and "..", but not leading "..".
 * Change the 2nd separator to the native separator, e.g. "/" to "\\"
 *
 * @param path	path to clean
 * @return		the path without duplicated separators and "." and ".."
 */
QByteArray ReFileUtils::cleanPath(const char* path) {
   QByteArray rc;
   int length = strlen(path);
   rc.reserve(length);
   int minLength = 0;
#ifdef __WIN32__
   // UNC path, e.g. "\\server\share"?
   if ((path[0] == OS_SEPARATOR || path[0] == OS_2nd_SEPARATOR)
         && (path[1] == OS_SEPARATOR || path[1] == OS_2nd_SEPARATOR)) {
      rc.append("\\\\");
      path += 2;
      minLength = 2;
   }
#endif
   char cc = *path;
   int startNode = 0;
   if (cc == OS_SEPARATOR || cc == OS_2nd_SEPARATOR) {
      path++;
      startNode++;
      rc.append(OS_SEPARATOR);
   }
   while ((cc = *path++) != '\0') {
      if (cc != OS_SEPARATOR && cc != OS_2nd_SEPARATOR)
         rc.append(cc);
      // ignore duplicated slashes:
      else if (rc.length() > 0 && rc.at(rc.length() - 1) != OS_SEPARATOR) {
         int length = rc.length() - startNode;
         if (length == 1 && rc.at(startNode) == '.') {
            // ignore ".": remove it:
            rc.resize(startNode);
         } else if (length == 2 && rc.at(startNode) == '.' &&rc.at(startNode + 1) == '.') {
            // remove "..":
            rc.resize(startNode);
            // remove the last slash and node
            if (rc.length() > minLength) {
               rc.resize(rc.size() - 1);
               int ix = rc.lastIndexOf(OS_SEPARATOR);
               if (ix > minLength)
                  rc.resize(ix + 1);
               startNode = rc.length();
            }
         } else {
            rc.append(OS_SEPARATOR);
            startNode = rc.length();
         }
      }
   }
   length = rc.length() - startNode;
   if (length == 1 && rc.at(startNode) == '.') {
      // ignore ".": remove it:
      rc.resize(startNode);
   } else if (length == 2 && rc.at(startNode) == '.'
              && startNode > 0 &&rc.at(startNode + 1) == '.') {
      // remove "..":
      rc.resize(startNode);
      // remove the last slash and node
      if (rc.length() > minLength) {
         rc.resize(rc.size() - 1);
         int ix = rc.lastIndexOf(OS_SEPARATOR);
         if (ix > minLength)
            rc.resize(ix);
      }
   }
   return rc;
}

/** Normalizes a file path.
 *
 * Removes duplicated slashes and "." and "..", but not leading "..".
 * Change the 2nd separator to the native separator, e.g. "/" to "\\"
 *
 * @param path	path to clean
 * @return		the path without duplicated separators and "." and ".."
 */
QString ReFileUtils::cleanPath(const QString& path) {
   return (QString) cleanPath(I18N::s2b(path).constData());
}

/**
 * Reads a string from a given file.
 *
 * @param filename      name of the file to read
 * @param buffer        OUT: the buffer to write
 * @return              <code>buffer</code> (for chaining)
 */
QByteArray& ReFileUtils::readFromFile(const char* filename,
                                      QByteArray& buffer) {
   FILE* fp = fopen(filename, "r");
   if (fp != NULL) {
      struct stat info;
      stat(filename, &info);
      buffer.resize(info.st_size);
      size_t newLength = fread(buffer.data(), 1, info.st_size, fp);
      if (newLength != (size_t) info.st_size)
         buffer.truncate(newLength == (size_t) - 1 ? 0 : newLength);
      fclose(fp);
   }
   return buffer;
}

/**
 * Replaces the file extension of a filename.
 *
 * @param path  the filename to change
 * @param ext   the new file extension, e.g. ".txt"
 * @return      path with a new file extension
 */

QString ReFileUtils::replaceExtension(const QString& path, const QString& ext) {
   QString oldExt = extensionOf(path);
   QString rc;
   if (oldExt.isEmpty())
      rc = path + ext;
   else
      rc = path.mid(0, path.size() - oldExt.size()) + ext;
   return rc;
}
/**
 * Replaces the file extension of a filename.
 *
 * @param path  the filename to change
 * @param ext   the new file extension, e.g. ".txt"
 * @return      path with a new file extension
 */

QByteArray ReFileUtils::replaceExtension(const char* path, const char* ext) {
   QByteArray oldExt = extensionOf(path);
   QByteArray rc;
   if (oldExt.isEmpty())
      rc.append(path).append(ext);
   else {
      size_t length = strlen(path);
      size_t lengthExt = strlen(ext);
      rc.reserve(length - oldExt.size() + lengthExt);
      rc.append(path, length - oldExt.size());
      rc.append(ext);
   }
   return rc;
}

/**
 * Splits an URL into its parts.
 *
 * Examples:
 * <pre>url: "file:///path/name.ext"
 * protocol: "file:" host: "//" path: "/path/" node: "name.ext" param: ""
 *
 * url: "http://b2.de/xy/index.htm?id=1"
 * protocol: "http:" host: "//b2.de" path: "/xy/" node: "index.htm" param: "?id=1"
 * </pre>
 *
 * @param url		the URL to split
 * @param protocol	OUT: NULL or the protocol
 * @param host		OUT: NULL or the host part
 * @param path		OUT: NULL or the path without the last node
 * @param node		OUT: NULL or the last node of the path
 * @param params	OUT: NULL or the parameters (e.g. at http)
 */
void ReFileUtils::splitUrl(const QString& url, QString* protocol, QString* host,
                           QString* path, QString* node, QString* params) {
   if (protocol != NULL)
      *protocol = ReQStringUtils::m_empty;
   if (host != NULL)
      *host = ReQStringUtils::m_empty;
   if (path != NULL)
      *path = ReQStringUtils::m_empty;
   if (params != NULL)
      *params = ReQStringUtils::m_empty;
   int ix = url.indexOf(':');
   if (ix < 0) {
      ix = 0;
   } else {
      ix++;
      if (protocol != NULL)
         *protocol = url.mid(0, ix);
   }
   int start = ix;
   if (url.length() >= start + 2 && url.at(start) == '/' && url.at(start + 1) == '/') {
      ix = url.indexOf("/", start + 2);
      if (ix < 0)
         ix = start + 2;
      if (host != NULL)
         *host = url.mid(start, ix - start);
      start = ix;
   }
   ix = url.lastIndexOf(OS_SEPARATOR);
   if (ix < 0)
      ix = url.lastIndexOf(OS_2nd_SEPARATOR);
   if (ix >= 0) {
      ix++;
      if (path != NULL)
         *path = url.mid(start, ix - start);
      start = ix;
   }
   ix = url.indexOf('?', start);
   if (ix < 0) {
      if (node != NULL)
         *node = url.mid(start);
   } else {
      if (node != NULL)
         *node = url.mid(start, ix - start);
      if (params != NULL)
         *params = url.mid(ix);
   }
}

/**
 * Sets the filetimes.
 *
 * @param filename	name of the file to change
 * @param modified	the new modification time
 * @param accessed	the new access time.
 *					If <code>m_undefinedTime</code> the current time is taken
 * @param logger	the logger
 * @return			<code>true</code>: success
 */
bool ReFileUtils::setTimes(const char* filename, const QDateTime& modified,
                           const QDateTime& accessed, ReLogger* logger) {
   bool rc = true;
#if defined __linux__
   struct timeval vals[2];
   int64_t millisec = accessed == m_undefinedTime
                      ? QDateTime::currentMSecsSinceEpoch() : accessed.toMSecsSinceEpoch();
   vals[0].tv_sec = millisec / 1000;
   vals[0].tv_usec = millisec % 1000 * 1000;
   millisec = modified.toMSecsSinceEpoch();
   vals[1].tv_sec = millisec / 1000;
   vals[1].tv_usec = millisec % 1000 * 1000;
   if (utimes(filename, vals) != 0) {
      if (logger != NULL)
         logger->logv(LOG_ERROR, LOC_SET_TIMES_1,
                      "cannot change times (%d): $s", errno, filename);
      rc = false;
   }
#elif defined __WIN32__
#error "not implemented"
#endif
   return rc;
}

/** Sets the read position of a file.
 * @param file		file to process
 * @param offset	the position. @see <code>whence</code>
 * @param whence	SEEK_SET: offset is absolute (from file's start)
 *					SEEK_END: offset is relative to the file' end
 *					SEEK_CUR: offset is relative to the current position
 * @return			0: success<br>
 *					otherwise: error code
 */
int ReFileUtils::seek(FILE* file, int64_t offset, int whence) {
   int rc;
#if defined __linux__
   rc = fseeko(file, offset, whence);
#elif defined __WIN32__
   rc = _fseek64(file, offset, whence);
#endif
   return rc;
}

/**
 * Returns the current file position.
 *
 * @param file	file to process
 * @return	< 0: error occurred<br>
 *			otherwise: the current read/write position (from the file's start)
 */
int64_t ReFileUtils::tell(FILE* file) {
   int64_t rc;
#if defined __linux__
   rc = ftello(file);
#elif defined __WIN32__
   rc = _ftell64(file);
#endif
   return rc;
}

/**
 * @brief Returns the name of a directory in the temp dir.
 *
 * If the named directory does not exist it will be created.
 *
 * @param node          NULL or the node (name without path)
 * @param parent        NULL or a node of the parent
 * @param withSeparator true: the result ends with slash/backslash
 * @return              the name of an existing directory
 */
QByteArray ReFileUtils::tempDir(const char* node, const char* parent,
                                bool withSeparator) {
#if defined __linux__
   QByteArray temp("/tmp");
   static const char* firstVar = "TMP";
   static const char* secondVar = "TEMP";
#elif defined WIN32
   QByteArray temp("c:\\temp");
   static const char* firstVar = "TEMP";
   static const char* secondVar = "TMP";
#endif
   struct stat info;
   const char* ptr;
   if ((ptr = getenv(firstVar)) != NULL)
      temp = ptr;
   else if ((ptr = getenv(secondVar)) != NULL)
      temp = ptr;
#if defined WIN32
   temp.replace('\\', '/');
#endif
   if (temp.at(temp.length() - 1) != '/')
      temp += '/';
   if (parent != NULL) {
      temp += parent;
      if (stat(temp.constData(), &info) != 0)
         _mkdir(temp.constData());
      temp += '/';
   }
   if (node != NULL) {
      temp += node;
      if (stat(temp.data(), &info) != 0)
         _mkdir(temp.data());
      temp += '/';
   }
   if (!withSeparator)
      temp.resize(temp.length() - 1);
   return temp;
}
/**
 * @brief Returns the name of a directory in the temp dir.
 *
 * If the named directory does not exist it will be created.
 *
 * @param node          NULL or the node (name without path)
 * @param parent        NULL or a node of the parent
 * @param withSeparator true: the result ends with slash/backslash
 * @return              the name of an existing directory
 */
QByteArray ReFileUtils::tempDirEmpty(const char* node, const char* parent,
                                     bool withSeparator) {
   QByteArray rc = tempDir(node, parent, withSeparator);
   deleteTree(rc, false, NULL);
   return rc;
}

/**
 * @brief Returns a name of a file in a temporary directory.
 *
 * @param node              the file's name without path
 * @param parent            NULL or the name of a subdirectory the file will be inside
 * @param deleteIfExists    true: if the file exists it will be removed
 * @return                  the full name of a temporary file
 */
QByteArray ReFileUtils::tempFile(const char* node, const char* parent,
                                 bool deleteIfExists) {
   QByteArray rc(tempDir(parent));
   if (!rc.endsWith('/'))
      rc += '/';
   rc += node;
   struct stat info;
   if (deleteIfExists && stat(rc.constData(), &info) == 0)
      _unlink(rc.constData());
   return rc;
}

/**
 * Writes a string into a given file.
 *
 * @param filename      name of the file to write
 * @param content       the content to write
 * @param contentLength -1: <code>strlen(content)</code><br>
 *                      otherwise: the length of <code>content</code>
 * @param mode          file write mode: "w" (write) or "a" (append)
 */
void ReFileUtils::writeToFile(const char* filename, const char* content,
                              size_t contentLength, const char* mode) {
   FILE* fp = fopen(filename, mode);
   if (fp != NULL) {
      if (contentLength == (size_t) - 1)
         contentLength = strlen(content);
      fwrite(content, 1, contentLength, fp);
      fclose(fp);
   }
}
