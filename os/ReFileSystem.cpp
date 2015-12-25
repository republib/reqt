/*
 * ReFileSystem.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "os/reos.hpp"

enum {
   LOC_READ_1 = LOC_FIRST_OF(LOC_FILESYSTEM), // 12001
   LOC_WRITE_1,		// 12002
   LOC_WRITE_2,		// 12003
   LOC_WRITE_3,		// 12004
   LOC_WRITE_4,		// 12005
   LOC_READ_2,			// 12006
   LOC_MAKE_DIR_1,		// 12007
   LOC_MAKE_DIR_2,		// 12008
   LOC_MAKE_DIR_3,		// 12009
   LOC_SET_PROPERTIES_1,	// 12010
   LOC_SET_PROPERTIES_2,	// 12011
   LOC_SET_PROPERTIES_3,	// 12012
   LOC_REMOVE_1,		// 12013
   LOC_REMOVE_2,		// 12014
   LOC_REMOVE_3,		// 12015
   LOC_SET_PROPERTIES_4,	// 12016
   LOC_SET_PROPERTIES_5,	// 12017
   LOC_OPEN_1,				// 12018
   LOC_CREATE_FILE_1,		// 12019
};

/**
 * Constructor.
 *
 * @param name	the name of the filesystem
 * @param logger	the logger
 */
ReFileSystem::ReFileSystem(const QString& name, ReLogger* logger) :
   m_name(name),
#ifdef __linux__
   m_uid(geteuid()),
   m_gid(getegid()),
#endif
   m_writeable(false),
   m_logger(logger),
   m_buffer(),
   m_blocksize(4 * 1024 * 1024),
   m_undefinedTime() {
}

/**
 * Destructor.
 */
ReFileSystem::~ReFileSystem() {
}

/**
 * Returns the current blocksize (for copy operations).
 *
 * @return the current blocksize
 */
int ReFileSystem::blocksize() const {
   return m_blocksize;
}

/**
 * Returns a filesystem given by an URL.
 *
 * @param url	the url with infos about a filesystem
 * @return		<code>NULL</code>: unknown filesystem<br>
 *				otherwise: the filesystem described in the URL
 */
ReFileSystem* ReFileSystem::buildFromUrl(const QString& url) {
   ReFileSystem* rc = NULL;
   if (url.startsWith("file:")) {
#if defined __linux__
      rc = new ReLocalFileSystem("/", ReLogger::globalLogger());
#elif defined __WIN32__
#error "missing evaluating drive"
#endif
   }
   return rc;
}

/**
 * Copy a file from a source filesystem to the current directory of the instance.
 *
 * @param source		meta data of the source file (in current directory)
 * @param sourceFS		the filesystem containing the source
 * @param targetNode	the name of the target file (without path)
 * @return			EC_SUCCESS: success<br>
 *
 */
ReFileSystem::ErrorCode ReFileSystem::copy(ReFileMetaData& source,
      ReFileSystem& sourceFS, QString targetNode) {
   int blocksize = min(m_blocksize, sourceFS.blocksize());
   ErrorCode rc = EC_SUCCESS;
   ErrorCode rc2;
   int64_t size = 0;
   if (targetNode.isEmpty())
      targetNode = source.m_node;
   ReLeafFile* sourceFile = sourceFS.buildFile(source);
   ReFileMetaData targetMeta;
   if (! exists(targetNode, &targetMeta)) {
      rc = createFile(targetNode, false, &targetMeta);
   }
   if (rc == EC_SUCCESS) {
      ReLeafFile* targetFile = buildFile(targetMeta);
      if (sourceFile->open(false) == EC_SUCCESS
            && targetFile->open(true) == EC_SUCCESS) {
         while (rc == EC_SUCCESS && size < source.m_size) {
            if ((rc2 = sourceFile->read(blocksize, m_buffer))
                  != EC_SUCCESS)
               rc = rc2;
            else if ((rc2 = targetFile->write(m_buffer)) != EC_SUCCESS)
               rc = rc2;
            size += blocksize;
         }
      }
      sourceFile->close();
      targetFile->close();
      ReFileMetaData target(targetNode, ReFileUtils::m_undefinedTime,
                            ReFileUtils::m_undefinedTime, m_uid, m_gid);
      QString node = source.m_node;
      source.m_node = targetNode;
      setProperties(source, target, false);
      source.m_node = node;
      delete sourceFile;
      delete targetFile;
   }
   return rc;
}

