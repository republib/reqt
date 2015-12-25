/*
 * ReTraverser.cpp
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
#if defined __WIN32__
#include "accctrl.h"
#include "aclapi.h"
#pragma comment(lib, "advapi32.lib")
#endif

enum {
   LC_RIGHTS_AS_STRING_1 = LOC_FIRST_OF(LOC_TRAVERSER), // 50401
   LC_RIGHTS_AS_STRING_2,	// 50402
   LC_RIGHTS_AS_STRING_3,	// 50403
   LC_GET_PRIVILEGE_1,		// 50404
   LC_GET_PRIVILEGE_2,		// 50405
   LC_GET_PRIVILEGE_3,		// 50406
   LC_GET_FILE_OWNER_1,	// 50407
   LC_GET_FILE_OWNER_2,	// 50408
};

int ReDirEntryFilter::m_serialId = buildSerialId(CLASSID_DIR_ENTRY_FILTER, 1);

/**
 * Constructor.
 */
ReDirStatus_t::ReDirStatus_t(ReLogger* logger) :
   m_path(),
   m_fullName(),
   m_passNo(0),
   m_logger(logger),
#ifdef __linux__
   m_handle(NULL),
   m_data(NULL)
//m_status;
#elif defined WIN32
   m_handle(INVALID_HANDLE_VALUE),
//m_data;
   m_getPrivilege(true)
#endif
{
#ifdef __linux__
   memset(&m_status, 0, sizeof m_status);
#elif defined WIN32
   memset(&m_data, 0, sizeof m_data);
#endif
}

/**
 * Returns the last access time.
 *
 * @return	the last access time
 */
const ReFileTime_t* ReDirStatus_t::accessed() {
#ifdef __linux__
   return &(getStatus()->st_atim);
#elif defined __WIN32__
   return &m_data.ftLastAccessTime;
#endif
}

/**
 * Returns the filesize.
 *
 * @return	the filesize
 */
ReFileSize_t ReDirStatus_t::fileSize() {
#ifdef __linux__
   return getStatus()->st_size;
#elif defined __WIN32__
   return ((int64_t) m_data.nFileSizeHigh << 32) + m_data.nFileSizeLow;
#endif
}

/**
 * Returns the file time as a string.
 *
 * @param buffer    OUT: the file time
 * @return          <code>buffer.str()</code> (for chaining)
 */
const char* ReDirStatus_t::filetimeAsString(ReByteBuffer& buffer) {
   return filetimeToString(modified(), buffer);
}

/**
 * Converts a filetime to a string.
 *
 * @param time		the filetime to convert
 * @param buffer	OUT: the buffer for the string
 * @return 			<code>buffer.str()</code>, e.g. "2014.01.07 02:59:43"
 */
const char* ReDirStatus_t::filetimeToString(const ReFileTime_t* time,
      ReByteBuffer& buffer) {
   time_t time1 = filetimeToTime(time);
   struct tm* time2 = localtime(&time1);
   buffer.setLength(4 + 2 * 2 + 2 * 2 + 1 + 3 * 2 + 2 * 1);
   strftime(buffer.buffer(), buffer.length(), "%Y.%m.%d %H:%M:%S", time2);
   return buffer.str();
}

/**
 * Converts a filetime to a unix time (seconds since the Epoche).
 *
 * @param filetime		the filetime to convert
 * @return 				the count of seconds since 1.1.1970
 */
time_t ReDirStatus_t::filetimeToTime(const ReFileTime_t* filetime) {
#ifdef __linux__
   return filetime->tv_sec;
#elif defined __WIN32__
   // 64-bit arithmetic:
   LARGE_INTEGER date, adjust;
   date.HighPart = filetime->dwHighDateTime;
   date.LowPart = filetime->dwLowDateTime;
   // 100-nanoseconds = milliseconds * 10000
   adjust.QuadPart = 11644473600000 * 10000;
   // removes the diff between 1970 and 1601
   date.QuadPart -= adjust.QuadPart;
   // converts back from 100-nanoseconds to seconds
   time_t rc = (time_t) (date.QuadPart / 10000000);
   return rc;
#endif
}

