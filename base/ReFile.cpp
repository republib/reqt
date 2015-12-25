/*
 * ReFile.cpp
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
};

#if defined __linux__ || defined WIN32
void* memichr(void* heap, int cc, size_t length) {
   const char* heap2 = reinterpret_cast<const char*>(heap);
   int cc2 = tolower(cc);
   void* rc = NULL;
   while(length > 0) {
      if (cc2 == tolower(*heap2++)) {
         rc = (void*)(heap2 - 1);
         break;
      }
   }
   return rc;
}

int memicmp(const void* str1, const void* str2, size_t length) {
   const char* str12 = reinterpret_cast<const char*>(str1);
   const char* str22 = reinterpret_cast<const char*>(str2);
   int rc = 0;
   for (size_t ix = 0; ix < length; ix++) {
      int diff = tolower(*str12++) - tolower(*str22++);
      if (diff != 0) {
         rc = diff;
         break;
      }
   }
   return rc;
}
#endif /* __linux__ */

/**
 * Constructor.
 */
ReLines::ReLines() :
   QStringList(),
   m_empty() {
}
/**
 * Destructor.
 */
ReLines::~ReLines() {
}

/**
 * Checks that the summary size of the undo information remains under the maximum.
 *
 * @param stringSize	the additional string size
 * @return				<code>true</code>: undo info should be stored
 *						<code>false</code>: too large undo info, all info has
 *						been freed
 */
bool ReUndoList::checkSummarySize(qint64 stringSize) {
   static qint64 sizeStruct = (qint64) sizeof(UndoItem);
   while (m_currentUndoSize + stringSize + sizeStruct > m_maxUndoSize) {
      if (m_list.length() == 0)
         m_currentUndoSize = 0;
      else {
         UndoItem* item = m_list.at(0);
         m_currentUndoSize -= sizeof *item - item->m_string.length();
         m_list.removeAt(0);
         delete item;
      }
   }
   return stringSize + sizeStruct < m_maxUndoSize;
}

/**
 * Removes all lines.
 */