/**
 * Returns the name of the current directory.
 *
 * @return	the name of the current directory
 */
const QString& ReFileSystem::directory() const {
   return m_directory;
}

/**
 * Returns a message describing the given error code.
 *
 * @param errorCode	code to convert
 * @return			a description of the error code
 */
QString ReFileSystem::errorMessage(ReFileSystem::ErrorCode errorCode) {
   QString rc;
   switch(errorCode) {
   case EC_SUCCESS:
      rc = QObject::tr("Success");
      break;
   case EC_PATH_NOT_FOUND:
      rc = QObject::tr("Path not found");
      break;
   case EC_NOT_ACCESSIBLE:
      rc = QObject::tr("not accessable");
      break;
   case EC_NOT_READABLE:
      rc = QObject::tr("not readable");
      break;
   case EC_READ:
      rc = QObject::tr("cannot read");
      break;
   case EC_FS_READ_ONLY:
      rc = QObject::tr("file is read only");
      break;
   case EC_NOT_WRITEABLE:
      rc = QObject::tr("file is not writeable");
      break;
   case EC_WRITE:
      rc = QObject::tr("cannot write");
      break;
   case EC_POSITION:
      rc = QObject::tr("cannot set new file position");
      break;
   case EC_FILE_ALREADY_EXISTS:
      rc = QObject::tr("file already exists");
      break;
   case EC_NOT_EXISTS:
      rc = QObject::tr("file does not exist");
      break;
   case EC_RENAME:
      rc = QObject::tr("file cannot renamed");
      break;
   case EC_HEADER_LENGTH:
      rc = QObject::tr("Header length mismatch");
      break;
   case EC_MARKER:
      rc = QObject::tr("marker mismatch");
      break;
   case EC_DIR_ALREADY_EXISTS:
      rc = QObject::tr("directory already exists");
      break;
   case EC_REMOTE_MKDIR:
      rc = QObject::tr("remote directory cannot be built");
      break;
   default:
      rc = QObject::tr("unknown error code: ") + QString::number(errorCode);
      break;
   }
   return rc;
}

/**
 * Returns the meta info of a file given by its url.
 *
 * Note: protocol and server of the URL will be ignored.
 *
 * @param url		filename as URL
 * @param metaData	OUT: the metadata of the found file if found
 * @return			<code>true</code>: the file has been found<br>
 *					<code>false</code>: the file was not found
 */
bool ReFileSystem::findByUrl(const QString& url, ReFileMetaData& metaData) {
   bool found = false;
   QString path, node;
   ReFileUtils::splitUrl(url, NULL, NULL, &path, &node);
   if (setDirectory(path) == EC_SUCCESS) {
      found = first(node, metaData);
   }
   return found;
}

/**
 * Finds the first file given by a pattern.
 *
 * @param pattern	pattern to find
 * @param file		OUT: the found file (valid only if return code is <code>true</code>
 * @param options	defines type of the found file, e.g. LO_FILE
 * @return			<code>true</code>: at least one file found<br>
 *					<code>false</code>: no file found
 */
bool ReFileSystem::first(const QString& pattern, ReFileMetaData& file,
                         ListOptions options) {
   ReFileMetaDataList list;
   QStringList names;
   names.append(pattern);
   ReIncludeExcludeMatcher matcher(names, ReQStringUtils::m_emptyList,
                                   Qt::CaseInsensitive, true);
   listInfos(matcher, list, options);
   bool rc = list.size() > 0;
   if (rc)
      file = list.at(0);
   return rc;
}

/**
 * Move a file from a source filesystem to the current directory of the instance.
 *
 * @param source		meta data of the source file (in current directory)
 * @param sourceFS		the filesystem containing the source
 * @param targetNode	the name of the target file (without path)
 * @return			EC_SUCCESS: success<br>
 *
 */
ReFileSystem::ErrorCode ReFileSystem::move(ReFileMetaData& source,
      ReFileSystem& sourceFS, QString targetNode) {
   if (targetNode.isEmpty())
      targetNode = source.m_node;
   ErrorCode rc = copy(source, sourceFS, targetNode);
   if (rc == EC_SUCCESS) {
      rc = sourceFS.remove(source);
   }
   return rc;
}

