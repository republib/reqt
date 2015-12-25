/*
 * ReSource.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Reading from several input media.
 *
 * The abstract base class <code>ReReader</code> and its concrete derivations
 * <code>ReStringReader</code>, <code>ReFileReader</code> are used to read
 * from one medium.
 * The <code>ReSource</code> combines several readers and build an uniquely
 * usable input stream.
 */
/** @file expr/ReSource.hpp
 *
 * @brief Definitions for reading from several input media.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

/** @class ReSource ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Stores all source positions.
 *
 */

/** @class ReSourceUnit ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Implements the base class of input source units.
 *
 * A source unit is set of input lines with a name, e.g. a file.
 */

/**
 * @brief Constructor
 * @param name      name of the unit
 * @param reader    the reader which can read the unit
 */
ReSourceUnit::ReSourceUnit(ReSourceUnitName name, ReReader* reader) :
   m_name(name),
   m_lineNo(0),
   m_reader(reader) {
}

/**
 * @brief Destructor.
 */
ReSourceUnit::~ReSourceUnit() {
}

/**
 * @brief Returns the name.
 * @return  the name
 */
ReSourceUnitName ReSourceUnit::name() const {
   return m_name.constData();
}

/**
 * @brief Returns the line number.
 *
 * @return  the line number
 */
int ReSourceUnit::lineNo() const {
   return m_lineNo;
}

/**
 * @brief Sets the line number.
 *
 * @param lineNo    the new line number
 */
void ReSourceUnit::setLineNo(int lineNo) {
   m_lineNo = lineNo;
}
/**
 * @brief Returns the reader of the instance.
 *
 * @return  the reader belonging to the instance
 */
ReReader* ReSourceUnit::reader() const {
   return m_reader;
}

/** @class ReSourcePosition ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Stores a precise position in the input source.
 *
 * The input source contains normally a lot of nested source units.
 *
 * Each source unit owns a name and a sequence of lines.
 *
 * The mostly used source unit is a text file.
 *
 * A precice position is the stack of source unit positions.
 */

/**
 * @brief Constructor.
 */
ReSourcePosition::ReSourcePosition() :
   m_sourceUnit(NULL),
   m_lineNo(0),
   m_column(0),
   m_caller(NULL) {
}

/**
 * @brief Constructor.
 *
 * @param unit      name of the input source (normally a file)
 * @param lineNo    line number inside the input source
 * @param colNo     distance to the line start
 */
ReSourcePosition::ReSourcePosition(ReSourceUnit* unit, int lineNo, int colNo) :
   m_sourceUnit(unit),
   m_lineNo(lineNo),
   m_column(colNo),
   m_caller(NULL) {
   ReReader* reader = dynamic_cast<ReReader*>(unit->reader());
   m_caller = reader->source().caller();
}
/**
 * @brief Destructor
 */
ReSourcePosition::~ReSourcePosition() {
   // That should never occure!
   assert(false);
}

/**
 * @brief Placement new operator (for our own memory management).
 *
 * @param cbSize    size of the instance
 * @param buffer    buffer for the instance
 * @return          <code>buffer</code>
 */
void* ReSourcePosition::operator new(size_t, void* buffer) {
   return buffer;
}

/**
 * @brief Returns a description of the source position: "<unit>-<lineNo> (<col>):".
 *
 * @return a description of the instance
 */
ReString ReSourcePosition::toString() const {
   char buffer[512];
   utf8(buffer, sizeof buffer);
   return ReString(buffer);
}

/**
 * @brief Returns the position as a C string.
 *
 * @param buffer        OUT: the target buffer
 * @param bufferSize    the size of the buffer
 * @return              <code>buffer</code>
 */
char* ReSourcePosition::utf8(char buffer[], size_t bufferSize) const {
   qsnprintf(buffer, bufferSize, "%s:%d:%d",
             m_sourceUnit == NULL ? "" : m_sourceUnit->name(), m_lineNo, m_column);
   return buffer;
}

/**
 * @brief Returns the line number.
 * @return  the line number
 */
int ReSourcePosition::lineNo() const {
   return m_lineNo;
}

/**
 * @brief Sets the line number.
 *
 * @param lineNo    the new lineNo
 */
void ReSourcePosition::setLineNo(int lineNo) {
   m_lineNo = lineNo;
}
/**
 * @brief Returns the column.
 *
 * @return  the column of instance.
 */
int ReSourcePosition::column() const {
   return m_column;
}

/**
 * @brief Sets the column.
 *
 * @param column    the new column
 */
void ReSourcePosition::setColumn(int column) {
   m_column = column;
}