/**
 * Loads the info about the first file into the instance.
 *
 * @return	<code>true</code>: success
 */
bool ReDirStatus_t::findFirst() {
   bool rc = false;
#if defined __linux__
   if (m_handle != NULL)
      closedir(m_handle);
   m_handle = opendir(m_path.str());
   rc = m_handle != NULL && (m_data = readdir(m_handle)) != NULL;
   m_status.st_ino = 0;
#elif defined __WIN32__
   if (m_handle != INVALID_HANDLE_VALUE)
      FindClose(m_handle);
   ReByteBuffer thePath(m_path);
   thePath.append(m_path.lastChar() == '\\' ? "*" : "\\*");
   m_handle = FindFirstFileA(thePath.str(), &m_data);
   rc = m_handle != INVALID_HANDLE_VALUE;
#endif
   m_fullName.setLength(0);
   return rc;
}

/**
 * Loads the info about the next file into the instance.
 *
 * @return	<code>true</code>: success
 */
bool ReDirStatus_t::findNext() {
#if defined __linux__
   bool rc = m_handle != NULL && (m_data = readdir(m_handle)) != NULL;
   m_status.st_ino = 0;
#elif defined __WIN32__
   bool rc = m_handle != INVALID_HANDLE_VALUE && FindNextFileA(m_handle, &m_data);
#endif
   m_fullName.setLength(0);
   return rc;
}

/**
 * Frees the resources of an instance.
 */
void ReDirStatus_t::freeEntry() {
#if defined __linux__
   if (m_handle != NULL) {
      closedir(m_handle);
      m_handle = NULL;
   }
#elif defined __WIN32__
   if (m_handle != INVALID_HANDLE_VALUE) {
      FindClose(m_handle);
      m_handle = INVALID_HANDLE_VALUE;
   }
#endif
   m_path.setLength(0);
   m_fullName.setLength(0);
}

/**
 * Returns the full filename (with path).
 *
 * @return	the filename with path
 */
const char* ReDirStatus_t::fullName() {
   if (m_fullName.length() == 0)
      m_fullName.set(m_path).append(node(), -1);
   return m_fullName.str();
}

#if defined __WIN32__
/** Gets the name of the file owner.
 *
 * @param handle		file handle (see <code>CreateFile()</code>)
 * @param name		OUT: the owner: [domain\\]name
 * @return			<code>true</code>: success
 */
bool ReDirStatus_t::getFileOwner(HANDLE handle, const char* file,
                                 ReByteBuffer& name, ReLogger* logger) {
   bool rc = false;
   PSID pSidOwner = NULL;
   PSECURITY_DESCRIPTOR pSD = NULL;
   if (GetSecurityInfo(handle, SE_FILE_OBJECT,
                       OWNER_SECURITY_INFORMATION, &pSidOwner, NULL, NULL, NULL, &pSD) != ERROR_SUCCESS) {
      if (logger != NULL)
         logger->sayF(LOG_ERROR | CAT_FILE, LC_GET_FILE_OWNER_1,
                      "GetSecurityInfo($1): $2").arg(file).arg((int) GetLastError()).end();
   } else {
      char accountName[128];
      char domainName[128];
      DWORD dwAcctName = sizeof accountName;
      DWORD dwDomainName = sizeof domainName;
      SID_NAME_USE eUse = SidTypeUnknown;
      if (! LookupAccountSid(NULL, pSidOwner, accountName, &dwAcctName, domainName,
                             &dwDomainName, &eUse)) {
         if (logger != NULL)
            logger->sayF(LOG_ERROR | CAT_SECURITY, LC_GET_FILE_OWNER_2,
                         "LookupAccountSid(): $1").arg((int) GetLastError()).end();
      } else {
         if (dwDomainName > 0)
            name.append(domainName).appendChar('\\');
         name.append(accountName);
         rc = true;
      }
   }
   return rc;
}
#endif /* __WIN32__ */

#if defined __WIN32__
/** Tries to get a privilege.
 *
 * @param privilege	the name of the privilege, e.g. "SeBackup"
 * @param logger		logger for error logging
 */