/**
 * Finds all nodes with a given prefix.
 *
 * @param prefix	IN: prefix to search OUT: common prefix of all found nodes,
 *					prefix "on", found nodes: 'onFilterA', 'onFilterB'
 *					result prefix: 'onFilter'
 * @param options	LO_FILES or LO_DIRS or both (or-combined)
 * @param list		list with the nodes starting with the prefix
 * @return			<code>list.length()</code>
 */
int ReFileSystem::nodesByPrefix(QString& prefix, ListOptions options,
                                QStringList& list) {
   int rc = 0;
   list.clear();
   ReFileMetaDataList entries;
   ReIncludeExcludeMatcher matcher(prefix + "*", Qt::CaseInsensitive, true);
   if (listInfos(matcher, entries, options) > 0) {
      ReFileMetaDataList::const_iterator it;
      for (it = entries.cbegin(); it != entries.cend(); ++it)
         list.append(it->m_node);
      if (list.length() == 1)
         prefix = list.at(0);
      else {
         prefix = ReQStringUtils::longestPrefix(list);
      }
   }
   return rc;
}

/**
 * Returns the operating system permission data of the filesystem.
 *
 * @return	the permission data: owner, group...
 */
ReOSPermissions ReFileSystem::osPermissions() const {
   return m_osPermissions;
}

/**
 * Tests whether a second filesystem has the same current directory.
 *
 * @param fileSystem	the other filesystem
 * @return				<code>true</code>: both filesystems uses the identical
 *						current directory: same filesytem type, same path
 */
bool ReFileSystem::sameCurrentDirectory(ReFileSystem& fileSystem) const {
   bool rc = fileSystem.m_name == m_name && fileSystem.directory() == m_directory;
   return rc;
}

void ReFileSystem::setOsPermissions(const ReOSPermissions& osPermissions) {
   m_osPermissions = osPermissions;
}

/**
 * Sets the filesystem to writeable or readonly.
 *
 * @param writeable	<code>true</code>: the filesystem is writeable
 */
void ReFileSystem::setWriteable(bool writeable) {
   m_writeable = writeable;
}

/**
 * Synchronizes recursivly the instances with another filesystem.
 *
 * The newer files in the source directory will be copied and files
 * which does not exist in the target.
 *
 * @param fileMatcher	only files matched by this will be processed
 * @param dirMatcher	only subdirectories matched by this will be processed
 * @param verboseLevel	defines the logging output (to stdout)
 * @param source		the source filesystem
 */
void ReFileSystem::synchronize(ReIncludeExcludeMatcher& fileMatcher,
                               ReIncludeExcludeMatcher& dirMatcher, VerboseLevel verboseLevel,
                               ReFileSystem& source) {
   ReFileMetaDataList sourceList;
   ReFileMetaData metaTarget;
   QByteArray dir;
   if (verboseLevel > V_SILENT)
      dir = I18N::s2b(directory());
   if (source.listInfos(fileMatcher, sourceList, LO_FILES) > 0) {
      QString sourceDir = source.directory();
      QString targetDir = directory();
      ReFileMetaDataList::iterator it;
      for (it = sourceList.begin(); it != sourceList.end(); ++it) {
         bool alreadyExists = exists(it->m_node, &metaTarget);
         if (! alreadyExists
               // precision of 2 seconds:
               || it->m_modified.currentMSecsSinceEpoch() - 2*1000
               > metaTarget.m_modified.currentMSecsSinceEpoch()) {
            if (verboseLevel > V_SILENT)
               printf("%c%s%s\n", alreadyExists ? '<' : '+',
                      dir.constData(), I18N::s2b(it->m_node).constData());
            copy(*it, source);
         } else if (verboseLevel > V_IMPORTANT) {
            printf("%c%s%s\n",
                   it->m_modified == metaTarget.m_modified ? '=' : '>',
                   dir.constData(), I18N::s2b(it->m_node).constData());
         }
      }
   }
   if (source.listInfos(dirMatcher, sourceList, LO_ONLY_DIRS_WITH_NAMEFILTER) > 0) {
      QString sourceDir = source.directory();
      QString targetDir = directory();
      ReFileMetaDataList::const_iterator it;
      for (it = sourceList.cbegin(); it != sourceList.cend(); ++it) {
         bool alreadyExists = exists(it->m_node, &metaTarget);
         if (! alreadyExists && S_ISDIR(metaTarget.m_mode)) {
            if (verboseLevel > V_SILENT)
               printf("-%s%s\n", dir.constData(),
                      I18N::s2b(it->m_node).constData());
            remove(metaTarget);
            alreadyExists = exists(it->m_node, &metaTarget);
         }
         if (! alreadyExists) {
            if (verboseLevel > V_SILENT)
               printf("&%s%s\n", dir.constData(),
                      I18N::s2b(it->m_node).constData());
            if (makeDir(it->m_node) != EC_SUCCESS)
               continue;
         }
         if (source.setDirectory(it->m_node) == EC_SUCCESS
               && setDirectory(it->m_node) == EC_SUCCESS)
            synchronize(fileMatcher, dirMatcher, verboseLevel, source);
         source.setDirectory(sourceDir);
         setDirectory(targetDir);
      }
   }
}
/**
 * Sets the size of the internal buffer for copy operations.
 *
 * @param blocksize	the new blocksize
 */