/**
 * @brief Returns the source unit belonging to the instance.
 *
 * @return  the source unit of the instance
 */
ReSourceUnit* ReSourcePosition::sourceUnit() const {
   return m_sourceUnit;
}

/**
 * @brief Sets the source unit.
 *
 * @param sourceUnit    the new source unit of the instance
 */
void ReSourcePosition::setSourceUnit(ReSourceUnit* sourceUnit) {
   m_sourceUnit = sourceUnit;
   m_lineNo = sourceUnit->lineNo();
}

/** @class ReReader ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Implements a base class for readers of different media.
 */

/**
 * @brief Constructor.
 *
 * @param source    the parent
 */
ReReader::ReReader(ReSource& source) :
   m_currentSourceUnit(NULL),
   m_units(),
   m_source(source) {
}

/**
 * @brief Destructor.
 */
ReReader::~ReReader() {
   clear();
}

/**
 * @brief Frees the resources.
 */
void ReReader::clear() {
   UnitMap::iterator it;
   for (it = m_units.begin(); it != m_units.end(); it++) {
      ReStringSourceUnit* unit = (ReStringSourceUnit*) (*it);
      delete unit;
   }
   m_units.clear();
   m_currentSourceUnit = NULL;
}

/**
 * @brief Returns the source of the reader.
 *
 * @return the parent, a source instance
 */
ReSource& ReReader::source() {
   return m_source;
}
/**
 * @brief Returns the current source unit.
 *
 * @return the source unit
 */
ReSourceUnit* ReReader::currentSourceUnit() const {
   return m_currentSourceUnit;
}

/**
 * @brief Sets the current source unit.
 *
 * @param sourceUnit    the name of the new source unit
 * @return              true: source unit exists<br>
 *                      false: source unit not found
 */
bool ReReader::setCurrentSourceUnit(ReSourceUnitName& sourceUnit) {
   bool rc = m_units.contains(sourceUnit);
   if (rc) {
      m_currentSourceUnit = m_units.value(sourceUnit);
      m_source.pushSourceUnit(m_currentSourceUnit);
   }
   return rc;
}

/**
 * @brief Removes the "latest" sourceUnit.
 */
void ReReader::removeSourceUnit() {
   m_currentSourceUnit = m_source.popSourceUnit(this);
   ;
}

/** @class ReSourcePositionBlock ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Efficient heap of <code>ReSourcePosition</code> instances.
 *
 * The <code>ReSourcePosition</code> heap is only growing. The deletion is
 * done for all entries together.
 * Therefore a simple allocation is possible with blocks.
 */
ReSourcePositionBlock::ReSourcePositionBlock() :
   m_successor(NULL)
// m_positions
{
   memset(m_positions, 0, sizeof m_positions);
}

/** @class ReSource ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Administrates a set of input sources with different readers.
 *
 * An input stream can be built by some different resources, e.g. files
 * and memory buffers. The ReSource can administrate these resources.
 */
/**
 * @brief Constructor.
 */
ReSource::ReSource() :
   m_sourcePositionStack(),
   m_sourcePositionBlock(NULL),
   m_countPositionBlock(RPL_POSITIONS_PER_BLOCK + 1),
   m_readers(),
   m_sourceUnits(),
   m_unitStack(),
   m_currentReader(NULL) {
   // the stack should never be empty:
   m_sourcePositionStack.push(NULL);
}

/**
 * @brief Destructor.
 */
ReSource::~ReSource() {
   destroy();
}

/**
 * @brief Frees the resources of the instance.
 */
void ReSource::destroy() {
   m_sourcePositionStack.clear();
   m_readers.clear();
   m_sourceUnits.clear();
   m_currentReader = NULL;
   ReSourcePositionBlock* block = m_sourcePositionBlock;
   m_sourcePositionBlock = NULL;
   m_countPositionBlock = RPL_POSITIONS_PER_BLOCK + 1;
   while (block != NULL) {
      ReSourcePositionBlock* last = block;
      block = block->m_successor;
      delete last;
   }
}

/**
 * @brief Returns the permanently valid source unit name.
 *
 * If unit names can be local objects (not string constants
 * like __FILE__) then this method must be overridden by
 * a method which builds a permanently valid string.
 *
 * @param unit      unit to find
 * @return          a permanently valid unit name
 */
ReSourceUnitName ReSource::permanentUnitName(ReSourceUnitName unit) {
   return unit;
}

/**
 * @brief Returns the stack with the positions of the open input resources.
 *
 * @return  the stack
 */
