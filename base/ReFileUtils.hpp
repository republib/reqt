/*
 * ReFileUtils.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REFILEUTILS_HPP
#define REFILEUTILS_HPP

/**
 * Statistic of a directory tree.
 */
class ReTreeStatistic {
   ReTreeStatistic();
public:
   int m_files;
   int m_directories;
   int64_t m_fileSizes;
};

/**
 * Usefull static methods around files/directories missing in QT.
 */
class ReFileUtils {
public:
   static bool deleteTree(const QString& path, bool withBase,
                          ReLogger* logger);
   static QByteArray cleanPath(const char* path);
   static QString cleanPath(const QString& path);
   static QString extensionOf(const QString& filename);
   static QByteArray extensionOf(const char* filename);
   static bool isAbsolutPath(const QString& path);
   static bool isAbsolutPath(const char* path);
   /** Returns a path with native separators.
    * QT under windows can operator with 2 separators: '\\' and '/'.
    * '\\' is the native separator.
    * @param path	the path to inspect
    * @return the path with native separators.
    */
   inline static QString nativePath(QString path) {
#if defined __linux__
      return path;
#elif defined WIN32
      return path.replace(OS_2nd_SEPARATOR, OS_SEPARATOR);
#endif
   }
   /** Returns a path with native separators.
    * QT under windows can operator with 2 separators: '\\' and '/'.
    * '\\' is the native separator.
    * @param path	the path to inspect
    * @return the path with native separators.
    */
   inline static QByteArray nativePath(QByteArray path) {
#if defined __linux__
      return path;
#elif defined WIN32
      return path.replace(OS_2nd_SEPARATOR, OS_SEPARATOR);
#endif
   }
   static QString nodeOf(const QString& filename);
   static QByteArray nodeOf(const char* filename);
   static QString parentOf(const QString& filename);
   static QString pathAppend(const QString& base, const QString& path);
   static QByteArray pathAppend(const char* base, const char* path);
   static QByteArray& readFromFile(const char* filename, QByteArray& buffer);
   static QString replaceExtension(const QString& path, const QString& ext);
   static QByteArray replaceExtension(const char* path, const char* ext);
   static int seek(FILE* file, int64_t offset, int whence);
   static bool setTimes(const char* filename, const QDateTime& modified,
                        const QDateTime& accessed = m_undefinedTime, ReLogger* logger = NULL);
   static void splitUrl(const QString& url, QString* protocol, QString* host,
                        QString* path, QString* node, QString* params = NULL);
   static int64_t tell(FILE* file);
   static QByteArray tempDir(const char* node, const char* parent = NULL,
                             bool withSeparator = true);
   static QByteArray tempDirEmpty(const char* node, const char* parent = NULL,
                                  bool withSeparator = true);
   static QByteArray tempFile(const char* node, const char* parent = NULL,
                              bool deleteIfExists = true);
   static void writeToFile(const char* filename, const char* content,
                           size_t contentLength = (size_t) - 1, const char* mode = "w");
public:
   static QDateTime m_undefinedTime;
};

#endif // REFILEUTILS_HPP