void ReFileSystem::setBlocksize(int blocksize) {
   m_blocksize = blocksize;
}

/**
 * Returns whether the filesystem is writeable.
 *
 * @return	<code>true</code>: modification in the filesystem are possible
 *			<code>false</code>: read only filesystem
 */
bool ReFileSystem::writeable() const {
   return m_writeable;
}

/**
 * Constructor.
 *
 * @param basePath	the root directory of the filesystem<br>
 *					Windows: e.g. "c:\" or "\\server\data"<br>
 *					Linux: Mount point, e.g. "/" or "/media/data"
 * @param logger
 */
ReLocalFileSystem::ReLocalFileSystem(const QString& basePath, ReLogger* logger) :
   ReFileSystem("localfs", logger),
   m_basePath(basePath),
   m_dir(basePath) {
   m_directory = basePath;
   ReQStringUtils::ensureLastChar(m_directory, OS_SEPARATOR);
   setWriteable(true);
}

/**
 * Destructor.
 */
ReLocalFileSystem::~ReLocalFileSystem() {
}

/**
 * Returns the base path of the filesystem.
 *
 * @return the base path
 */
const QString& ReLocalFileSystem::basePath() const {
   return m_basePath;
}

/**
 * Search a file in the current directory given by name.
 *
 * @param node		the filename without path
 * @param metaData	OUT: the info of the found file. May be NULL
 * @return			<code>true</code>: the file was found
 */
bool ReLocalFileSystem::exists(const QString& node, ReFileMetaData* metaData) const {
   QByteArray full(fullNameAsUTF8(node));
   struct stat info;
   bool rc = stat(full.constData(), &info) == 0;
   if (rc && metaData != NULL) {
      metaData->m_node = node;
      metaData->m_modified = QDateTime::fromTime_t(info.st_mtime);
      metaData->m_created = QDateTime::fromTime_t(info.st_ctime);
      metaData->m_owner = info.st_uid;
      metaData->m_group = info.st_gid;
      metaData->m_mode = info.st_mode;
      metaData->m_size = info.st_size;
   }
   return rc;
}

/**
 * Fills a list with the items of the current directory.
 *
 * @param matcher	the matching processor
 * @param list		OUT: the list of the found files
 * @param options	a set (bitmap) of options, e.g. LO_FILES | LO_DIRS
 * @return			the count of the found entries (<code>list.size()</code>)
 */
int ReLocalFileSystem::listInfos(const ReIncludeExcludeMatcher& matcher,
                                 ReFileMetaDataList& list, ListOptions options) {
   list.clear();
   bool withDirs = (options & LO_DIRS) != 0;
   bool withFiles = (options & LO_FILES) != 0;
   if (! (withDirs | withFiles))
      withDirs = withFiles = true;
   bool matchDirs = (options & LO_NAME_FILTER_FOR_DIRS) != 0;
   bool earlyMatching = matchDirs || ! withDirs;
   const QStringList& patterns = matcher.includes().patterns();
   QStringList nodes =
      ! earlyMatching || patterns.size() == 0
      ? m_dir.entryList() : m_dir.entryList(patterns);
   QStringList::const_iterator it;
   QByteArray full = I18N::s2b(m_directory);
   int pathLength = full.length();
   struct stat info;
   const ReListMatcher& excludeMatcher = matcher.excludes();
   bool excludeActive = excludeMatcher.patterns().length() > 0;
   for (it = nodes.cbegin(); it != nodes.cend(); it++) {
      QString node = *it;
      if (node != "." &&  node != "..") {
         if (earlyMatching) {
            if (excludeActive && excludeMatcher.matches(node))
               continue;
         }
         full.resize(pathLength);
         full.append(I18N::s2b(node));
         if (stat(full.constData(), &info) == 0) {
            bool isDir = S_ISDIR(info.st_mode);
            if ((isDir && ! withDirs) || (! isDir && ! withFiles))
               continue;
            if (! earlyMatching) {
               if ( (! isDir || matchDirs) && ! matcher.matches(node))
                  continue;
            }
            list.append(
               ReFileMetaData(node, QDateTime::fromTime_t(info.st_mtime),
                              QDateTime::fromTime_t(info.st_ctime), info.st_uid,
                              info.st_gid, info.st_mode, info.st_size));
         }
      }
   }
   return list.size();
}

