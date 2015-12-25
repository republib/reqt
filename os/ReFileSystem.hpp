/*
 * ReFileSystem.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef OS_REFILESYSTEM_HPP_
#define OS_REFILESYSTEM_HPP_

class ReOSPermissions {
public:
   ReOSPermissions();
   ReOSPermissions(const ReOSPermissions& source);
   ReOSPermissions& operator =(const ReOSPermissions& source);
public:
   int m_user;
   int m_group;
   mode_t m_fileMode;
   mode_t m_dirMode;
};

class ReFileMetaData {
public:
   ReFileMetaData();
   ReFileMetaData(const QString& node, const QDateTime& modified,
                  const QDateTime& created, int owner = -1, int group = -1,
                  mode_t mode = (mode_t) - 1, int64_t size = 0, int32_t id = 0);
   virtual ~ReFileMetaData();
   ReFileMetaData(const ReFileMetaData& source);
   ReFileMetaData& operator =(const ReFileMetaData& source);
public:
   QString m_node;
   QDateTime m_modified;
   QDateTime m_created;
   int64_t m_size;
   int16_t m_owner;
   int16_t m_group;
   // unique inside the directory:
   int32_t m_id;
   mode_t m_mode;
};
typedef QList<ReFileMetaData> ReFileMetaDataList;

class ReLeafFile;
/**
 * Base class of file systems.
 *
 * A file system is a hierarchical set of directories with exact one root
 * (dirctory). Each directory can contain files and directories.
 */
class ReFileSystem {
public:
   enum ListOptions {
      LO_UNDEF = 0,
      LO_FILES = 1,
      LO_DIRS = 2,
      LO_ALL = 3,
      LO_NAME_FILTER_FOR_DIRS = 4,
      LO_ONLY_DIRS_WITH_NAMEFILTER = 6,
      LO_ALL_AND_NAME_FILTER_FOR_DIRS = 7,
   };