bool ReDirStatus_t::getPrivilege(const char* privilege, ReLogger* logger) {
   bool rc = false;
   LUID luidPrivilege;
   HANDLE hAccessToken;
   if (! OpenProcessToken (GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hAccessToken)) {
      if (logger != NULL)
         logger->sayF(LOG_ERROR | CAT_FILE, LC_GET_PRIVILEGE_1,
                      "OpenProcessToken(): $1").arg((int) GetLastError()).end();
   } else if (! LookupPrivilegeValue (NULL, SE_BACKUP_NAME, &luidPrivilege)) {
      if (logger != NULL)
         logger->sayF(LOG_ERROR | CAT_FILE, LC_GET_PRIVILEGE_2,
                      "LookupPrivilegeValue(): $1").arg((int) GetLastError()).end();
   } else {
      TOKEN_PRIVILEGES tpPrivileges;
      tpPrivileges.PrivilegeCount = 1;
      tpPrivileges.Privileges[0].Luid = luidPrivilege;
      tpPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      if (AdjustTokenPrivileges (hAccessToken, FALSE, &tpPrivileges,
                                 0, NULL, NULL) == 0)
         rc = true;
      else {
         int error = GetLastError();
         if (error != 1300 && logger != NULL)
            logger->sayF(LOG_ERROR | CAT_FILE, LC_GET_PRIVILEGE_3,
                         "AdjustTokenPrivileges(): $1").arg((int) GetLastError()).end();
      }
   }
   return rc;
}
#endif /* __WIN32__ */

/**
 * Tests whether the instance is a directory.
 *
 * @return	<code>true</code>: instance contains the data of a directory
 */