/** Creates a directory.
 * @param node	the name without path (in the current directory)
 * @return		EC_SUCCESS: successful<br>
 *				EC_FS_READ_ONLY: the filesystem is readonly<br>
 *				EC_ALREADY_EXISTS: a file with this name exists<br>
 *				EC_NOT_ACCESSIBLE: creation failed
 */
ReFileSystem::ErrorCode ReLocalFileSystem::makeDir(const QString& node) {
   ErrorCode rc = EC_SUCCESS;
   if (!m_writeable) {
      m_logger->log(LOG_ERROR, LOC_MAKE_DIR_1, "filesystem is readonly");
      rc = EC_FS_READ_ONLY;
   } else if (m_dir.exists(node)) {
      m_logger->logv(LOG_ERROR, LOC_MAKE_DIR_2, "node exists already: %s",
                     fullNameAsUTF8(node).constData());
      rc = EC_FILE_ALREADY_EXISTS;
   } else if (!m_dir.mkdir(node)) {
      m_logger->logv(LOG_ERROR, LOC_MAKE_DIR_2, "cannot create directory: %s",
                     fullNameAsUTF8(node).constData());
      rc = EC_NOT_ACCESSIBLE;
   }
   return rc;
}

/**
 * Changes the current directory of the filesystem.
 *
 * @param path	the new current directory
 * @return		EC_SUCCESS: success<br>
 *				EC_PATH_NOT_FOUND	directory does not exist<br>
 *				EC_NOT_ACCESSIBLE	parent not readable
 */
ReFileSystem::ErrorCode ReLocalFileSystem::setDirectory(const QString& path) {
   ErrorCode rc;
   if (ReFileUtils::isAbsolutPath(path)) {
      m_dir.setPath(path);
      rc = m_dir.exists() ? EC_SUCCESS : EC_PATH_NOT_FOUND;
   } else {
      QString dir = ReFileUtils::pathAppend(m_directory, path);
      m_dir.setPath(dir);
      rc = m_dir.exists() ? EC_SUCCESS : EC_PATH_NOT_FOUND;
   }
   if (rc == EC_SUCCESS) {
      m_directory = m_dir.path();
      ReQStringUtils::ensureLastChar(m_directory, OS_SEPARATOR);
   }
   return rc;
}

/**
 * Creates a new file in the current directory.
 *
 * @param node				the name of the new file
 * @param inDirectoryOnly	<code>false</code>: the file is made physically<br>
 *							otherwise: the file exists only in the metadata
 *							of the directory
 * @param metadata			OUT: NULL or the metadata of the new file
 * @return					EC_SUCCESS: success<br>
 *							EC_ALREADY_EXISTS: the file already exists
 *							EC_WRITE: the file could not be written
 */
ReFileSystem::ErrorCode ReLocalFileSystem::createFile(const QString& node,
      bool inDirectoryOnly, ReFileMetaData* metadata) {
   ErrorCode rc = EC_SUCCESS;
   if (exists(node))
      rc = EC_ALREADY_EXISTS;
   else {
      if (metadata != NULL) {
         QDateTime now = QDateTime::currentDateTime();
         *metadata = ReFileMetaData(node, now, now, m_osPermissions.m_user,
                                    m_osPermissions.m_group, m_osPermissions.m_dirMode, 0);
      }
      if (! inDirectoryOnly) {
         QByteArray name(fullNameAsUTF8(node));
         FILE* fp = fopen(name.constData(), "w");
         if (fp == NULL) {
            rc = EC_WRITE;
            m_logger->logv(LOG_ERROR, LOC_CREATE_FILE_1, "cannot open (%d): %s",
                           errno, name.constData());
         } else
            fclose(fp);
      }
   }
   return rc;
}