   enum ErrorCode {
      EC_SUCCESS,
      EC_PATH_NOT_FOUND,
      EC_NOT_ACCESSIBLE,
      EC_NOT_READABLE,
      EC_READ,
      EC_FS_READ_ONLY,
      EC_NOT_WRITEABLE,
      EC_WRITE,
      EC_POSITION,
      EC_FILE_ALREADY_EXISTS,
      EC_NOT_EXISTS,
      EC_RENAME,
      EC_HEADER_LENGTH,
      EC_MARKER,
      EC_DIR_ALREADY_EXISTS,
      EC_REMOTE_MKDIR,
      EC_CANNOT_OPEN,
      EC_INVALID_STATE,
      EC_ALREADY_EXISTS,
   };
   enum VerboseLevel {
      V_SILENT,
      V_IMPORTANT,
      V_VERBOSE,
      V_DEBUG
   };

public:
   ReFileSystem(const QString& name, ReLogger* logger);
   virtual ~ReFileSystem();
public:
   /** Creates a new file in the current directory.
    * @param node				the name of the new file
    * @param inDirectoryOnly	<code>false</code>: the file is made physically<br>
    *							otherwise: the file exists only in the metadata
    *							of the directory
    * @param metadata			OUT: NULL or the metadata of the new file
    * @return					EC_SUCCESS or the error code
    */
   virtual ErrorCode createFile(const QString& node, bool inDirectoryOnly,
                                ReFileMetaData* metadata = NULL) = 0;
   /**
    * Returns a filesystem dependent instance of a file.
    *
    * The file must exist in the current directory.
    * The caller must free the instance.
    *
    * @param meta	the metadata of the file
    * @return		NULL: no file found<br>
    *				otherwise: the file
    */
   virtual ReLeafFile* buildFile(const ReFileMetaData& meta) = 0;
   /** Returns the canonical form of a given path.
    * @param path	path to convert
    * @return		all nodes of the parts which are links are replaced by its
    *				link targets
    */
   virtual QString canonicalPathOf(const QString& path) = 0;
   /** Returns the name of the current directory.
    * @return	the name of the current directory
    */
   virtual const QString& directory() const;
   /** Search a file in the current directory given by name.
    * @param node		the filename without path
    * @param metaData	OUT: the info of the found file. May be NULL
    * @return			<code>true</code>: the file was found
    */
   virtual bool exists(const QString& node, ReFileMetaData* metaData = NULL)
   const = 0;
   /** Fills a list with the items of the current directory.
    * @param matcher	the matching processor
    * @param list		OUT: the found files
    * @param options	a set (bitmap) of options, e.g. LO_FILES | LO_DIRS
    * @return			the count of the found entries (<code>list.size()</code>)
    */
   virtual int listInfos(const ReIncludeExcludeMatcher& matcher,
                         ReFileMetaDataList& list, ListOptions options
                         = ListOptions(LO_FILES | LO_DIRS )) = 0;
   /** Creates a directory.
    * @param node	the name without path (in the current directory)
    * @return		EC_SUCCESS or error code
    */
   virtual ErrorCode makeDir(const QString& node) = 0;
   /** Removes a file or directory.
    * @param node	the properties ot the node (in the current directory)
    * @return		EC_SUCCESS or error code
    */
   virtual ErrorCode remove(const ReFileMetaData& node) = 0;
   /** Sets the current directory.
    * @param path	relative or absolute path. If absolute it must be part of the
    *				base path
    * @return		EC_SUCCESS or error code
    */
   virtual ErrorCode setDirectory(const QString& path) = 0;
   /** Sets the properties of a file in the current directory.
    * @param source	the properties to copy
    * @param target	the properties of the file to change
    * @param force		<code>true</code>: try to change rights to enable
    *					other changes<br>
    *					<code>false</code>: current rights will be respected
    * @return			EC_SUCCESS or error code
    */
   virtual ErrorCode setProperties(const ReFileMetaData& source,
                                   ReFileMetaData& target, bool force) = 0;
public:
   /** Returns a filesystem dependent instance of a file.
    *
    * The file must exist in the current directory.
    * The caller must free the instance.
    *
    * @param node	the filename without path
    * @return		NULL: no file found<br>
    *				otherwise: the file
    */
   ReLeafFile* buildFile(const QString& node) {
      ReFileMetaData meta;
      return exists(node, &meta) ? buildFile(meta) : NULL;
   }
   virtual QString errorMessage(ErrorCode rc);
   virtual ErrorCode copy(ReFileMetaData& source, ReFileSystem& sourceFS,
                          QString targetNode = ReQStringUtils::m_empty);
   virtual ErrorCode move(ReFileMetaData& source, ReFileSystem& sourceFS,
                          QString targetNode = ReQStringUtils::m_empty);
public:
   int blocksize() const;
   bool findByUrl(const QString& url, ReFileMetaData& metaData);
   bool first(const QString& pattern, ReFileMetaData& file,
              ListOptions options = LO_ALL_AND_NAME_FILTER_FOR_DIRS);
   /** Returns the full name (with path).
    * @param node	the name without path
    * @return		the full filename
    */
   inline QString fullName(const QString& node) const {
      return m_directory + node;
   }
   /** Returns the full name (with path) as UTF-8 string.
    * @param node	the name without path
    * @return		the full filename
    */
   QByteArray fullNameAsUTF8(const QString& node) const {
      QString full;
      full.reserve(m_directory.length() + node.length() + 32);
      return full.append(m_directory).append(node).toUtf8();
   }
   int nodesByPrefix(QString& prefix, ListOptions options, QStringList& list);
   ReOSPermissions osPermissions() const;
   bool sameCurrentDirectory(ReFileSystem& fileSystem) const;
   void setBlocksize(int blocksize);
   void setOsPermissions(const ReOSPermissions& osPermissions);
   void setWriteable(bool writeable);
   void synchronize(ReIncludeExcludeMatcher& fileMatcher,
                    ReIncludeExcludeMatcher& dirMatcher,
                    VerboseLevel verboseLevel,
                    ReFileSystem& source);
   bool writeable() const;
public:
   static ReFileSystem* buildFromUrl(const QString& url);
protected:
   QString m_name;
#ifdef __linux__
   int m_uid;
   int m_gid;
#endif
   // ending with OS_SEPARATOR:
   QString m_directory;
   // All links are replaced by its targets:
   QString m_canonicalDirectory;
   bool m_writeable;
   ReLogger* m_logger;
   QByteArray m_buffer;
   int m_blocksize;
   QDateTime m_undefinedTime;
   ReOSPermissions m_osPermissions;
};