QStack<const ReSourcePosition*> ReSource::sourcePositionStack() const {
   return m_sourcePositionStack;
}

/**
 * @brief Returns the source unit stack.
 * @return  the stack of the source units
 */
QStack<ReSourceUnit*>& ReSource::sourceUnitStack() {
   return m_unitStack;
}

/**
 * @brief Adds a source reader.
 *
 * @param reader    the new reader. Will be freed in the destructor
 */
void ReSource::addReader(ReReader* reader) {
   m_readers.push_back(reader);
   if (m_currentReader == NULL)
      m_currentReader = reader;
}

/**
 * @brief Adds a source unit.
 *
 * @param unit  the new unit. Will be freed in the destructor
 */
void ReSource::addSourceUnit(ReSourceUnit* unit) {
   m_sourceUnits.push_back(unit);
}

/**
 * @brief Starts a new source unit.
 *
 * Saves the current source position onto the top of stack.
 * Pushes the source unit onto the top of stack.
 *
 * @param unit      the source unit
 * @param caller    the position of the include
 *
 */
bool ReSource::startUnit(ReSourceUnitName unit,
                         const ReSourcePosition& caller) {
   m_sourcePositionStack.push_back(&caller);
   ReReader* reader = NULL;
   QList<ReReader*>::iterator it;
   for (it = m_readers.begin(); reader == NULL && it != m_readers.end();
         it++) {
      ReReader* current = *it;
      if (current->openSourceUnit(unit)) {
         reader = current;
         m_currentReader = current;
         break;
      }
   }
   return reader != NULL;
}

/**
 * @brief Pushes a source unit onto the stack.
 *
 * @param unit      the source unit
 */
void ReSource::pushSourceUnit(ReSourceUnit* unit) {
   m_unitStack.push(unit);
}

/**
 * @brief Removes the latest source unit from the stack.
 *
 * @param reader    the current reader
 * @return          NULL: the current reader does not have an open source unit<br>
 *                  otherwise: the last entry from the source unit stack
 */
ReSourceUnit* ReSource::popSourceUnit(ReReader* reader) {
   ReSourceUnit* rc = NULL;
   if (m_unitStack.size() > 0)
      m_unitStack.pop();
   m_currentReader =
      m_unitStack.size() <= 0 ? NULL : m_unitStack.top()->reader();
   if (m_currentReader == reader)
      rc = m_unitStack.top();
   else {
      for (int ix = m_unitStack.size() - 2; ix >= 0; ix--) {
         if (m_unitStack[ix]->reader() == reader) {
            rc = m_unitStack[ix];
            break;
         }
      }
   }
   return rc;
}

/**
 * @brief Returns the reader of the current source unit.
 *
 * @return  NULL: no reader active<br>
 *          otherwise: the current reader
 */
ReReader* ReSource::currentReader() {
   return m_currentReader;
}

/**
 * @brief Returns a new instance of the current source position.
 *
 * The storage is done in a block (efficency).
 *
 * @param colNo     the column in the line
 * @return          a new instance of a source position
 */
const ReSourcePosition* ReSource::newPosition(int colNo) {
   if (m_countPositionBlock >= RPL_POSITIONS_PER_BLOCK) {
      ReSourcePositionBlock* newBlock = new ReSourcePositionBlock;
      newBlock->m_successor = m_sourcePositionBlock;
      m_sourcePositionBlock = newBlock;
      m_countPositionBlock = 0;
   }
   unsigned offset = m_countPositionBlock * sizeof(ReSourcePosition);
   m_countPositionBlock++;
   char* posInBlock = &m_sourcePositionBlock->m_positions[offset];
   ReSourceUnit* unit = dynamic_cast<ReSourceUnit*>(m_currentReader
                        ->currentSourceUnit());
   ReSourcePosition* rc = new (posInBlock) ReSourcePosition(unit,
         unit->lineNo(), colNo);
   return rc;
}

/**
 * @brief Resets all states in the source.
 */
void ReSource::clear() {
   destroy();
}

/**
 * @brief Returns the top position of the source unit stack.
 *
 * @return  NULL: stack is empty<br>
 *          the top of the source unit stack
 */
const ReSourcePosition* ReSource::caller() const {
   return
      m_sourcePositionStack.size() == 0 ? NULL : m_sourcePositionStack.top();
}

/** @class ReStringSourceUnit ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Stores the state of a string based source unit.
 *
 */

/**
 * @brief Constructor.
 *
 * @param name      name of the unit
 * @param content   content of the unit
 * @param reader    the parent
 */