/**
 * Returns the canonical form of a given path.
 *
 * @param path	path to convert
 * @return		all nodes of the parts which are links are replaced by its
 *				link targets
 */
QString ReLocalFileSystem::canonicalPathOf(const QString& path) {
   QString rc = ReFileUtils::cleanPath(path);
   assert(false);
   return rc;
}

/**
 * Removes a file or directory.
 *
 * @param node	the properties ot the node (in the current directory)
 * @return		EC_SUCCESS: successful<br>
 *				EC_FS_READ_ONLY: filesystem is readonly<br>
 *				EC_NOT_EXISTS: the node does not exist
 *				EC_NOT_ACCESSIBLE: removing failed
 *
 */
ReFileSystem::ErrorCode ReLocalFileSystem::remove(const ReFileMetaData& node) {
   ErrorCode rc = EC_SUCCESS;
   if (!m_writeable) {
      m_logger->log(LOG_ERROR, LOC_REMOVE_1, "remove(): filesystem is readonly");
      rc = EC_FS_READ_ONLY;
   } else if (!m_dir.exists(node.m_node)) {
      m_logger->logv(LOG_ERROR, LOC_REMOVE_2, "remove(): node does not exists: %s",
                     fullNameAsUTF8(node.m_node).constData());
      rc = EC_NOT_EXISTS;
   } else {
      if (S_ISDIR(node.m_mode)) {
         if (!m_dir.rmdir(node.m_node)) {
            m_logger->logv(LOG_ERROR, LOC_REMOVE_3,
                           "cannot remove directory: %s",
                           fullNameAsUTF8(node.m_node).constData());
            rc = EC_NOT_ACCESSIBLE;
         }
      } else {
         if (!m_dir.remove(node.m_node)) {
            m_logger->logv(LOG_ERROR, LOC_REMOVE_3,
                           "cannot remove file: %s",
                           fullNameAsUTF8(node.m_node).constData());
            rc = EC_NOT_ACCESSIBLE;
         }
      }
   }
   return rc;
}

/**
 * Sets the properties of a file in the current directory.
 *
 * @param source	the properties to copy
 * @param target	the properties of the file to change
 * @param force		<code>true</code>: try to change rights to enable other changes<br>
 *					<code>false</code>: current rights will be respected
 * @return			EC_SUCCESS: successful<br>
 *					EC_FS_READ_ONLY: filesystem is readonly<br>
 *					EC_ALREADY_EXISTS: renaming failed: target node exists already<br>
 *					EC_RENAME: renaming failed
 *
 */
ReFileSystem::ErrorCode ReLocalFileSystem::setProperties(
   const ReFileMetaData& source, ReFileMetaData& target, bool force) {
   ErrorCode rc = EC_SUCCESS;
   if (!m_writeable) {
      m_logger->log(LOG_ERROR, LOC_SET_PROPERTIES_1,
                    "filesystem is readonly");
      rc = EC_FS_READ_ONLY;
   } else
      do {
         QByteArray name;
         bool nameChanged = target.m_node != source.m_node;
         bool timeChanged = source.m_modified != target.m_modified
                            && source.m_modified != ReFileUtils::m_undefinedTime;
#ifdef __linux__
         bool modeChanged = (source.m_mode & ALLPERMS) != (target.m_mode & ALLPERMS)
                            && source.m_mode != (mode_t) -1;
         bool ownerChanged = (source.m_owner != target.m_owner
                              && source.m_owner != -1)
                             || (source.m_group != source.m_group
                                 && source.m_group != -1);
         if (force && m_uid != 0 && (nameChanged || timeChanged || modeChanged
                                     || ownerChanged)) {
            name = fullNameAsUTF8(target.m_node);
            chmod(name.constData(), ALLPERMS);
            modeChanged = true;
         }
#endif
         if (nameChanged) {
            if (m_dir.exists(source.m_node)) {
               if (name.length() == 0)
                  name = fullNameAsUTF8(target.m_node);
               rc = EC_FILE_ALREADY_EXISTS;
               m_logger->logv(LOG_ERROR, LOC_SET_PROPERTIES_2,
                              "renaming impossible: node exists: %s",
                              name.constData());
               break;
            } else if (!m_dir.rename(target.m_node, source.m_node)) {
               rc = EC_RENAME;
               if (name.length() == 0)
                  name = fullNameAsUTF8(target.m_node);
               m_logger->logv(LOG_ERROR, LOC_SET_PROPERTIES_3,
                              "renaming impossible: %s -> %s",
                              I18N::s2b(source.m_node).constData(), name.constData());
               break;
            } else {
               name.resize(0);
               target.m_node = source.m_node;
            }
         }
         if (timeChanged) {
            if (name.length() == 0)
               name = fullNameAsUTF8(target.m_node);
            if (!ReFileUtils::setTimes(name.constData(), source.m_modified,
                                       ReFileUtils::m_undefinedTime, m_logger))
               rc = EC_NOT_ACCESSIBLE;
         }
#ifdef __linux__
         if (ownerChanged) {
            int uid = source.m_owner == -1 ? target.m_owner : source.m_owner;
            int gid = source.m_group == -1 ? target.m_group : source.m_group;
            if (name.length() == 0)
               name = fullNameAsUTF8(target.m_node);
            if (chown(name.constData(), uid, gid) != 0) {
               rc = EC_NOT_ACCESSIBLE;
               m_logger->logv(LOG_ERROR, LOC_SET_PROPERTIES_5,
                              "changing owner/group is impossible: %s",
                              name.constData());
            }
         }
         if (modeChanged) {
            mode_t mode = source.m_mode == (mode_t) -1
                          ? target.m_mode : source.m_mode;
            if (chmod(name.constData(), mode & ALLPERMS) != 0) {
               rc = EC_NOT_ACCESSIBLE;
               m_logger->logv(LOG_ERROR, LOC_SET_PROPERTIES_4,
                              "changing permissions is impossible: %s",
                              name.constData());
            }
         }
#endif
      } while (false);
   return rc;
}