/**
 * An abstract base class for leafs of the tree spanned by a filesystem.
 *
 * A leaf file could not be a directory.
 */
class ReLeafFile : public ReFileMetaData {
public:
   ReLeafFile(const ReFileMetaData& metaData, const QString& fullName,
              ReLogger* logger);
   virtual ~ReLeafFile();
public:
   /** Opens a file for reading or writing.
    * @param writeable	<code>true</code>: open for writing
    * @return			EC_SUCCESS: success<br>
    *					otherwise: the error code
    */
   virtual ReFileSystem::ErrorCode open(bool writeable) = 0;
   /** Frees the resources occupied by <code>open()</code>.
    * @return			EC_SUCCESS: success<br>
    *					otherwise: the error code
    */
   virtual ReFileSystem::ErrorCode close() = 0;
   /** Reads data from the current position into a buffer.
    * @param maxSize	number of bytes to read
    * @param buffer	OUT: content of the file
    * @return			EC_SUCCESS or error code
    */
   virtual ReFileSystem::ErrorCode read(int maxSize, QByteArray& buffer) = 0;
   /** Writes a buffer to a file at the current position.
    * @param buffer	content to write
    * @return			EC_SUCCESS or error code
    */
   virtual ReFileSystem::ErrorCode write(const QByteArray& buffer) = 0;
protected:
   QString m_fullName;
   ReLogger* m_logger;
   ReFileMetaData m_meta;
};


/**
 * An abstract base class for leafs of the tree spanned by a filesystem.
 *
 * A leaf file could not be a directory.
 */
class ReLocalLeafFile : public ReLeafFile {
public:
   ReLocalLeafFile(const ReFileMetaData& metaData, const QString& fullName,
                   ReLogger* logger);
   virtual ~ReLocalLeafFile();
public:
   virtual ReFileSystem::ErrorCode open(bool writeable);
   virtual ReFileSystem::ErrorCode close();
   virtual ReFileSystem::ErrorCode read(int maxSize, QByteArray& buffer);
   virtual ReFileSystem::ErrorCode write(const QByteArray& buffer);
protected:
   FILE* m_fp;
};

class ReLocalFileSystem: public ReFileSystem {
public:
   ReLocalFileSystem(const QString& basePath, ReLogger* logger);
   virtual ~ReLocalFileSystem();
public:
   const QString& basePath() const;
   ErrorCode setDirectory(const QString& path);

public:
   /** Returns a file instance of the local filesystem.
    * @param meta	the metadata of the file
    * @return		NULL: not found<br>
    *				otherwise: the file
    */
   virtual ReLeafFile* buildFile(const ReFileMetaData& meta) {
      return new ReLocalLeafFile(meta, fullName(meta.m_node), m_logger);
   }
   /** Returns the canonical form of a given path.
    * @param path	path to convert
    * @return		all nodes of the parts which are links are replaced by its
    *				link targets
    */
   virtual QString canonicalPathOf(const QString& path);
   virtual ErrorCode createFile(const QString& node, bool inDirectoryOnly,
                                ReFileMetaData* metadata = NULL);
   virtual bool exists(const QString& node, ReFileMetaData* metaData = NULL) const;
   virtual int listInfos(const ReIncludeExcludeMatcher& matcher,
                         ReFileMetaDataList& list, ListOptions options);
   virtual ErrorCode makeDir(const QString& node);
   virtual ErrorCode remove(const ReFileMetaData& node);
   virtual ErrorCode setProperties(const ReFileMetaData& source,
                                   ReFileMetaData& target, bool force = false);
protected:
   QString m_basePath;
   QDir m_dir;


};



#endif /* OS_REFILESYSTEM_HPP_ */
