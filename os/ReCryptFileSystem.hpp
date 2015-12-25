/*
 * ReCryptFileSystem.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */


#ifndef RECRYPTFILESYSTEM_HPP
#define RECRYPTFILESYSTEM_HPP

class ReCryptFileSystem;
class ReCryptDirectory;

/**
 * Administrates an encrypted file for reading / writing.
 *
 * A leaf file could not be a directory.
 */
class ReCryptLeafFile : public ReLeafFile {
public:
   ReCryptLeafFile(const ReFileMetaData& metaData, const QString& fullName,
                   ReCryptDirectory& directory, ReLogger* logger);
   virtual ~ReCryptLeafFile();
public:
   virtual ReFileSystem::ErrorCode open(bool writeable);
   virtual ReFileSystem::ErrorCode close();
   virtual ReFileSystem::ErrorCode read(int size, QByteArray& buffer);
   virtual ReFileSystem::ErrorCode write(const QByteArray& buffer);
public:
   static uint32_t dynamicLength(int64_t length);
protected:
   ReFileSystem::ErrorCode writeBlock(const QByteArray& data);
private:
   QByteArray m_fullHostedName;
   QByteArray m_fileHeader;
   /// checksum of the unencrypted data
   ReHmHash64 m_dataSum;
   /// checksum of the encrypted data
   ReHmHash64 m_sumOfEncrypted;
   FILE* m_fp;
   ReCryptDirectory& m_directory;
   int64_t m_dataSize;
};

/**
 * Administrates an encrypted directory.
 */
class ReCryptDirectory : public ReByteScrambler {
public:
   typedef struct {
      int64_t m_size;
      //@ millisec from epoch
      int64_t m_modified;
      //@ millisec from epoch
      int64_t m_created;
      int32_t m_id;
      int16_t m_owner;
      int16_t m_group;
      mode_t m_mode;
      // 0: strlen(node) > 255
      uint8_t m_nodeLength;
   } FileEntry_t;
   typedef struct {
      int32_t m_countFiles;
      int32_t m_size;
   } MetaInfo_t;
public:
   ReCryptDirectory(ReRandomizer& contentRandom, ReCryptFileSystem* parent,
                    ReLogger* logger);
   ~ReCryptDirectory();
public:
   bool addEntry(ReFileMetaData& entry);
   int blockSize() const;
   QString buildHostedNode(int id) const;
   QByteArray& fileBuffer();
   const ReFileMetaData* find(const QString& node) const;
   ReLogger* logger() const;
   ReCryptFileSystem* parentFS() const;
   bool removeEntry(const QString& entry);
   bool readMetaFile();
   void setBlockSize(int blockSize);
   ReFileSystem::ErrorCode writeFileBlock(const QString& target, int64_t offset,
                                          const QByteArray& buffer);
   bool writeMetaFile();
protected:
   int buildId(const QString& hostedNode) const;
   const QString& hostedFilename(const ReFileMetaData& entry);
   void splitBlock(bool isLast, QByteArray& block);
public:
   static const int MARKER_LENGTH;
   static const int META_INFO_LENGTH;
   static const int META_DIR_HEADER_LENGTH;
   static const int MAX_ENTRY_SIZE;
   static const int FILE_MARKER_LENGTH;
   static const int FILE_FLAGS_LENGTH;
   static const int FILE_LENGTH_LENGTH;
   static const int FILE_HEADER_LENGTH;
   static const int FILE_CHECKSUM_LENGTH;

protected:
   ReFileMetaDataList m_list;
   ReCryptFileSystem* m_parentFS;
   bool m_changed;
   // to avoid ambigousity:
   ReLogger* m_logger2;
   QString m_currentNode;
   QByteArray m_fileBuffer;
   QByteArray m_entryBuffer;
   QByteArray m_smallBuffer;
   int m_blockSize;
   int m_maxFileId;
};

/**
 * A filesystem with encrypted filenames and file content.
 *
 * The storage is done with a 'host filesystem'.
 * The filenames used in the interface (parameters) are clear text.
 * The filenames of the base filesystem are encrypted.
 * If a file content is copied / moved to the base filesystem the content
 * will be encrypted. In the other direction the file content will be decrypted.
 */
class ReCryptFileSystem: public ReFileSystem,
   public ReCryptDirectory {
public:
   static const int NODE_LENGHT;
   static const int HEADER_LENGTH;
   static const int CHECKSUM_LENGHT;
   static const int MARKER_LENGHT;
   static const QString NODE_META_DIR;
   static const QString NODE_META_DEVICE;
public:
   ReCryptFileSystem(ReFileSystem& hostFileSystem,
                     ReRandomizer& contentRandom, ReLogger* logger);
   ~ReCryptFileSystem();
public:
   virtual ReLeafFile* buildFile(const ReFileMetaData& metadata);
   virtual QString canonicalPathOf(const QString& path);
   virtual ErrorCode createFile(const QString& node, bool inDirectoryOnly,
                                ReFileMetaData* metadata = NULL);
   virtual bool exists(const QString& node, ReFileMetaData* metaInfo) const;
   virtual int listInfos(const ReIncludeExcludeMatcher& matcher,
                         ReFileMetaDataList& list, ListOptions options = LO_ALL);
   virtual ErrorCode makeDir(const QString& node);
   virtual ErrorCode read(const ReFileMetaData& source, int64_t offset,
                          int size, QByteArray& buffer);
   virtual ErrorCode remove(const ReFileMetaData& node);
   virtual ErrorCode setDirectory(const QString& path);
   virtual ErrorCode setProperties(const ReFileMetaData& source, ReFileMetaData& target, bool force);
   virtual ErrorCode write(const QString& target, int64_t offset,
                           const QByteArray& buffer);
public:
   /** Returns the filesystem hosting the encrypted files.
    * @return	the hosting filesystem
    */
   ReFileSystem& host() const {
      return m_host;
   }
   void addFile(const QString& node);
protected:
   ReFileSystem& m_host;
};

#endif // RECRYPTFILESYSTEM_HPP