/**
 * Constructor.
 */
ReFileMetaData::ReFileMetaData() :
   m_node(),
   m_modified(),
   m_created(),
   m_size(-1),
   m_owner(-1),
   m_group(-1),
   m_mode(-1) {
}

/**
 * Constructor.
 *
 * @param node		the filename without path
 * @param modified	the modification date/time
 * @param created	the creation date/time
 * @param owner		the owner of the file (UID)
 * @param group		the group of the file (GID)
 * @param mode		rights and attributs of the file
 * @param size		the filesize (0 for directories)
 * @param id		an identifier of the file unique inside the directory
 */
ReFileMetaData::ReFileMetaData(const QString& node, const QDateTime& modified,
                               const QDateTime& created, int owner, int group, mode_t mode, int64_t size,
                               int32_t id) :
   m_node(node),
   m_modified(modified),
   m_created(created),
   m_size(size),
   m_owner(owner),
   m_group(group),
   m_id(id),
   m_mode(mode) {
}

/**
 * Destructor.
 */
ReFileMetaData::~ReFileMetaData() {
}
/**
 * Copy constructor.
 *
 * @param source	source to copy
 */
ReFileMetaData::ReFileMetaData(const ReFileMetaData& source) :
   m_node(source.m_node),
   m_modified(source.m_modified),
   m_created(source.m_created),
   m_size(source.m_size),
   m_owner(source.m_owner),
   m_group(source.m_group),
   m_id(source.m_id),
   m_mode(source.m_mode) {
}

/**
 * Assign operator.
 *
 * @param source	source to copy
 * @return			the instance itself
 */
ReFileMetaData& ReFileMetaData::operator =(const ReFileMetaData& source) {
   m_node = source.m_node;
   m_modified = source.m_modified;
   m_created = source.m_created;
   m_owner = source.m_owner;
   m_group = source.m_group;
   m_mode = source.m_mode;
   m_size = source.m_size;
   m_id = source.m_id;
   return *this;
}

/**
 * Constructor.
 */
ReOSPermissions::ReOSPermissions() :
   m_user(-1),
   m_group(-1),
   m_fileMode(0),
   m_dirMode(0) {
#if defined __linux__
   m_user = getuid();
   m_group = getgid();
   m_fileMode =  S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH;
   m_dirMode =  S_IWUSR | S_IRUSR | S_IXUSR | S_IWGRP | S_IRGRP
                | S_IXGRP | S_IROTH | S_IXOTH | __S_IFDIR;
#endif
}

/**
 * Copy constructor.
 * @param source	source to copy
 */
ReOSPermissions::ReOSPermissions(const ReOSPermissions& source) :
   m_user(source.m_user),
   m_group(source.m_group),
   m_fileMode(source.m_fileMode),
   m_dirMode(source.m_dirMode) {
}