void ReLines::clear() {
   QStringList::clear();
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
bool ReFile::deleteTree(const QString& path, bool withBase, ReLogger* logger) {
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
            if (!deleteTree(full, true, logger))
               rc = false;
            else if (rmdir(I18N::s2b(full)) != 0) {
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
   if (withBase && (rmdir(I18N::s2b(path))) != 0) {
      rc = false;
      logger->logv(LOG_ERROR, LOC_DELETE_TREE_3,
                   "cannot delete directory (%d): %s", errno, I18N::s2b(path).constData());
   }
   return rc;
}

/**
 * Inserts one or more lines into the lines.
 *
 * @param lineNo	the line number (0..N-1) of the new line
 * @param text		the text to insert. May not contain newlines!
 * @param withUndo	<code>true</code>: prepares undo operation<br>
 *					<code>false</code>: undo is impossible
 */
void ReLines::insertLines(int lineNo, const QString& text, bool withUndo) {
   if (lineNo >= 0) {
      int count = 0;
      if (text.isEmpty())
         count = 1;
      else {
         count = ReQStringUtils::countOf(text, '\n');
         if (text.at(text.length() - 1) != '\n')
            count++;
      }
      if (withUndo)
         storeInsertLines(lineNo, count);
      int start = 0;
      int end;
      if (lineNo >= length()) {
         while ((end = text.indexOf('\n', start)) >= 0) {
            append(text.mid(start, end - start));
            start = end + 1;
         }
         if (start < text.length())
            append(text.mid(start));
      } else {
         while ((end = text.indexOf('\n', start)) >= 0) {
            insert(lineNo++, text.mid(start, end - start));
            start = end + 1;
         }
         if (start < text.length())
            insert(lineNo, text.mid(start));
      }
   }
}
/**
 * Inserts a text into a given position of the file.
 *
 * @param lineNo	the line number (0..N-1) of the insert position
 * @param col		the column number (0..M-1) of the insert position<br>
 *					col < 0: no insertion<br>
 *					col > length_of_the_line: the text will be appended to the line
 * @param text		the text to insert. May not contain newlines!
 * @param withUndo	<code>true</code>: prepares undo operation<br>
 *					<code>false</code>: undo is impossible
 */
void ReLines::insertPart(int lineNo, int col, const QString& text,
                         bool withUndo) {
   if (lineNo >= 0 && lineNo < lineCount() && col >= 0) {
      if (withUndo)
         storeInsertPart(lineNo, col, text.length());
      QString current = lineAt(lineNo);
      if (col == 0)
         replace(lineNo, text + current);
      else if (col < current.length())
         replace(lineNo, current.left(col) + text + current.mid(col));
      else
         replace(lineNo, current.left(col) + text);
   }
}

/**
 * Inserts a text into a given position of the file, with or without newlines.
 *
 * Note: the line separators will not be stored.
 *
 * @param lineNo	the line number (0..N-1) of the insert position
 * @param col		the column number (0..M-1) of the insert position
 * @param text		the text to insert. May contain line separators ('\n' or "\r\n").
 *					In this case the number of lines grows
 */
void ReLines::insertText(int lineNo, int col, const QString& text) {
   if (length() == 0)
      insertLines(0, "", true);
   int endOfLine = text.indexOf(QChar('\n'));
   if (endOfLine < 0)
      insertPart(lineNo, col, text, true);
   else {
      splitLine(lineNo, col, true);
      int newLines = 0;
      if (lineNo < length())
         insertPart(lineNo, col, text.left(endOfLine), true);
      else
         insertLines(lineNo, text.left(endOfLine), true);
      int lastEoLn = text.lastIndexOf('\n');
      if (lastEoLn != endOfLine) {
         int oldCount = lineCount();
         insertLines(lineNo + 1,
                     text.mid(endOfLine + 1, lastEoLn - endOfLine), true);
         newLines = lineCount() - oldCount;
      }
      if (lastEoLn != text.length() - 1) {
         int nextLine = lineNo + newLines + 1;
         if (nextLine < length())
            insertPart(nextLine, 0, text.mid(lastEoLn + 1), true);
         else
            insertLines(nextLine, text.mid(lastEoLn + 1), true);
      }
   }
}

/**
 * Joins a line and the following line into one line.
 *
 * @param first the line number of the first line
 * @return		<code>true</code>: the join has been done<br>
 *				<code>false</code>: wrong parameter (first)
 */
bool ReLines::joinLines(int first) {
   bool rc = false;
   if (first >= 0 && first < length() - 1) {
      replace(first, at(first) + at(first + 1));
      removeAt(first + 1);
      rc = true;
   }
   return rc;
}

/**
 * Removes a part of a line.
 *
 * @param lineNo	the line number (0..N-1) of the first position to delete
 * @param col		-1: join the current line with the previous line.
 *					>= line_length: join the current line with the following
 *					the column number (0..M-1) of the first position to delete
 * @param count		the number of character to delete
 * @param withUndo	<code>true</code>: prepares undo operation<br>
 *					<code>false</code>: undo is impossible
 * @return			<code>true</code>: a join of 2 lines has been done
 */
bool ReLines::removePart(int lineNo, int col, int count, bool withUndo) {
   bool rc = false;
   if (lineNo >= 0 && lineNo < lineCount() && count > 0) {
      const QString& current = at(lineNo);
      int length = current.length();
      if (col <= -1) {
         if (lineNo > 0) {
            if (withUndo)
               storeJoin(lineNo - 1, at(lineNo - 1).length());
            rc = joinLines(lineNo - 1);
         }
      } else if (col >= length) {
         if (withUndo)
            storeJoin(lineNo, length);
         rc = joinLines(lineNo);
      } else {
         if (col + count > length)
            count = length - col;
         if (withUndo)
            storeRemovePart(lineNo, col, current.mid(col, count));
         if (col == 0)
            replace(lineNo, current.mid(count));
         else if (col + count >= length)
            replace(lineNo, current.left(col));
         else
            replace(lineNo, current.left(col) + current.mid(col + count));
      }
   }
   return rc;
}

/**
 * Removes a given number of lines.
 *
 * If all lines are removed a single empty line will remain.
 *
 * @param start		the line number (0..N-1) of the first line to remove
 * @param count		the number of lines to delete
 * @param withUndo	<code>true</code>: prepares undo operation<br>
 *					<code>false</code>: undo is impossible
 */
void ReLines::removeLines(int start, int count, bool withUndo) {
   if (start >= 0 && start < length()) {
      if (start + count > length())
         count = length() - start;
      if (withUndo)
         storeRemoveLines(start, count, *this);
      for (int ix = start + count - 1; ix >= start; ix--)
         removeAt(ix);
      if (length() == 0)
         append(m_empty);
   }
}

/**
 * Splits a line at a given position into two lines.
 *
 * @param lineNo	the line number (0..N-1) of line to split
 * @param col		the column number (0..M-1) of the split point. The character
 *					of this position will be the first in the next line
 * @param withUndo	<code>true</code>: prepares undo operation<br>
 *					<code>false</code>: undo is impossible
 */
void ReLines::splitLine(int lineNo, int col, bool withUndo) {
   int count = length();
   if (lineNo >= 0 && lineNo < count && col >= 0) {
      QString current = at(lineNo);
      if (withUndo)
         storeSplit(lineNo, col);
      if (col >= current.length()) {
         if (lineNo >= count - 1)
            append("");
         else
            insert(lineNo + 1, "");
      } else {
         QString x = current.mid(col);
         if (lineNo >= count - 1)
            append(current.mid(col));
         else
            insert(lineNo + 1, current.mid(col));
         x = current.left(col);
         replace(lineNo, current.left(col));
      }
   }
}
/**
 * Rewinds the last change operation (insertion/deletion).
 *
 * @param lineNo	OUT: the line number of the restored operation
 * @param col		OUT: the column of the restored operation
 */
void ReLines::undo(int& lineNo, int& col) {
   if (m_list.length() > 0) {
      UndoItem* item = pop();
      lineNo = item->m_lineNo;
      col = item->m_position;
      switch (item->m_type) {
      case UIT_INSERT_PART:
         removePart(item->m_lineNo, item->m_position, item->m_length, false);
         break;
      case UIT_INSERT_LINES:
         removeLines(item->m_lineNo, item->m_length, false);
         break;
      case UIT_SPLIT:
         joinLines(item->m_lineNo);
         break;
      case UIT_JOIN:
         splitLine(item->m_lineNo, item->m_position, false);
         break;
      case UIT_REMOVE_LINES:
         insertLines(item->m_lineNo, item->m_string, false);
         break;
      case UIT_REMOVE_PART:
         insertPart(item->m_lineNo, item->m_position, item->m_string, false);
         break;
      default:
         break;
      }
      delete item;
   }
}

/**
 * Constructor.
 *
 * @param filename  name of the file
 * @param readOnly	<code>true</code>: the file will be opened for
 * 					reading only
 * @param logger	the logger
 */
ReFile::ReFile(const QString& filename, bool readOnly, ReLogger* logger) :
   ReLineSource(),
   ReLines(),
   m_endOfLine(),
   m_filename(filename),
   m_file(filename),
   m_block(NULL),
   // in 32-bit address space we allocate only 10 MByte, in 64-bit environments 100 GByte
   m_blocksize(
      sizeof(void*) <= 4 ?
      10 * 1024 * 1024ll : 0x100ll * 0x10000 * 0x10000),
   m_blockOffset(0),
   m_filesize(0),
   m_startOfLine(NULL),
   m_lineLength(0),
   m_lineOffset(0),
   m_currentLineNo(0),
   m_maxLineLength(0x10000),
   m_content(),
   m_readOnly(readOnly),
   m_logger(logger) {
#if defined __linux__
   setEndOfLine("\n");
#elif defined __WIN32__
   setEndOfLine("\r\n");
#endif
   if (readOnly) {
      m_file.open(QIODevice::ReadOnly);
      m_filesize = m_file.size();
   } else {
      read();
   }
}

/**
 * Destructor.
 */
ReFile::~ReFile() {
   close();
}

/**
 * Returns the blocksize.
 *
 * @return the blocksize
 */
int64_t ReFile::blocksize() const {
   return m_blocksize;
}

/**
 * Frees the resources.
 */
void ReFile::close() {
   ReFile::clearUndo();
   m_file.close();
}

/**
 * Returns the current filename.
 *
 * @return  the filename (with path)
 */
QString ReFile::filename() const {
   return m_filename;
}
/**
 * Finds the next line with the string.
 *
 * @param toFind		the string to find    "" or the pattern matching the result
 * @param ignoreCase	true: the search is case insensitive
 * @param lineNo		OUT: 0 or the line number
 * @param line			OUT: "" or the found line
 * @return				true: a line has been found<br>
 *						false: a line has not been found
 */
bool ReFile::findLine(const char* toFind, bool ignoreCase, int& lineNo,
                      QString* line) {
   bool rc = false;
   int length;
   int sourceLength = strlen(toFind);
   char* start;
   char first = toFind[0];
   while (!rc && (start = nextLine(length)) != NULL) {
      const char* ptr = start;
      int restLength = length - sourceLength + 1;
      while (restLength > 0
             && (ptr = reinterpret_cast<const char*>(
                          ignoreCase ?
                          memchr(start, first, restLength) :
                          memichr(start, first, restLength))) != NULL) {
         if ((
                  ignoreCase ?
                  _memicmp(ptr, toFind, sourceLength) :
                  memcmp(ptr, toFind, sourceLength)) == 0) {
            rc = true;
            lineNo = m_currentLineNo;
            QByteArray buffer(m_startOfLine, m_lineLength);
            if (line == NULL)
               *line = QString::fromUtf8(buffer);
            break;
         }
         restLength = length - (ptr - start) - sourceLength + 1;
      }
   }
   return rc;
}
/**
 * Finds the next line with the given conditions.
 *
 * @param includePattern    "" or the pattern matching the result
 * @param includeIsRegExpr  true: the pattern is a regular expression<br>
 *                          false: the pattern is a string without meta chars
 * @param includeIgnoreCase true: includePattern is case insensitive
 * @param excludePattern    "" or the pattern wich must not match the result
 * @param excludeIsRegExpr  true: the exclude pattern is a regular expression<br>
 *                          false: the pattern is a string without meta chars
 * @param excludeIgnoreCase true: excludePattern is case insensitive
 * @param lineNo            OUT: 0 or the line number
 * @param line              OUT: "" or the found line
 * @return                  true: a line has been found<br>
 *                          false: a line has not been found
 */
bool ReFile::findLine(const QString& includePattern, bool includeIsRegExpr,
                      bool includeIgnoreCase, const QString& excludePattern,
                      bool excludeIsRegExpr, bool excludeIgnoreCase, int& lineNo, QString* line) {
   bool rc = false;
   if (line != NULL)
      *line = "";
   lineNo = 0;
   if (!includePattern.isEmpty()) {
   }
   return rc;
}

int ReFile::hasMoreLines(int index) {
   bool rc = false;
#if 1
   ReUseParameter(index);
#else
   if (m_countLines >= 0) {
      rc = index < m_countLines - 1;
   } else {
      seek(m_lastLineNo);
      while (index > m_lastLineNo + 1) {
         if (! nextLine()) {
            break;
         }
      }
      rc = index < m_lastLineNo;
   }
#endif
   return rc;
}

/**
 * @brief Gets the line behind the current line.
 *
 * @param length    OUT: the line length
 * @return          NULL: end of file reached<br>
 *                  otherwise: the pointer to the next line
 */
char* ReFile::nextLine(int& length) {
   char* rc = NULL;
   length = 0;
   if (m_lineOffset + m_lineLength < m_filesize) {
      int lineLength;
      if (m_currentLineNo == 65639)
         m_currentLineNo += 0;
      rc = m_startOfLine = remap(m_lineOffset += m_lineLength,
                                 m_maxLineLength, lineLength);
      const char* ptr = reinterpret_cast<const char*>(memchr(rc, '\n',
                        lineLength));
      if (ptr != NULL)
         lineLength = ptr - rc + 1;
      length = m_lineLength = lineLength;
      m_currentLineNo++;
   }
   return rc;
}

/**
 * @brief Gets the line in top of the current line.
 *
 *
 * @param length    OUT: the line length
 * @return          NULL: begin of file reached<br>
 *                  otherwise: the pointer to the previous line
 */
char* ReFile::previousLine(int& length) {
   char* rc = NULL;
   length = 0;
   if (m_lineOffset > 0) {
      int lineLength;
      rc = remap(m_lineOffset - m_lineLength, m_maxLineLength, lineLength);
      m_startOfLine = rc + lineLength - 1;
      m_lineLength = 0;
      while (m_startOfLine > rc && m_startOfLine[-1] != '\n') {
         m_lineLength++;
         m_startOfLine--;
      }
      rc = m_startOfLine;
      length = m_lineLength;
   }
   return rc;
}

/**
 * Reads the content of the file into the line list.
 *
 * @param filename  the full name of the file. If "" the internal name will be used
 * @return          <code>true</code>success<br>
 *                  <code>false</code>file not readable
 */
bool ReFile::read(const QString& filename) {
   QFile inputFile(filename.isEmpty() ? m_filename : filename);
   bool rc = false;
   if (inputFile.open(QIODevice::ReadOnly)) {
      rc = true;
      m_filesize = inputFile.size();
      reserve(m_filesize / 80 * 11 / 10);
      QTextStream in(&inputFile);
      int countCR = 0;
      while (!in.atEnd()) {
         QByteArray line = inputFile.readLine();
         int len = line.length();
         if (len > 0 && line.at(len - 1) == '\n') {
            len--;
            line.remove(len, 1);
            if (len > 0 && line.at(len - 1) == '\r') {
               countCR++;
               line.remove(--len, 1);
            }
         }
         append(QString::fromUtf8(line));
      }
      if (countCR > lineCount() / 2)
         setEndOfLine("\r\n");
      else
         setEndOfLine("\n");
      inputFile.close();
   }
   return rc;
}

/**
 * Reads a string from a given file.
 *
 * @param filename      name of the file to read
 * @param buffer        OUT: the buffer to write
 * @return              <code>buffer</code> (for chaining)
 */
QByteArray& ReFile::readFromFile(const char* filename, QByteArray& buffer) {
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
 * Creates an IO mapping for a block of a given size at a given offset.
 *
 * IO mapping allows to access file data in a very efficient way: the os
 * automatically loads the accessed data.
 *
 * The instance administrates an internal block independent from the requested
 * block which is normally much larger.
 *
 * @param offset    the distance of the blockstart to the begin of the file
 * @param size      the requested block size
 * @param length    OUT: the length of the returned block.
 *                  May be smaller than requested size
 * @return          NULL: wrong parameters<br>
 *                  otherwise: a pointer to the block
 */
char* ReFile::remap(int64_t offset, int size, int& length) {
   char* rc = NULL;
   length = 0;
   // valid block?
   if (offset >= 0 && offset < m_filesize) {
      if (size > m_blocksize)
         size = m_blocksize;
      if (offset + size > m_filesize)
         // fs=3, o=1, => s= 2 = fs - o
         size = m_filesize - offset;
      // Note: size <= m_blocksize
      if (m_block != NULL && offset >= m_blockOffset
            && offset + size <= m_blockOffset + m_blocksize) {
         // new block is inside the internal block:
         // no remapping needed
         rc = m_block + (offset - m_blockOffset);
         length = size;
      } else {
         // the new block will be surrounded by the internal block
         m_blockOffset = offset - m_blocksize / 2;
         if (m_blockOffset < 0)
            m_blockOffset = 0;
         else if (m_blockOffset + m_blocksize > m_filesize)
            m_blockOffset = m_filesize - m_blocksize;
         if (m_block != NULL)
            m_file.unmap(reinterpret_cast<uchar*>(m_block));
         m_block = reinterpret_cast<char*>(m_file.map(m_blockOffset,
                                           m_blocksize));
         rc = m_block + (offset - m_blockOffset);
         length = m_blocksize - (rc - m_block);
         if (length > size)
            length = size;
      }
   }
   return rc;
}

/**
 * Sets the read position prior to the begin of file.
 */
void ReFile::rewind() {
   m_currentLineNo = 0;
   m_lineLength = 0;
   m_lineOffset = 0;
   m_startOfLine = NULL;
}

/**
 * Sets the internal blocksize.
 *
 * @param blocksize the blocksize to set
 */
void ReFile::setBlocksize(const int64_t& blocksize) {
   m_blocksize = blocksize;
   if (m_maxLineLength > blocksize / 2)
      m_maxLineLength = blocksize / 2;
}

/**
 * Sets the current filename.
 *
 * @param filename  the new filename
 */
void ReFile::setFilename(const QString& filename) {
   m_filename = filename;
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
QByteArray ReFile::tempDir(const char* node, const char* parent,
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
 * @brief Returns a name of a file in a temporary directory.
 *
 * @param node              the file's name without path
 * @param parent            NULL or the name of a subdirectory the file will be inside
 * @param deleteIfExists    true: if the file exists it will be removed
 * @return                  the full name of a temporary file
 */
QByteArray ReFile::tempFile(const char* node, const char* parent,
                            bool deleteIfExists) {
   QByteArray rc(tempDir(parent));
   if (!rc.endsWith('/'))
      rc += '/';
   rc += node;
   struct stat info;
   if (deleteIfExists && stat(rc.constData(), &info) == 0)
      unlink(rc.constData());
   return rc;
}

/**
 * Reads the content of the file into the line list.
 *
 * @param filename	the full name of the file. If "" the internal name will be used
 * @return			<code>true</code>success<br>
 *					<code>false</code>file not readable
 */
bool ReFile::write(const QString& filename) {
   bool rc = false;
   if (!m_readOnly) {
      QFile outputFile(filename.isEmpty() ? m_filename : filename);
      if (outputFile.open(QIODevice::WriteOnly)) {
         QByteArray buffer;
         int maxIx = length() - 1;
         for (int ix = 0; ix <= maxIx; ix++) {
            const QString& line = at(ix);
            buffer = I18N::s2b(line);
            outputFile.write(buffer.constData(), buffer.length());
            outputFile.write(m_endOfLine.constData(), m_endOfLine.length());
         }
         rc = true;
         outputFile.close();
      }
   }
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
void ReFile::writeToFile(const char* filename, const char* content,
                         size_t contentLength, const char* mode) {
   FILE* fp = fopen(filename, mode);
   if (fp != NULL) {
      if (contentLength == (size_t) - 1)
         contentLength = strlen(content);
      fwrite(content, 1, contentLength, fp);
      fclose(fp);
   }
}

/**
 * Constructor.
 */
ReUndoList::ReUndoList() :
   m_list(),
   m_current(NULL),
   m_lastLine(-1),
   m_lastPosition(-1),
   m_maxUndoSize(10 * 1024 * 1024),
   m_currentUndoSize(0) {
}

/**
 * Destructor.
 */
ReUndoList::~ReUndoList() {
   clearUndo();
}

/**
 * Frees the resources.
 */
void ReUndoList::clearUndo() {
   for (int ii = 0; ii < m_list.length(); ii++) {
      delete m_list.at(ii);
   }
   m_list.clear();
}

/**
 * Returns the current maximum size of the undo information.
 *
 * @return	the maximum size of the undo information
 */
qint64 ReUndoList::maxUndoSize() const {
   return m_maxUndoSize;
}
/**
 * Prepares the undo operation of an insertion in a given lineNo.
 *
 * @param lineNo	the line number of the the first line to insert
 * @param count		the number of lines to insert
 */
void ReUndoList::storeInsertLines(int lineNo, int count) {
   UndoItem* item = new UndoItem();
   item->m_type = UIT_INSERT_LINES;
   item->m_lineNo = lineNo;
   item->m_position = 0;
   item->m_length = count;
   item->m_isPart = false;
   m_list.append(item);
   checkSummarySize(0);
   m_currentUndoSize += (qint64) sizeof(*item);
}

/**
 * Prepares the undo operation of an insertion in a given line.
 *
 * @param lineNo	the line number of the insertion point
 * @param col	the index of the insertion point in the line
 * @param count	the number of chars has been inserted
 */
void ReUndoList::storeInsertPart(int lineNo, int col, int count) {
   UndoItem* item = new UndoItem();
   item->m_type = UIT_INSERT_PART;
   item->m_lineNo = lineNo;
   item->m_position = col;
   item->m_length = count;
   item->m_isPart = false;
   m_list.append(item);
   checkSummarySize(0);
   m_currentUndoSize += (qint64) sizeof(*item);
}

/**
 * Prepares the undo operation of a join of two lines.
 *
 * @param lineNo		the number of the first line to join. The second is line+1
 * @param length	the length of the first line
 */
void ReUndoList::storeJoin(int lineNo, int length) {
   UndoItem* item = new UndoItem();
   item->m_type = UIT_JOIN;
   item->m_lineNo = lineNo;
   item->m_position = length;
   item->m_length = 0;
   item->m_isPart = false;
   m_list.append(item);
   checkSummarySize(0);
   m_currentUndoSize += (qint64) sizeof(*item);
}

/**
 * Prepares the undo operation of the deletion of a part of a line.
 *
 * @param lineNo		the line number of the deletion point
 * @param col		the index of the deletion point in the line
 * @param string	the text which is deleted
 */
void ReUndoList::storeRemovePart(int lineNo, int col, const QString& string) {
   UndoItem* item = new UndoItem();
   item->m_type = UIT_REMOVE_PART;
   item->m_lineNo = lineNo;
   item->m_position = col;
   item->m_length = string.length();
   item->m_string = string;
   item->m_isPart = false;
   m_list.append(item);
}

/**
 * Sets the maximum size of the undo information.
 *
 * If the undo info exceeds this value the oldest infos will be removed.
 *
 * @param maxUndoSize	the new maximum size of undo information
 */
void ReUndoList::setMaxUndoSize(qint64 maxUndoSize) {
   if (maxUndoSize < (qint64) sizeof(UndoItem) + 1)
      maxUndoSize = (qint64) sizeof(UndoItem) + 1;
   m_maxUndoSize = maxUndoSize;
}

/**
 * Prepares the undo operation of a line split.
 *
 * @param lineNo		the line number of line to split
 * @param col		the index of the split point. The line will be split behind
 */
void ReUndoList::storeSplit(int lineNo, int col) {
   UndoItem* item = new UndoItem();
   item->m_type = UIT_SPLIT;
   item->m_lineNo = lineNo;
   item->m_position = col;
   item->m_length = 0;
   item->m_isPart = false;
   m_list.append(item);
   checkSummarySize(0);
   m_currentUndoSize += (qint64) sizeof(*item);
}

/**
 * Prepares the undo operation of some lines.
 * @param lineNo	the number of the first line to remove
 * @param count	the number of lines to remove
 * @param list	the list containing the lines for extracting the line content
 */
void ReUndoList::storeRemoveLines(int lineNo, int count,
                                  const QStringList& list) {
   qint64 size = 0;
   // Calculate the additional space
   for (int ii = lineNo + count - 1; ii >= lineNo; ii--)
      // +1: the newline
      size += list.at(ii).length() + 1;
   //@ToDo: handle more than 2**31 summary length
   if (checkSummarySize(size)) {
      assert(size <= INT_MAX);
      UndoItem* item = new UndoItem();
      item->m_type = UIT_REMOVE_LINES;
      item->m_lineNo = lineNo;
      item->m_position = 0;
      item->m_length = count;
      item->m_string.reserve(size);
      for (int ii = lineNo; ii < lineNo + count; ii++)
         item->m_string.append(list.at(ii)).append('\n');
      m_list.append(item);
   }
}

/**
 * Returns the last element of the undo item and remove it from the list.
 *
 * Note: Do not forget to free the returned item!
 *
 * @return the last element of the list
 */
ReUndoList::UndoItem* ReUndoList::pop() {
   ReUndoList::UndoItem* item = m_list.last();
   m_list.removeLast();
   return item;
}
