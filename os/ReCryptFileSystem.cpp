/*
 * ReCryptFileSystem.cpp
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
//#define WITH_TRACE
#include "base/retrace.hpp"
/**
 * @file
 *
 * Format of a file (on the hosted FS):
 * <pre> random (8 byte)
 * checksum (from offset 16) (8 byte, encrypted with resetted random)
 * marker (2 byte, encrypted with resetted random)
 * flags (2 byte) see ReFileHeaderOptions
 * dynamic filelength (4 byte)
 * encrypted file content
 * checksum of the original file content
 * </pre>
 * <b>Calculation of the  checksum:</b><br>
 * Algorithm: see <code>ReHmHash64</code>
 * factor=0x7b644ac5d1187d25, increment=0x6b85115d6064365b
 *
 * Checksum of the unencrypted file content: straight forward
 *
 * Checksum of the encrypted data:<br>
 * <ul><li>Reset hash</li>
 * <li>Encrypt data and add it to the hash</li>
 * <li>Encrypt the checksum of the unencrypted data and add it to the hash</li<
 * <li>Store the data length (without checksum) to the header</li<
 * <li>Add the header from offset 16 to the hash</li>
 * <li>Add the checksum in the header at offset 8</li>
 * </ul>
 */
enum {
   LOC_ADD_ENTRY_1 = LOC_FIRST_OF(LOC_CRYPTFILESYSTEM), // 12301
   LOC_REMOVE_ENTRY_1,		// 12302
   LOC_READ_META_FILE_1,	// 12303
   LOC_READ_META_FILE_2,	// 12304
   LOC_READ_META_FILE_3,	// 12305
   LOC_WRITE_META_1,		// 12306
   LOC_WRITE_META_2,		// 12307
   LOC_MAKE_DIR_1,			// 12308
   LOC_FILE_OPEN_1,		// 12309
   LOC_FILE_WRITE_1,		// 12310
   LOC_CREATE_FILE_1,		// 12311
};

const int ReCryptFileSystem::NODE_LENGHT = 44;
const int ReCryptFileSystem::MARKER_LENGHT = 2;
const int ReCryptFileSystem::CHECKSUM_LENGHT = 16;
const int ReCryptFileSystem::HEADER_LENGTH = sizeof(int64_t)
      + ReCryptFileSystem::NODE_LENGHT
      + ReCryptFileSystem::MARKER_LENGHT;
const QString ReCryptFileSystem::NODE_META_DEVICE = ".0";
const QString ReCryptFileSystem::NODE_META_DIR = ".1";
const int ReCryptDirectory::MARKER_LENGTH = 4;
const int ReCryptDirectory::META_INFO_LENGTH = sizeof (ReCryptDirectory::MetaInfo_t);
const int ReCryptDirectory::META_DIR_HEADER_LENGTH = sizeof(int64_t)
      + ReCryptDirectory::MARKER_LENGTH + ReCryptDirectory::META_INFO_LENGTH;
// space for the struct and the node:
const int ReCryptDirectory::MAX_ENTRY_SIZE = sizeof(ReCryptDirectory::FileEntry_t)
      + 512;
const int ReCryptDirectory::FILE_MARKER_LENGTH = 2;
const int ReCryptDirectory::FILE_FLAGS_LENGTH = 2;
const int ReCryptDirectory::FILE_LENGTH_LENGTH = 4;
const int ReCryptDirectory::FILE_HEADER_LENGTH = 2 * sizeof(int64_t)
      + FILE_MARKER_LENGTH + FILE_FLAGS_LENGTH + FILE_LENGTH_LENGTH;
const int ReCryptDirectory::FILE_CHECKSUM_LENGTH = sizeof(int64_t);

/**
 * Constructor.
 *
 * @param hostFileSystem	the filesystem which does the storage (with
 *							encrypted names and content
 * @param contentRandom		a pseudo random generator for content
 * @param logger			the logger
 */