/**
 * Assignment operator.
 *
 * @param source	source to copy
 * @return			the instance itself
 */
ReOSPermissions& ReOSPermissions::operator =(const ReOSPermissions& source) {
   m_user = source.m_user;
   m_group = source.m_group;
   m_fileMode = source.m_fileMode;
   m_dirMode = source.m_dirMode;
   return *this;
}

/**
 * Constructor.
 *
 * @param metaData	the meta data of the file
 * @param fullName	the name with path of the file
 * @param logger	the logger
 */
ReLocalLeafFile::ReLocalLeafFile(const ReFileMetaData& metaData,
                                 const QString& fullName, ReLogger* logger) :
   ReLeafFile(metaData, fullName, logger),
   m_fp(NULL) {
}

/**
 * Destructor.
 */
ReLocalLeafFile::~ReLocalLeafFile() {
}

/**
 * Opens a file for reading or writing.
 *
 * @param writeable	<code>true</code>: open for writing
 * @return			EC_SUCCESS: success<br>
 *					EC_CANNOT_OPEN: opening failed
 */
ReFileSystem::ErrorCode ReLocalLeafFile::open(bool writeable) {
   ReFileSystem::ErrorCode  rc = ReFileSystem::EC_SUCCESS;
   if ( (m_fp = fopen(I18N::s2b(m_fullName).constData(),
                      writeable ?"wb" : "rb")) == NULL) {
      rc = ReFileSystem::EC_NOT_EXISTS;
      m_logger->logv(LOG_ERROR, LOC_OPEN_1, "cannot open: %s",
                     I18N::s2b(m_fullName).constData());
   }
   return rc;
}

/** Frees the resources occupied by <code>open()</code>.
 *
 * @return			EC_SUCCESS: success
 */
ReFileSystem::ErrorCode ReLocalLeafFile::close() {
   ReFileSystem::ErrorCode  rc = ReFileSystem::EC_SUCCESS;
   if (m_fp != NULL) {
      fclose(m_fp);
      m_fp = NULL;
   }
   return rc;
}
/**
 * Reads data from the current position into a buffer.
 *
 * @param maxSize		number of bytes to read
 * @param buffer	OUT: content of the file
 * @return			EC_SUCCESS: success<br>
 *					EC_INVALID_STATE: file not open
 */
ReFileSystem::ErrorCode ReLocalLeafFile::read(int maxSize,
      QByteArray& buffer) {
   ReFileSystem::ErrorCode  rc = ReFileSystem::EC_SUCCESS;
   if (m_fp == NULL) {
      rc = ReFileSystem::EC_INVALID_STATE;
   } else {
      maxSize = min(maxSize, m_meta.m_size - ftell(m_fp));
      buffer.resize(maxSize);
      int nRead = 0;
      if ( (nRead = fread(buffer.data(), 1, maxSize, m_fp)) != maxSize) {
         rc = ReFileSystem::EC_READ;
         m_logger->logv(LOG_ERROR, LOC_READ_1, "cannnot read %s (%d): %d/%d",
                        I18N::s2b(m_fullName).constData(), errno, nRead, maxSize);
      }
   }
   return rc;
}

/**
 * Writes a buffer to a file at the current position.
 *
 * @param buffer	content to write
 * @return			EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReLocalLeafFile::write(const QByteArray& buffer) {
   ReFileSystem::ErrorCode  rc = ReFileSystem::EC_SUCCESS;
   if (m_fp == NULL) {
      rc = ReFileSystem::EC_INVALID_STATE;
   } else {
      int nWritten = 0;
      int nToWrite = buffer.length();
      if ( (nWritten = fwrite(buffer.constData(), 1, nToWrite, m_fp)) != nToWrite) {
         rc = ReFileSystem::EC_WRITE;
         m_logger->logv(LOG_ERROR, LOC_WRITE_1, "cannnot read %s (%d): %d/%d",
                        I18N::s2b(m_fullName).constData(), errno, nWritten, nToWrite);
      }
   }
   return rc;
}


/**
 * Constructor.
 *
 * @param metaData	metadata of the file
 * @param fullName	filename with path
 * @param logger	the logger
 */
ReLeafFile::ReLeafFile(const ReFileMetaData& metaData, const QString& fullName,
                       ReLogger* logger) :
   m_fullName(fullName),
   m_logger(logger),
   m_meta(metaData) {
}

/**
 * Destructor.
 */
ReLeafFile::~ReLeafFile() {
}