bool ReDirStatus_t::isDirectory() {
#ifdef __linux__
   return m_data->d_type == DT_DIR
          || (m_data->d_type == DT_UNKNOWN && S_ISDIR(getStatus()->st_mode));
#elif defined __WIN32__
   return 0 != (m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

/**
 * Tests whether the instance contains data about "." or "..".
 *
 * @return	<code>true</code>: an ignorable entry has been found
 */
bool ReDirStatus_t::isDotDir() const {
#ifdef __linux__
   bool rc = m_data == NULL
             || (m_data->d_name[0] == '.'
                 && (m_data->d_name[1] == '\0'
                     || (m_data->d_name[1] == '.' && m_data->d_name[2] == '\0')));
#elif defined __WIN32__
   bool rc = m_data.cFileName[0] == '.' && (m_data.cFileName[1] == '\0'
             || (m_data.cFileName[1] == '.' && m_data.cFileName[2] == '\0'));
#endif
   return rc;
}

/**
 * Tests whether the instance is a symbolic link.
 *
 * Unter windows it tests whether the the instance is a reparse point.
 *
 * @return	<code>true</code>: instance contains the data of a link
 */
bool ReDirStatus_t::isLink() {
   bool rc;
#ifdef __linux__
   rc = m_data->d_type == DT_LNK
        || (m_data->d_type == DT_UNKNOWN && S_ISLNK(getStatus()->st_mode));
#elif defined __WIN32__
   rc = 0 != (m_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
#endif
   return rc;
}
/**
 * Tests whether the instance is a "normal" file.
 *
 * @return	<code>true</code>: instance contains the data of a not special file
 */
bool ReDirStatus_t::isRegular() {
#ifdef __linux__
   return m_data->d_type == DT_REG
          || (m_data->d_type == DT_UNKNOWN && S_ISREG(getStatus()->st_mode));
#elif defined __WIN32__
   return 0 == (m_data.dwFileAttributes & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE));
#endif
}
/**
 * Returns the modification time.
 *
 * @return	the modification time
 */
const ReFileTime_t* ReDirStatus_t::modified() {
#ifdef __linux__
   return &(getStatus()->st_mtim);
#elif defined __WIN32__
   return &m_data.ftLastWriteTime;
#endif
}

/**
 * Returns the name of the current file (without path).
 *
 * @return	the name of the current file.
 */
const char* ReDirStatus_t::node() const {
#ifdef __linux__
   return m_data->d_name;
#elif defined __WIN32__
   return m_data.cFileName;
#endif
}

inline void addRight(int mode, ReByteBuffer& buffer) {
   char right;
   switch (mode & 7) {
   case 1:
      right = 'x';
      break;
   case 2:
      right = 'w';
      break;
   case 3:
      right = 'X';
      break;
   case 4:
      right = 'r';
      break;
   case 5:
      right = 'R';
      break;
   case 6:
      right = 'W';
      break;
   case 7:
      right = 'A';
      break;
   default:
      right = '-';
      break;
   }
   buffer.appendChar(right);
}
inline void addId(const char* id, int maxLength, ReByteBuffer& buffer) {
   int length = strlen(id);
   if (length == maxLength)
      buffer.append(id, length);
   else if (length < maxLength)
      buffer.append(id, length).appendChar(' ', maxLength - length);
   else {
      buffer.append(id, 2);
      buffer.append(id + length - maxLength - 2, maxLength - 2);
   }
}
/**
 * Returns the file rights as a string.
 *
 * @param buffer		OUT: the file rights
 * @param numerical		<code>true</code>: the owner/group should be numerical (UID/GID)
 * @param ownerWidth	the width for group/owner
 * @return				<code>buffer.str()</code> (for chaining)
 */
const char* ReDirStatus_t::rightsAsString(ReByteBuffer& buffer, bool numerical,
      int ownerWidth) {
   buffer.setLength(0);
#if defined __linux__
   if (numerical) {
      buffer.appendInt(getStatus()->st_mode & ALLPERMS, "%04o");
      buffer.appendInt(getStatus()->st_uid, " %4d");
      buffer.appendInt(getStatus()->st_gid, " %4d");
   } else {
      int mode = getStatus()->st_mode & ALLPERMS;
      addRight(mode >> 6, buffer);
      addRight(mode >> 3, buffer);
      addRight(mode, buffer);
      buffer.appendChar(' ');
      struct passwd* passwd = getpwuid(getStatus()->st_uid);
      if (passwd == NULL)
         buffer.appendInt(getStatus()->st_uid, "%4d");
      else
         addId(passwd->pw_name, 5, buffer);
      buffer.appendChar(' ');
      struct group* group = getgrgid(getStatus()->st_gid);
      if (group == NULL)
         buffer.appendInt(getStatus()->st_gid, "%4d");
      else
         addId(group->gr_name, 5, buffer);
      buffer.appendChar(' ');
   }
#elif defined __WIN32__
   const char* name = fullName();
   HANDLE handle = INVALID_HANDLE_VALUE;
   if (! isDirectory()) {
      if ( (handle = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
         m_logger->sayF(LOG_ERROR | CAT_FILE, LC_RIGHTS_AS_STRING_1,
                        "CreateFile($1): $2").arg(name).arg((int) GetLastError()).end();
   } else if (m_getPrivilege) {
      // we try only one time:
      m_getPrivilege = false;
      if (getPrivilege(SE_BACKUP_NAME, m_logger)) {
         if ( (handle = CreateFile(name, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                   NULL)) != INVALID_HANDLE_VALUE)
            m_logger->sayF(LOG_ERROR | CAT_FILE, LC_RIGHTS_AS_STRING_2,
                           "CreateFile($1): $2").arg(name).arg((int) GetLastError()).end();
      }
   }
   ReByteBuffer owner;
   if (handle != INVALID_HANDLE_VALUE)
      getFileOwner(handle, name, owner, m_logger);
   CloseHandle(handle);
   buffer.appendFix(owner.str(), owner.length(), ownerWidth, ownerWidth);
#endif
   return buffer.str();
}

/**
 * Converts the unix time (time_t) to the file time.
 *
 * @param time			the unix time (secondes since 1.1.1970)
 * @param filetime	OUT: the OS specific filetime
 */
void ReDirStatus_t::timeToFiletime(time_t time, ReFileTime_t& filetime) {
#ifdef __linux__
   filetime.tv_sec = time;
   filetime.tv_nsec = 0;
#elif defined __WIN32__
   LONGLONG ll = Int32x32To64(time, 10000000) + 116444736000000000;
   filetime.dwLowDateTime = (DWORD)ll;
   filetime.dwHighDateTime = ll >> 32;
#endif
}
/**
 * Returns the type of the entry.
 * return       the file type, e.g. TF_REGULAR
 */
ReDirStatus_t::Type_t ReDirStatus_t::type() {
   Type_t rc = TF_UNDEF;
#if defined __linux__
   int flags = getStatus()->st_mode;
   if (S_ISDIR(flags))
      rc = TF_SUBDIR;
   else if (flags == 0 || S_ISREG(flags))
      rc = TF_REGULAR;
   else if (S_ISLNK(flags))
      rc = TF_LINK;
   else if (S_ISCHR(flags))
      rc = TF_CHAR;
   else if (S_ISBLK(flags))
      rc = TF_BLOCK;
   else if (S_ISFIFO(flags))
      rc = TF_PIPE;
   else if (S_ISSOCK(flags))
      rc = TF_SOCKET;
   else
      rc = TF_OTHER;
#elif defined __WIN32__
   int flags = (m_data.dwFileAttributes & ~(FILE_ATTRIBUTE_READONLY
                | FILE_ATTRIBUTE_HIDDEN
                | FILE_ATTRIBUTE_SYSTEM
                | FILE_ATTRIBUTE_ARCHIVE
                | FILE_ATTRIBUTE_NORMAL
                | FILE_ATTRIBUTE_TEMPORARY
                | FILE_ATTRIBUTE_SPARSE_FILE
                | FILE_ATTRIBUTE_COMPRESSED
                | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
                | FILE_ATTRIBUTE_ENCRYPTED
                | FILE_ATTRIBUTE_HIDDEN));
   if (0 == flags)
      rc = TF_REGULAR;
   else if (0 != (m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      rc = (0 != (m_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
           ? TF_LINK_DIR : TF_SUBDIR;
   } else if (0 != (m_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
      rc = TF_LINK;
   else
      rc = TF_OTHER;
#endif
   return rc;
}

/**
 * Returns the filetype as a single character.
 *
 * @return	the filetype, e.g. 'd' for a directory
 */
char ReDirStatus_t::typeAsChar() {
   char rc = ' ';
   switch (type()) {
   case TF_REGULAR:
      rc = ' ';
      break;
   case TF_LINK:
      rc = 'l';
      break;
   case TF_SUBDIR:
      rc = 'd';
      break;
   case TF_CHAR:
      rc = 'c';
      break;
   case TF_BLOCK:
      rc = 'b';
      break;
   case TF_PIPE:
      rc = 'p';
      break;
   case TF_SOCKET:
      rc = 's';
      break;
   default:
      rc = 'o';
      break;
   }
   return rc;
}
/**
 * Constructor.
 */
ReDirEntryFilter::ReDirEntryFilter() :
   ReSerializable(m_serialId),
   m_types(ReDirStatus_t::TC_ALL),
   m_nodePatterns(NULL),
   m_pathPatterns(NULL),
   m_minSize(0),
   m_maxSize(-1),
   //m_minAge(0),
   //m_maxAge(0),
   m_minDepth(0),
   m_maxDepth(512),
   m_allDirectories(false) {
   setFiletimeUndef(m_minAge);
   setFiletimeUndef(m_maxAge);
}

/**
 * Destructor.
 */
ReDirEntryFilter::~ReDirEntryFilter() {
}

/**
 * Sets the members of the instance from a byte sequence.
 *
 * @param sequence	IN/OUT: the serialized byte sequence
 * @param length	INT/OUT	the length of <code>sequence</code>
 */
void ReDirEntryFilter::deserialize(const uint8_t*& sequence, size_t& length) {
   int id;
   unpackInt24(sequence, length, id);
   if (id != m_serialId)
      throw ReSerializeFormatException("wrong serialId", this);
   ReByteBuffer buffer;
   unpackString64k(sequence, length, buffer);
   bool ignoreCase;
   unpackBool(sequence, length, ignoreCase);
   m_nodePatterns->set(buffer.str(), ignoreCase);
   unpackString64k(sequence, length, buffer);
   unpackBool(sequence, length, ignoreCase);
   unpackString64k(sequence, length, buffer);
   m_pathPatterns->set(buffer.str(), ignoreCase);
   unpackInt64(sequence, length, m_minSize);
   unpackInt64(sequence, length, m_maxSize);
   int64_t value;
   unpackInt64(sequence, length, value);
   unpackBool(sequence, length, m_allDirectories);
}
/**
 * Packs the members into a byte sequence.
 *
 * @param sequence	IN/OUT: the place for the byte sequence
 */
ReByteBuffer& ReDirEntryFilter::serialize(ReByteBuffer& sequence) {
   sequence.appendBits24(m_serialId);
   packString64k(sequence, m_nodePatterns->patternString());
   packBool(sequence, m_nodePatterns->ignoreCase());
   packString64k(sequence, m_pathPatterns->patternString());
   packBool(sequence, m_pathPatterns->ignoreCase());
   sequence.appendBits64(m_minSize);
   sequence.appendBits64(m_maxSize);
   uint64_t value;
#if defined __linux__
   value = (m_minAge.tv_sec << 32) + m_minAge.tv_nsec;
   sequence.appendBits64(int64_t(value));
   value = (m_minAge.tv_sec << 32) + m_minAge.tv_nsec;
#elif defined __WIN32__
#error "missing impl"
#endif
   sequence.appendBits64(int64_t(value));
   packBool(sequence, m_allDirectories);
}

/**
 * Tests whether an entry matches the conditions of the filter.
 *
 * @param entry		entry to test
 * @return 			<code>true</code>: the entry matches the conditions of the filter<br>
 * 					<code>false</code>: otherwise
 */
bool ReDirEntryFilter::match(ReDirStatus_t& entry) {
   bool rc = false;
   do {
      if (m_allDirectories && entry.isDirectory()) {
         rc = true;
         break;
      }
      if (0 == (entry.type() & m_types))
         break;
      int64_t size = entry.fileSize();
      if (m_minSize > 0 && size < m_minSize)
         break;
      if (m_maxSize >= 0 && size > m_maxSize)
         break;
      if (!filetimeIsUndefined(m_minAge) && *entry.modified() > m_minAge)
         break;
      if (!filetimeIsUndefined(m_maxAge) && m_maxAge > *entry.modified())
         break;
      const char* node = entry.node();
      if (m_nodePatterns != NULL && !m_nodePatterns->match(node))
         break;
      rc = true;
   } while (false);
   return rc;
}
;

#ifdef __linux__
/**
 * Returns the status of the current file (lazy loading).
 *
 * @return	the status of the current file
 */
struct stat* ReDirStatus_t::getStatus() {
   if (m_status.st_ino == 0) {
      if (stat(fullName(), &m_status) != 0)
         m_status.st_ino = 0;
   }
   return &m_status;
}
#endif

/**
 * Constructor.
 */
ReDirTreeStatistic::ReDirTreeStatistic() :
   m_directories(0),
   m_files(0),
   m_sizes(0ll) {
}
/**
 * Builds a string describing the data.
 *
 * @param buffer		IN/OUT: a buffer for the result
 * @param append		<code>true</code>: the string will be appended to the buffer<br>
 * 						<code>false</code>: the buffer will be cleared at the beginning
 * @param formatFiles	the <code>sprintf</code> format for the file count, e.g. "%8d"
 * @param formatSizes	the <code>sprintf</code> format for the MByte format, e.g. "%12.6f"
 * @param formatDirs	the <code>sprintf</code> format for the directory count, e.g. "%6d"
 * @return				a human readable string
 */
const char* ReDirTreeStatistic::statisticAsString(ReByteBuffer& buffer,
      bool append, const char* formatFiles, const char* formatSizes,
      const char* formatDirs) {
   if (!append)
      buffer.setLength(0);
   buffer.appendInt(m_files, formatFiles);
   buffer.append(i18n("file(s)")).appendChar(' ');
   buffer.append(m_sizes / 1000.0 / 1000, formatSizes);
   buffer.append(" ", 1).append(i18n("MByte")).appendChar(' ');
   buffer.appendInt(m_directories, formatDirs);
   buffer.append(i18n("dirs(s)"));
   return buffer.str();
}

/**
 * Constructor.
 *
 * @param triggerCount	efficiency: only every N calls a time check takes place
 * @param interval		the minimum number of seconds between two traces
 */
ReTraceUnit::ReTraceUnit(int triggerCount, int interval) :
   m_count(0),
   m_triggerCount(triggerCount),
   m_lastTrace(0),
   m_interval(interval),
   m_startTime(time(NULL)) {
   m_lastTrace = m_startTime;
}
/**
 * Destructor.
 */
ReTraceUnit::~ReTraceUnit() {
}

/**
 * Prints a message.
 *
 * Often overwritten by a subclass.
 *
 * @param message	message for the trace
 * @return			<code>true</code> (for chaining)
 */
bool ReTraceUnit::trace(const char* message) {
   printf("%s\n", message);
   return true;
}

/**
 * Constructor.
 *
 * @param base		the base directory. The traversal starts at this point
 * @param tracer	actor doing the trace
 * @param logger	the logger
 */
ReTraverser::ReTraverser(const char* base, ReTraceUnit* tracer,
                         ReLogger* logger) :
   ReDirTreeStatistic(),
   m_minLevel(0),
   m_maxLevel(512),
   m_level(-1),
   m_base(base),
   // m_dirs
   m_passNoForDirSearch(2),
   m_dirPatterns(NULL),
   m_tracer(tracer),
   m_logger(logger) {
   memset(m_dirs, 0, sizeof m_dirs);
   m_dirs[0] = new ReDirStatus_t(m_logger);
   // remove a preceeding "./". This simplifies the pattern expressions:
   if (m_base.startsWith(
            ReByteBuffer(".").appendChar(OS_SEPARATOR_CHAR).str())) {
      m_base.remove(0, 2);
   }
}

/**
 * Destructor.
 */
ReTraverser::~ReTraverser() {
   destroy();
}

/**
 * Initializes the instance to process a new base.
 *
 * @param base	the base directory to search
 */
void ReTraverser::changeBase(const char* base) {
   destroy();
   m_base.setLength(0).append(base);
   memset(m_dirs, 0, sizeof m_dirs);
   m_dirs[0] = new ReDirStatus_t(m_logger);
   // remove a preceeding "./". This simplifies the pattern expressions:
   if (m_base.startsWith(
            ReByteBuffer(".").appendChar(OS_SEPARATOR_CHAR).str())) {
      m_base.remove(0, 2);
   }
}

/**
 * Releases the resources.
 */
void ReTraverser::destroy() {
   for (size_t ix = 0; ix < sizeof m_dirs / sizeof m_dirs[0]; ix++) {
      if (m_dirs[ix] != NULL) {
         m_dirs[ix]->freeEntry();
         delete m_dirs[ix];
         m_dirs[ix] = NULL;
      }
   }
}
/**
 * Returns the info about the next file in the directory tree traversal.
 *
 * @param level	OUT: the level relative to the base.<br>
 * 					0 means the file is inside the base.<br>
 * 					Not defined if the result is NULL
 * @return NULL	no more files<br>
 * 					otherwise: the stack entry with the next file in the
 * 					directory tree. May be a directory too
 */
ReDirStatus_t* ReTraverser::rawNextFile(int& level) {
   ReDirStatus_t* rc = NULL;
   bool alreadyRead = false;
   bool again;
   do {
      again = false;
      if (m_level < 0) {
         // Not yet initialized?
         if (m_dirs[0]->m_passNo == 2)
            rc = NULL;
         else {
            // first call:
            if (initEntry(m_base.str(), NULL, 0)) {
               m_directories++;
               if (1 != m_passNoForDirSearch)
                  rc = m_dirs[0];
               else
                  again = alreadyRead = true;
            }
         }
      } else {
         ReDirStatus_t* current = m_dirs[m_level];
         if (alreadyRead || current->findNext()) {
            alreadyRead = false;
            // a file or directory found:
            if (m_tracer != NULL && m_tracer->isCountTriggered()
                  && m_tracer->isTimeTriggered())
               m_tracer->trace(current->fullName());
            if (current->m_passNo != m_passNoForDirSearch) {
               // we search for any file:
               rc = m_dirs[m_level];
            } else {
               // we are interested only in true subdirectories:
               again = true;
               if (m_level < m_maxLevel && current->isDirectory()
                     && !current->isDotDir() && !current->isLink()
                     && (m_dirPatterns == NULL
                         || isAllowedDir(current->node()))) {
                  // open a new level
                  alreadyRead = initEntry(current->m_path,
                                          current->node(), m_level + 1);
                  m_directories++;
               }
            }
         } else {
            // the current subdir does not have more files:
            if (current->m_passNo == 1) {
               // we start the second pass:
               alreadyRead = initEntry(current->m_path, NULL, -1);
               current->m_passNo = 2;
               again = true;
            } else {
               // this subdirectory is complete. We continue in the parent directory:
               current->freeEntry();
               if (--m_level >= 0) {
                  again = true;
               }
            }
         }
      }
      if (rc != NULL && rc->isDotDir())
         again = true;
   } while (again);
   if (rc != NULL && !rc->isDirectory()) {
      m_files++;
      if (m_sizes >= 0)
         m_sizes += rc->fileSize();
   }
   level = m_level;
   return rc;
}
/**
 * Returns the info about the next file matching the filter options.
 *
 * @param level	OUT: the level relative to the base.<br>
 * 					0 means the file is inside the base.<br>
 * 					Not defined if the result is NULL
 * @param filter	NULL: every file matches<br>
 * 					otherwise: each found file must match this filter conditions
 * @return NULL	no more files<br>
 * 					otherwise: the info about the next file in the
 * 					directory tree
 */
ReDirStatus_t* ReTraverser::nextFile(int& level, ReDirEntryFilter* filter) {
   ReDirStatus_t* rc = rawNextFile(level);
   while (rc != NULL) {
      if (filter == NULL || filter->match(*rc)) {
         break;
      }
      rc = rawNextFile(level);
   }
   return rc;
}

/**
 * Initializes an entry in the directory entry stack.
 *
 * @param parent	the parent directory of the entry
 * @param node      the name of the directory belonging to the entry (without path)
 * @param level	    the index of the entry in the stack.<br>
 *                  If &lt; 0: m_levels and m_path will not be changed
 * @return          <code>true</code>: a new file is available<br>
 *                  <code>false</code>: findFirstEntry() signals: no entry.
 */
bool ReTraverser::initEntry(const ReByteBuffer& parent, const char* node,
                            int level) {
   bool rc = false;
   if (level < MAX_ENTRY_STACK_DEPTH) {
      if (level >= 0)
         m_level = level;
      if (m_dirs[m_level] == NULL)
         m_dirs[m_level] = new ReDirStatus_t(m_logger);
      ReDirStatus_t* current = m_dirs[m_level];
      current->m_passNo = 1;
      if (level >= 0) {
         current->m_path.set(parent.str(), parent.length());
         if (!parent.endsWith(OS_SEPARATOR))
            current->m_path.append(OS_SEPARATOR);
         if (node != NULL)
            current->m_path.append(node).append(OS_SEPARATOR);
      }
      rc = current->findFirst();
   }
   return rc;
}

/**
 * Sets some properties from a filter.
 *
 * @param filter	the filter with the properties to set
 */
void ReTraverser::setPropertiesFromFilter(ReDirEntryFilter* filter) {
   m_minLevel = filter->m_minDepth;
   m_maxLevel = filter->m_maxDepth;
   setDirPattern(filter->m_pathPatterns);
}

/**
 * Returns the info of an entry the directory stack.
 *
 * @param offsetFromTop		0: return the top of stack<br>
 * 							1: returns the entry one below the top<br>
 * 							2: ...
 * @return					NULL: not available<br>
 * 							otherwise: the wanted entry
 */
ReDirStatus_t* ReTraverser::topOfStack(int offsetFromTop) {
   ReDirStatus_t* rc = NULL;
   if (offsetFromTop >= 0 && m_level - 1 - offsetFromTop >= 0)
      rc = m_dirs[m_level - 1 - offsetFromTop];
   return rc;
}