ReCryptFileSystem::ReCryptFileSystem(ReFileSystem& hostFileSystem,
                                     ReRandomizer& contentRandom, ReLogger* logger) :
   ReFileSystem("cryptfs", logger),
   ReCryptDirectory(contentRandom, this, logger),
   m_host(hostFileSystem) {
}

/**
 * Destructor.
 */
ReCryptFileSystem::~ReCryptFileSystem() {
}

/**
 * Returns a filesystem dependent instance of a file.
 *
 * The file must exist in the current directory.
 * The caller must free the instance.
 *
 * @param metadata	the metadata of the file
 * @return			NULL: no file found<br>
 *					otherwise: the file
 */
ReLeafFile* ReCryptFileSystem::buildFile(const ReFileMetaData& metadata) {
   return new ReCryptLeafFile(metadata, fullName(metadata.m_node),
                              *this, m_logger2);
}

QString ReCryptFileSystem::canonicalPathOf(const QString& path) {
   return ReFileUtils::cleanPath(path);
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
ReFileSystem::ErrorCode ReCryptFileSystem::createFile(const QString& node,
      bool inDirectoryOnly, ReFileMetaData* metadata) {
   ErrorCode rc = EC_SUCCESS;
   if (exists(node, NULL))
      rc = EC_ALREADY_EXISTS;
   else {
      QDateTime now = QDateTime::currentDateTime();
      int id = ++m_maxFileId;
      ReFileMetaData meta = ReFileMetaData(node, now, now,
                                           m_osPermissions.m_user, m_osPermissions.m_group,
                                           m_osPermissions.m_dirMode, 0, id);
      m_list.append(meta);
      if (metadata != NULL) {
         *metadata = meta;
      }
      if (! inDirectoryOnly) {
         QString hostName = buildHostedNode(id);
         QByteArray fullName = fullNameAsUTF8(hostName);
         FILE* fp = fopen(fullName.constData(), "w");
         if (fp == NULL) {
            rc = EC_WRITE;
            m_logger2->logv(LOG_ERROR, LOC_CREATE_FILE_1, "cannot open (%d): %s",
                            errno, fullName.constData());
         } else
            fclose(fp);
      }
   }
   return rc;
}

/**
 * Tests whether a file exists in the current directory.
 *
 * @param node		filename without path
 * @param metaInfo	OUT: info of the found file. May be NULL
 * @return
 */
bool ReCryptFileSystem::exists(const QString& node, ReFileMetaData* metaInfo) const {
   const ReFileMetaData* info = find(node);
   bool rc = info != NULL;
   if (rc && metaInfo != NULL)
      *metaInfo = *info;
   return rc;
}

/**
 * Fills a list with the items of the current directory.
 *
 * @param matcher	the matching processor for selecting the files
 * @param list		OUT: the list of the found files
 * @param options	options to define what kind of files should be found
 * @return			the count of the found entries (<code>list.size()</code>)
 */
int ReCryptFileSystem::listInfos(const ReIncludeExcludeMatcher& matcher,
                                 ReFileMetaDataList& list, ListOptions options) {
   ReFileMetaDataList::const_iterator it;
   bool withDirs = (options & LO_DIRS) != 0;
   bool withFiles = (options & LO_FILES) != 0;
   if (! withDirs && ! withFiles)
      withDirs = withFiles = true;
   bool filterDirs = (options & LO_NAME_FILTER_FOR_DIRS);
   list.clear();
   for (it = m_list.cbegin(); it != m_list.cend(); ++it) {
      bool isDir = S_ISDIR(it->m_mode);
      if ( (isDir && ! withDirs) || (! isDir && ! withFiles))
         continue;
      if ((! filterDirs && isDir) || matcher.matches(it->m_node))
         list.append(*it);
   }
   return list.length();
}

/**
 * Creates a directory.
 *
 * @param node	the name without path (in the current directory)
 * @return		EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptFileSystem::makeDir(const QString& node) {
   ReFileMetaData file;
   ErrorCode rc = EC_SUCCESS;
   if (exists(node, &file)) {
      if ((S_ISDIR(file.m_mode)))
         rc = EC_DIR_ALREADY_EXISTS;
      else
         rc = EC_FILE_ALREADY_EXISTS;
   } else {
      QDateTime now = QDateTime::currentDateTime();
      int id = ++m_maxFileId;
      QString hostName = buildHostedNode(id);
      ErrorCode rc2 = m_host.makeDir(hostName);
      if (rc2 != EC_SUCCESS) {
         m_logger2->logv(LOG_ERROR, LOC_MAKE_DIR_1, "cannot create hosted directory %s: %s",
                         I18N::s2b(hostName).constData(),
                         I18N::s2b(errorMessage(rc2)).constData());
         rc = EC_REMOTE_MKDIR;
      } else {
         m_list.append(ReFileMetaData(node, now, now, m_osPermissions.m_user,
                                      m_osPermissions.m_group, m_osPermissions.m_dirMode, 0,
                                      id));
         m_changed = true;
      }
   }
   return rc;
}

/**
 * Reads a part of a file into a buffer.
 *
 * @param source	the file to read (inside the current directory)
 * @param offset	first position to read
 * @param size		number of bytes to read
 * @param buffer	OUT: content of the file
 * @return			EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptFileSystem::read(const ReFileMetaData& source,
      int64_t offset, int size, QByteArray& buffer) {
   return EC_SUCCESS;
}

/** Removes a file or directory.
 * @param node	the properties ot the node (in the current directory)
 * @return		EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptFileSystem::remove(const ReFileMetaData& node) {
   return EC_SUCCESS;
}

/**
 * Sets the current directory.
 *
 * @param path	relative or absolute path. If absolute it must be part of the
 *				base path
 * @return		EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptFileSystem::setDirectory(const QString& path) {
   return EC_SUCCESS;
}

/**
 * Sets the properties of a file in the current directory.
 *
 * @param source	the properties to copy
 * @param target	the properties of the file to change
 * @param force		<code>true</code>: try to change rights to enable
 *					other changes<br>
 *					<code>false</code>: current rights will be respected
 * @return			EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptFileSystem::setProperties(const ReFileMetaData& source,
      ReFileMetaData& target, bool force) {
   return EC_SUCCESS;
}

/**
 * Writes a buffer to a file.
 *
 * @param target	the file to write (without path, inside the current directory)
 * @param offset	first position to write
 * @param buffer	content to write
 * @return			EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptFileSystem::write(const QString& target,
      int64_t offset, const QByteArray& buffer) {
   m_contentRandom.reset();
   QByteArray m_header;
   return writeFileBlock(target, offset, buffer);
}

/**
 * Adds a file to the current directory.
 *
 * @param node	filename without path
 */
void ReCryptFileSystem::addFile(const QString& node) {
   QDateTime now = QDateTime::currentDateTime();
   ReFileMetaData entry(node, now, now, m_osPermissions.m_user,
                        m_osPermissions.m_group, m_osPermissions.m_fileMode,
                        0, ++m_maxFileId);
   m_list.append(entry);
}

/**
 * Constructor
 *
 * @param contentRandom	the pseudo random generator uses for encryption
 * @param parent		the hosting filesystem
 * @param logger		the logger
 */
ReCryptDirectory::ReCryptDirectory(ReRandomizer& contentRandom,
                                   ReCryptFileSystem* parent, ReLogger* logger) :
   ReByteScrambler(contentRandom, logger),
   m_list(),
   m_parentFS(parent),
   m_changed(false),
   m_logger2(logger),
   m_currentNode(),
   m_fileBuffer(),
   m_entryBuffer(),
   m_smallBuffer(),
   m_blockSize(1024 * 1024),
   m_maxFileId(0) {
   m_fileBuffer.reserve(m_blockSize);
   m_entryBuffer.reserve(m_blockSize + MAX_ENTRY_SIZE + 10);
}

/**
 * Destructor.
 */
ReCryptDirectory::~ReCryptDirectory() {
   if (m_changed)
      writeMetaFile();
}

/**
 * Adds an file entry to the directory.
 *
 * @param entry	the meta data of the file to add
 * @return		<code>true</code>: success
 */
bool ReCryptDirectory::addEntry(ReFileMetaData& entry) {
   bool rc = true;
   if (find(entry.m_node)) {
      rc = ! m_logger->logv(LOG_ERROR, LOC_ADD_ENTRY_1, "file exists yet: %s",
                            entry.m_node.constData());
   } else {
      m_list.append(entry);
      m_changed = true;
   }
   return rc;
}

/**
 * Makes a node name from an id.
 *
 * @param id	a unique number of the file
 * @return		a unique node name derived from the id
 */
QString ReCryptDirectory::buildHostedNode(int id) const {
   QByteArray rc;
   static const int BASE = 'Z' - 'A' + 1;
   while(id > 0) {
      rc.insert(0, 'a' + id % BASE);
      id /= BASE;
   }
   return QString(rc);
}

/**
 * Makes an id from an node name in the hosted filesystem.
 *
 * @param hostedNode	the node to convert
 * @return				-1: invalid node name<br>
 *						otherwise: the id of the node
 */
int ReCryptDirectory::buildId(const QString& hostedNode) const {
   int id = 0;
   static const int BASE = 'Z' - 'A' + 1;
   for (int ix = 0; id >= 0 && ix < hostedNode.length(); ix++) {
      int digit = hostedNode.at(ix).unicode() - QChar('a').unicode();
      if (digit < 0 || digit >= BASE)
         id = -1;
      else
         id = id*BASE + digit;
   }
   return id;
}

/**
 * Returns the buffer for file data blocks.
 *
 * @return	the file buffer
 */
QByteArray& ReCryptDirectory::fileBuffer() {
   return m_fileBuffer;
}

/**
 * Search an file entry by name.
 *
 * @param node	the filename without path
 * @return		NULL: not found<br>
 *				otherwise: the found file
 *
 */
const ReFileMetaData* ReCryptDirectory::find(const QString& node) const {
   const ReFileMetaData* rc = NULL;
   ReFileMetaDataList::const_iterator it;
   for (it = m_list.cbegin(); it != m_list.cend(); ++it) {
      if (it->m_node == node) {
         rc = &*it;
      }
   }
   return rc;
}

/**
 * Returns the logger.
 *
 * @return	the logger
 */
ReLogger* ReCryptDirectory::logger() const {
   return m_logger2;
}

/**
 * Returns the parent filesystem.
 *
 * @return the parent filesystem
 */
ReCryptFileSystem* ReCryptDirectory::parentFS() const {
   return m_parentFS;
}

/**
 * Removes an entry given by the name.
 *
 * @param node	the filename without path
 * @return
 */
bool ReCryptDirectory::removeEntry(const QString& node) {
   bool rc = true;
   const ReFileMetaData* entry = find(node);
   if (entry == NULL)
      rc = ! m_logger->logv(LOG_ERROR, LOC_REMOVE_ENTRY_1, "cannot remove file %s: not found",
                            I18N::s2b(node).constData());
   else {
      ReFileMetaData& entry2 = *(ReFileMetaData*) entry;
      //@ToDo:
      //m_list.removeOne(entry2);
      assert(false);
      ReFileSystem& host = m_parentFS->host();
      ReFileMetaData hostedFile;
      QString hostedNode = buildHostedNode(entry->m_id);
      if (host.first(hostedNode, hostedFile))
         host.remove(hostedFile);
      m_changed = true;
   }
   return rc;
}

/**
 * Reads the file containing the metadata of the files.
 *
 * @return	<code>true</code>: success
 */
bool ReCryptDirectory::readMetaFile() {
   bool rc = true;
   m_list.clear();
   QString fnMetaFile = m_parentFS->host().directory()
                        + ReCryptFileSystem::NODE_META_DIR;
   FILE* fp = fopen(I18N::s2b(fnMetaFile).constData(), "rb");
   m_maxFileId = 0;
   if (fp != NULL) {
      QByteArray header;
      header.resize(META_DIR_HEADER_LENGTH);
      int nRead = fread(header.data(), 1, META_DIR_HEADER_LENGTH, fp);
      if (nRead != META_DIR_HEADER_LENGTH) {
         rc = ! m_logger->logv(LOG_ERROR, LOC_READ_META_FILE_1,
                               "header of %s too small: %d/%d",
                               I18N::s2b(fnMetaFile).constData(), nRead, META_DIR_HEADER_LENGTH);
      } else {
         QByteArray info;
         rc = initFromHeader(0, MARKER_LENGTH, META_INFO_LENGTH, 0, &header, info);
         if (rc) {
            const MetaInfo_t* meta = reinterpret_cast<const MetaInfo_t*>(info.constData());
            TRACE2("count: %d size: %d\n", meta->m_countFiles, meta->m_size);
            if (meta->m_countFiles > 0) {
               m_fileBuffer.resize(m_blockSize);
               m_entryBuffer.resize(0);
               int sumLength = 0;
               randomReset();
               while ( (nRead = fread(m_fileBuffer.data(),
                                      1, m_blockSize, fp)) > 0) {
                  sumLength += nRead;
                  if (nRead < m_blockSize)
                     m_fileBuffer.resize(nRead);
                  m_contentRandom.codec(m_fileBuffer);
                  m_entryBuffer.append(m_fileBuffer);
                  splitBlock(sumLength >= meta->m_size,
                             m_entryBuffer);
               }
               if (sumLength != meta->m_size) {
                  m_logger->logv(LOG_ERROR, LOC_READ_META_FILE_2,
                                 "file %s too small: %d/%d",
                                 I18N::s2b(fnMetaFile).constData(),
                                 sumLength, meta->m_size);
               }
            }
         }
      }
   }
   return rc;
}

/**
 * Writes the metadata of the directory into a file.
 *
 * @return	<code>true</code>: success
 */
bool ReCryptDirectory::writeMetaFile() {
   TRACE("writeMetaFile:\n");
   bool rc = true;
   QByteArray meta;
   meta.resize(sizeof(MetaInfo_t));
   MetaInfo_t* meta2 = reinterpret_cast<MetaInfo_t*>(meta.data());
   meta2->m_countFiles = m_list.length();
   meta2->m_size = meta2->m_countFiles * sizeof(FileEntry_t);
   ReFileMetaDataList::const_iterator it;
   for (it = m_list.cbegin(); it != m_list.cend(); ++it) {
      int length = I18N::s2b(it->m_node).length();
      meta2->m_size += length + (length < 256 ? 0 : 1);
   }
   TRACE2("count: %d size: %d\n", meta2->m_countFiles, meta2->m_size);
   initHeader(0, MARKER_LENGTH, META_INFO_LENGTH, 0, meta);
   QByteArray node;
   for (it = m_list.cbegin(); it != m_list.cend(); ++it) {
      node = I18N::s2b(it->m_node);
      int length = node.length();
      meta2->m_size += length + (length < 256 ? 0 : 1);
   }
   QString fnMetaFile = m_parentFS->host().directory()
                        + ReCryptFileSystem::NODE_META_DIR;
   FILE* fp = fopen(I18N::s2b(fnMetaFile).constData(), "wb");
   if (fp == NULL) {
      m_logger->logv(LOG_ERROR, LOC_WRITE_META_1, "cannot write (%d): %s",
                     errno, fnMetaFile.constData());
   } else {
      m_fileBuffer.resize(0);
      m_fileBuffer.append(m_header);
      int offset = m_header.length();
      int ixList = 0;
      randomReset();
      while (ixList < m_list.length()) {
         const ReFileMetaData& file = m_list.at(ixList++);
         FileEntry_t trg;
         trg.m_created = file.m_created.toMSecsSinceEpoch();
         trg.m_modified = file.m_modified.toMSecsSinceEpoch();
         trg.m_owner = file.m_owner;
         trg.m_group = file.m_group;
         trg.m_size = file.m_size;
         trg.m_mode = file.m_mode;
         trg.m_id = file.m_id;
         node = I18N::s2b(file.m_node);
         int length = node.length();
         trg.m_nodeLength = length < 256 ? length : 0;
         m_fileBuffer.append(reinterpret_cast<const char*>(&trg), sizeof trg);
         TRACE2("%2d: %s", ixList, hexBytes(&trg, sizeof trg).constData());
         m_fileBuffer.append(node);
         TRACE2("  length: %d %s\n", length, node.constData());
         if (length >= 256)
            m_fileBuffer.append('\0');
         int blockLength = m_fileBuffer.length();
         bool lastBlock = ixList >= m_list.length();
         if (lastBlock || blockLength  >= m_blockSize - 512) {
            if (! lastBlock && blockLength % sizeof(int64_t) != 0) {
               int newLength = blockLength - blockLength % sizeof(int64_t);
               m_smallBuffer = m_fileBuffer.mid(newLength);
               m_fileBuffer.resize(newLength);
            }
            m_contentRandom.codec(m_fileBuffer, m_fileBuffer, offset);
            offset = 0;
            int nWritten = fwrite(m_fileBuffer.constData(), 1,
                                  m_fileBuffer.length(), fp);
            if (nWritten != m_fileBuffer.length()) {
               rc = ! m_logger->logv(LOG_ERROR, LOC_WRITE_META_2,
                                     "write error (%d): %s [%d/%d]", errno,
                                     nWritten, m_fileBuffer.length());
            }
            m_fileBuffer.resize(0);
            if (! lastBlock && blockLength % sizeof(int64_t) != 0)
               m_fileBuffer.append(m_smallBuffer);
         }
      }
      fclose(fp);
   }
   return rc;
}

/**
 * Splits a block with many metadata entries into single entries.
 *
 * @param isLast	<code>true</code>the given block is the last in the file
 * @param block		the data to split.<br>
 *					Format: FileEntry_t_1 node1 File_entry2 node2 ...<br>
 *					Precondition: the block starts with a FileEntry_t<br>
 *					Postcondition: the block is empty or starts with the
 *					first FileEntry_t which is not processed. All processed
 *					entries are deleted from the buffer. block.length() < MAX_ENTRY_SIZE
 */
void ReCryptDirectory::splitBlock(bool isLast, QByteArray& block) {
   ReFileMetaData file;
   TRACE("splitBlock:\n");
   IF_TRACE(int ix = 0);
   const char* srcPtr = reinterpret_cast<const char*>(block.constData());
   const char* endPtr = srcPtr + block.length() - (isLast ? 0 : MAX_ENTRY_SIZE);
   while (srcPtr < endPtr) {
      const FileEntry_t* src = reinterpret_cast<const FileEntry_t*>(srcPtr);
      TRACE2("%2d: %s", ++ix, hexBytes(&src, sizeof src).constData());
      file.m_created.setMSecsSinceEpoch(src->m_created);
      file.m_modified.setMSecsSinceEpoch(src->m_modified);
      file.m_owner = src->m_owner;
      file.m_group = src->m_group;
      file.m_size = src->m_size;
      file.m_mode = src->m_mode;
      file.m_id = src->m_id;
      if (file.m_id > m_maxFileId)
         m_maxFileId = file.m_id;
      srcPtr += sizeof(FileEntry_t);
      int nodeLength = src->m_nodeLength != 0 ? src->m_nodeLength : strlen(srcPtr);
      QByteArray node(srcPtr, nodeLength);
      TRACE2(" Length: %d %s\n", nodeLength, node.constData());
      file.m_node = node;
      m_list.append(file);
      srcPtr += nodeLength + (src->m_nodeLength != 0 ? 0 : 1);
   }
   block.remove(0, srcPtr - block.constData());
   printf("List: %d Rest: %d\n", m_list.length(), block.length());
}

/**
 * Returns the block size.
 * @return	the block size
 */
int ReCryptDirectory::blockSize() const {
   return m_blockSize;
}
/**
 * Sets the block size.
 *
 * @param blockSize	the new block size
 */
void ReCryptDirectory::setBlockSize(int blockSize) {
   m_blockSize = blockSize;
}

/**
 * Gets the filename of an entry in the hosted filesystem.
 *
 * @param entry	the name was built for this file
 * @return		the full name of the hosted (encrypted) file
 */
const QString& ReCryptDirectory::hostedFilename(const ReFileMetaData& entry) {
   QString node = buildHostedNode(entry.m_id);
   m_currentNode = ReFileUtils::pathAppend(m_parentFS->directory(), node);
   return m_currentNode;
}

/**
 * Writes a buffer to a file.
 *
 * @param target	the file to write (without path, inside the current directory)
 * @param offset	first position to write
 * @param buffer	content to write
 * @return			EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptDirectory::writeFileBlock(const QString& target,
      int64_t offset, const QByteArray& buffer) {
   ReFileSystem::ErrorCode rc = ReFileSystem::EC_SUCCESS;
   if (offset == 0) {
      ReFileMetaData metaData;
      if (! m_parentFS->m_parentFS->exists(target, &metaData)) {
         QDateTime now = QDateTime::currentDateTime();
         metaData.m_node = target;
         metaData.m_modified = now;
         metaData.m_created = now;
         metaData.m_owner = m_parentFS->osPermissions().m_user;
         metaData.m_group = m_parentFS->osPermissions().m_group;
         metaData.m_mode = m_parentFS->osPermissions().m_fileMode;
         metaData.m_size = buffer.length() + ReCryptDirectory::FILE_HEADER_LENGTH
                           + ReCryptDirectory::FILE_CHECKSUM_LENGTH;
         metaData.m_id = ++m_maxFileId;
      }
   }
   return rc;
}

/**
 * Constructor.
 *
 * @param metadata		the file infos
 * @param fullName		the filename with path
 * @param directory		the assoziated directory
 * @param logger		the logger
 *
 */
ReCryptLeafFile::ReCryptLeafFile(const ReFileMetaData& metadata,
                                 const QString& fullName, ReCryptDirectory& directory, ReLogger* logger) :
   ReLeafFile(metadata, fullName, logger),
   m_fullHostedName(I18N::s2b(directory.parentFS()->host().directory()
                              + directory.parentFS()->buildHostedNode(metadata.m_id))),
   m_fileHeader(),
   m_dataSum(0x7b644ac5d1187d25L, 0x6b85115d6064365bL),
   m_sumOfEncrypted(0x7b644ac5d1187d25L, 0x6b85115d6064365bL),
   m_fp(NULL),
   m_directory(directory),
   m_dataSize(0) {
}

/**
 * Destructor.
 */
ReCryptLeafFile::~ReCryptLeafFile() {
   close();
}

/**
 * Initializes the file for reading/writing.
 *
 * @param writeable	<code>true</code>: the file can be written
 * @return			EC_SUCCESS: success<br>
 *					x
 */
ReFileSystem::ErrorCode ReCryptLeafFile::open(bool writeable) {
   ReFileSystem::ErrorCode rc = ReFileSystem::EC_SUCCESS;
   close();
   m_fp = fopen(m_fullHostedName, writeable? "wb" : "rb");
   if (m_fp == NULL) {
      m_directory.logger()->logv(LOG_ERROR, LOC_FILE_OPEN_1, "cannot open hosted file (%d): %s",
                                 errno, m_fullHostedName.constData());
      rc = writeable ? ReFileSystem::EC_NOT_WRITEABLE : ReFileSystem::EC_NOT_READABLE;
   }
   return rc;
}

/**
 * Writes a block to the hosted file.
 *
 * @param data	data to write
 * @return		EC_SUCCESS: success
 */
ReFileSystem::ErrorCode ReCryptLeafFile::writeBlock(const QByteArray& data) {
   ReFileSystem::ErrorCode rc = ReFileSystem::EC_SUCCESS;
   if (m_fp != NULL && fwrite(data.constData(), data.length(), 1, m_fp)
         != size_t(data.length())) {
      m_directory.logger()->logv(LOG_ERROR, LOC_FILE_WRITE_1,
                                 "cannot write (%d): %s", errno, m_fullHostedName.constData());
      rc = ReFileSystem::EC_NOT_WRITEABLE;
      fclose(m_fp);
      m_fp = NULL;
   }
   return rc;
}

/**
 * Writes a data block to the file.
 *
 * @param data
 * @return
 */
ReFileSystem::ErrorCode ReCryptLeafFile::write(const QByteArray& data) {
   ReFileSystem::ErrorCode rc = ReFileSystem::EC_SUCCESS;
   m_dataSum.updateBlock(data);
   QByteArray& target = m_directory.fileBuffer();
   int blockSize = m_directory.blockSize();
   if (m_dataSize > 0 && m_dataSize <= blockSize) {
      // write the header...
      if ( (rc = writeBlock(m_fileHeader)) == ReFileSystem::EC_SUCCESS)
         // and the first block...
         rc = writeBlock(target);
      target.clear();
   }
   m_dataSize += data.length();
   if (rc == ReFileSystem::EC_SUCCESS) {
      m_directory.contentRandom(false).codec(target, data);
      m_sumOfEncrypted.updateBlock(target);
   }
   return rc;
}

/**
 * Calculates the "dynamic length".
 *
 * The dynamic length is the length if it can be stored in a 32 bit integer.
 * If the length is greater it will be shifted to right until 32 bit is enough.
 *
 * @param length	the 64 bit length
 * @return			the dynamic length
 */
uint32_t ReCryptLeafFile::dynamicLength(int64_t length) {
   while (length > 0xFFFFffffL)
      length >>= 1;
   return uint64_t(length);
}

/**
 * Flush data and free resources.
 *
 * @return EC_SUCCESS: success
 */
ReFileSystem::ErrorCode ReCryptLeafFile::close() {
   ReFileSystem::ErrorCode rc = ReFileSystem::EC_SUCCESS;
   if (m_fp != NULL) {
      QByteArray& target = m_directory.fileBuffer();
      int blockSize = m_directory.blockSize();
      bool doFlush = m_dataSize > 0 && m_dataSize <= blockSize;
      m_meta.m_size = m_dataSize;
      QByteArray checkSum = m_dataSum.digest();
      // Add the data checksum at the end of the data block:
      target.append(reinterpret_cast<const char*>(&checkSum), sizeof checkSum);
      // Update the encrypted checksum:
      m_sumOfEncrypted.update(&checkSum, sizeof checkSum);
      // Update the protected part of the header info:
      uint32_t size = dynamicLength(m_dataSize);
      memcpy(m_fileHeader.data() + sizeof(ReRandomizer::seed_t)
             + ReCryptDirectory::FILE_FLAGS_LENGTH, &size, sizeof size);
      // Add the protected part of the header to the encrypted checksum:
      m_sumOfEncrypted.update(m_fileHeader.constData() + 2 * sizeof(int64_t),
                              m_fileHeader.length() - 2 * sizeof(int64_t));
      // update the unprotected part of the header:
      memcpy(m_fileHeader.data() + sizeof(ReRandomizer::seed_t), &checkSum,
             sizeof checkSum);
      if (doFlush)
         target.insert(0, m_fileHeader);
      rc = writeBlock(target);
      target.clear();
      if (m_fp != NULL && ! doFlush) {
         fseek(m_fp, 0, SEEK_SET);
         rc = writeBlock(m_fileHeader);
      }
      if (m_fp != NULL) {
         fclose(m_fp);
         m_fp = NULL;
      }
   }
   return rc;
}

/**
 * Reads data from the current position into a buffer.
 *
 * @param size		number of bytes to read
 * @param buffer	OUT: content of the file
 * @return			EC_SUCCESS or error code
 */
ReFileSystem::ErrorCode ReCryptLeafFile::read(int size, QByteArray& buffer) {
   ReFileSystem::ErrorCode rc = ReFileSystem::EC_SUCCESS;
   return rc;
}