ReStringSourceUnit::ReStringSourceUnit(ReSourceUnitName name,
                                       const ReSourceUnitContent& content, ReStringReader* reader) :
   ReSourceUnit(name, reader),
   m_currentPosition(0),
   m_content(content) {
}

/**
 * @brief Destructor.
 */
ReStringSourceUnit::~ReStringSourceUnit() {
}
/**
 * @brief Returns the current read position.
 *
 * @return      the offset (count of QChars) of the end of the last read block
 *              inside m_content
 */
int ReStringSourceUnit::currentPosition() const {
   return m_currentPosition;
}

/**
 * @brief Sets the current read position.
 *
 * @param currentPosition   the offset (count of QChars) of the end of
 *                          the last read block inside <code>m_content</code>
 */
void ReStringSourceUnit::setCurrentPosition(int currentPosition) {
   m_currentPosition = currentPosition;
}
/**
 * @brief Returns the content of the source unit.
 *
 * @return  the content
 */
ReSourceUnitContent ReStringSourceUnit::content() const {
   return m_content.constData();
}

/** @class ReStringReader ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Implements a source which provides reading from memory based buffers.
 *
 * Examples for usage: interactive input
 */

/**
 * @brief Constructor.
 *
 * @param source    the parent
 */
ReStringReader::ReStringReader(ReSource& source) :
   ReReader(source) {
}

/**
 * @brief Destructor.
 */
ReStringReader::~ReStringReader() {
   clear();
}

/**
 * @brief Opens a new source unit.
 *
 * @param unit  name of the source
 * @return      NULL: unknown source<br>
 *              otherwise: an instance of a sub class of
 *              <code>ReSourceUnit</code>
 */
ReSourceUnit* ReStringReader::openSourceUnit(ReSourceUnitName unit) {
   ReSourceUnit* rc = NULL;
   if (setCurrentSourceUnit(unit)) {
      rc = m_currentSourceUnit;
      ((ReStringSourceUnit*) rc)->setCurrentPosition(0);
   }
   return rc;
}
/**
 * @brief Delivers the next line from the input medium or the first part of it.
 *
 * @param maxSize   if the line length is longer than this value, only the
 *                  first part of the line will be returned
 * @param buffer    OUT: the line will be put here
 * @param hasMore   true: the line was longer than <code>maxSize</code>, only
 *                  the first part of the line is put into the <code>buffer</code>
 * @return          false: no more input available<br>
 *                  true: success
 */
bool ReStringReader::nextLine(int maxSize, QByteArray& buffer, bool& hasMore) {
   bool rc = m_currentSourceUnit != NULL;
   if (rc) {
      m_currentSourceUnit->setLineNo(m_currentSourceUnit->lineNo() + 1);
      rc = fillBuffer(maxSize, buffer, hasMore);
   }
   return rc;
}

/**
 * @brief Delivers the next part of a long line.
 *
 * @param maxSize   if the remaining line length is longer than this value,
 *                  only the a part of the line will be returned
 * @param buffer    OUT: the part of line will be put here
 * @param hasMore   true: the line was longer than <code>maxSize</code>, only
 *                  the first part of the line is put into the <code>buffer</code>
 * @return          false: no more input available<br>
 *                  true: success
 */
bool ReStringReader::fillBuffer(int maxSize, QByteArray& buffer,
                                bool& hasMore) {
   ReStringSourceUnit* unit = (ReStringSourceUnit*) m_currentSourceUnit;
   ReSourceUnitContent content = unit->content();
   int startPos = unit->currentPosition();
   const char* start = content + startPos;
   const char* end = strchr(start, '\n');
   hasMore = false;
   int size = end == NULL ? strlen(start) : end - start + 1;
   hasMore = false;
   if (size > maxSize) {
      size = maxSize;
      hasMore = true;
   }
   if (size > 0) {
      buffer.append(start, size);
      unit->setCurrentPosition(startPos + size);
   } else {
      removeSourceUnit();
   }
   return size > 0;
}

/**
 * @brief Adds a source buffer to the reader
 *
 * @param name      name of the medium
 * @param content
 */
void ReStringReader::addSource(ReSourceUnitName name,
                               ReSourceUnitContent content) {
   // Deletion in the destructor of the base class ReReader
   ReStringSourceUnit* unit = new ReStringSourceUnit(name, content, this);
   m_units.insert(m_units.begin(), unit->name(), unit);
   m_currentSourceUnit = unit;
}

/**
 * @brief Replaces the content of a source unit.
 *
 * @param name      name of the source unit
 * @param content   new content
 */
void ReStringReader::replaceSource(ReSourceUnitName name,
                                   ReSourceUnitContent content) {
   if (m_units.contains(name)) {
      ReStringSourceUnit* unit =
         dynamic_cast<ReStringSourceUnit*>(m_units[name]);
      unit->m_content = content;
   }
}

/** @class ReFileSourceUnit ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Stores the state of a file based source unit.
 *
 * This is the mostly used implementation of the ReSourceUnit/ReReader.
 */

ReFileSourceUnit::ReFileSourceUnit(ReSourceUnitName filename,
                                   ReFileReader* reader) :
   ReSourceUnit(filename, reader),
   m_currentPosition(0),
   m_fp(fopen(filename, "r")),
   m_textStream(m_fp, QIODevice::ReadOnly),
   m_line() {
}

/**
 * @brief Destructor.
 */
ReFileSourceUnit::~ReFileSourceUnit() {
   fclose(m_fp);
}

bool ReFileSourceUnit::isOpen() const {
   return m_fp != NULL;
}
/** @class ReFileReader ReSource.hpp "expr/ReSource.hpp"
 *
 * @brief Implements a source which provides reading from memory based buffers.
 *
 * Examples for usage: interactive input
 */

/**
 * @brief Constructor.
 */
ReFileReader::ReFileReader(ReSource& source) :
   ReReader(source) {
}

/**
 * @brief Destructor.
 */
ReFileReader::~ReFileReader() {
}

/**
 * @brief Opens a new source unit.
 *
 * @param unit  name of the source
 * @return      NULL: unknown source<br>
 *              otherwise: an instance of a sub class of
 *              <code>ReSourceUnit</code>
 */
ReSourceUnit* ReFileReader::openSourceUnit(ReSourceUnitName unit) {
   ReSourceUnit* rc = NULL;
   if (m_units.contains(unit)) {
      rc = *m_units.find(unit);
      m_currentSourceUnit = static_cast<ReFileSourceUnit*>(rc);
   }
   return rc;
}
/**
 * @brief Delivers the next line from the input medium or the first part of it.
 *
 * @param maxSize   if the line length is longer than this value, only the
 *                  first part of the line will be returned
 * @param buffer    OUT: the line will be put here
 * @param hasMore   true: the line was longer than <code>maxSize</code>, only
 *                  the first part of the line is put into the <code>buffer</code>
 * @return          false: no more input available<br>
 *                  true: success
 */
bool ReFileReader::nextLine(int maxSize, QByteArray& buffer, bool& hasMore) {
   ReFileSourceUnit* unit = static_cast<ReFileSourceUnit*>(m_currentSourceUnit);
   bool rc = !feof(unit->m_fp);
   if (!rc) {
      m_source.popSourceUnit(this);
   } else {
      m_currentSourceUnit->setLineNo(m_currentSourceUnit->lineNo() + 1);
      unit->m_currentPosition = 0;
      QByteArray& line = unit->m_line;
      line.reserve(maxSize + 1);
      if (fgets(line.data(), maxSize, unit->m_fp) == NULL)
         rc = false;
      else {
         line[maxSize] = '\0';
         line.resize(strlen(line.constData()));
         rc = fillBuffer(maxSize, buffer, hasMore);
      }
   }
   return rc;
}

/**
 * @brief Delivers the next part of a long line.
 *
 * @param maxSize   if the remaining line length is longer than this value,
 *                  only the a part of the line will be returned
 * @param buffer    OUT: the part of line will be put here
 * @param hasMore   true: the line was longer than <code>maxSize</code>, only
 *                  the first part of the line is put into the <code>buffer</code>
 * @return          false: no more input available<br>
 *                  true: success
 */
bool ReFileReader::fillBuffer(int maxSize, QByteArray& buffer, bool& hasMore) {
   ReFileSourceUnit* unit = static_cast<ReFileSourceUnit*>(m_currentSourceUnit);
   int start = unit->m_currentPosition;
   QByteArray& content = unit->m_line;
   int size = content.size() - start;
   if (size > maxSize)
      size = maxSize;
   buffer += content.mid(start, size);
   unit->m_currentPosition = (start += size);
   hasMore = start < content.size();
   return size > 0;
}

/**
 * @brief Adds a source file to the reader
 *
 * @param filename  the file' name (relative or absolute)
 */
void ReFileReader::addSource(ReSourceUnitName filename) {
   // Deleting in ~ReSourceUnit():
   ReFileSourceUnit* unit = new ReFileSourceUnit(filename, this);
   m_units.insert(m_units.begin(), unit->name(), unit);
   m_currentSourceUnit = unit;
}

