/*
 * ReSource.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef RPLSOURCE_HPP
#define RPLSOURCE_HPP

// type of buffer names and filenames. Codec: UTF-8
typedef const char* ReSourceUnitName;

typedef const char* ReSourceUnitContent;

class ReSource;
class ReReader;

class ReSourceUnit {
public:
   ReSourceUnit(const char* name, ReReader* reader);
   virtual ~ReSourceUnit();
public:
   const char* name() const;
   int lineNo() const;
   void setLineNo(int lineNo);
   ReReader* reader() const;

protected:
   QByteArray m_name;
   int m_lineNo;
   ReReader* m_reader;
};

class ReSourcePosition {
public:
   ReSourcePosition();
   ReSourcePosition(ReSourceUnit* unit, int lineNo, int colNo);
   ~ ReSourcePosition();
   void* operator new(size_t cbSize, void* buffer);
private:
   /// forbid usage of the copy constructor!
   ReSourcePosition(const ReSourcePosition& source);
   /// forbid usage of the the assignment!
   ReSourcePosition& operator=(const ReSourcePosition& source);
public:
   ReString toString() const;
   int lineNo() const;
   void setLineNo(int lineNo);

   int column() const;
   void setColumn(int column);

   ReSourceUnit* sourceUnit() const;
   void setSourceUnit(ReSourceUnit* sourceUnit);
   char* utf8(char buffer[], size_t bufferSize) const;
private:
   ReSourceUnit* m_sourceUnit;
   int m_lineNo;
   int m_column;
   const ReSourcePosition* m_caller;
};

class ReReader {
public:
   typedef ReCharPtrMap<ReSourceUnit*> UnitMap;
public:
   ReReader(ReSource& source);
   ~ReReader();
public:
   /**
    * @brief Prepares the reading from a given source unit.
    *
    * @param unit  name of the unit
    * @return      NULL: unit not known<br>
    *              otherwise: an instance with the state of the reader
    *              for the source. This is normally a sub class of
    *              <code>ReSourceUnit</code>
    */
   virtual ReSourceUnit* openSourceUnit(const char* unit) = 0;
   /**
    * @brief Reads the first part of the next line into a given buffer.
    *
    * @param maxSize   the maximum length of the read input.
    *                  If a line is longer the next part must be read
    *                  by <code>fillBuffer()</code>
    * @param buffer    IN/OUT: the read input will be appended here
    * @param hasMore   OUT: true: the line is longer than maxSize
    * @return          true: the read was successful<br>
    *                  false: no more input is available
    */
   virtual bool nextLine(int maxSize, QByteArray& buffer, bool& hasMore) = 0;
   /**
    * @brief Reads the next part of the current line into a given buffer.
    *
    * @param maxSize   the maximum length of the read input.
    * @param buffer    IN/OUT: the read input will be appended here
    * @param hasMore   OUT: true: the rest of line is longer than maxSize
    * @return          true: the read was successful<br>
    *                  false: no more input is available
    */
   virtual bool fillBuffer(int maxSize, QByteArray& buffer, bool& hasMore) = 0;
public:
   virtual void clear();
   ReSource& source();
   ReSourceUnit* currentSourceUnit() const;
   bool setCurrentSourceUnit(ReSourceUnitName& currentSourceUnit);
protected:
   void removeSourceUnit();

protected:
   ReSourceUnit* m_currentSourceUnit;
   /// name -> source
   UnitMap m_units;
   ReSource& m_source;
};

#define RPL_POSITIONS_PER_BLOCK 512
class ReSourcePositionBlock {
   friend class ReSource;
public:
   ReSourcePositionBlock();
private:
   ReSourcePositionBlock* m_successor;
   char m_positions[RPL_POSITIONS_PER_BLOCK* sizeof(ReSourcePosition)];
};

class ReSource {
public:
   ReSource();
   virtual ~ReSource();
public:
   virtual const char* permanentUnitName(const char* unit);
   void finishSourceUnit();
   void addReader(ReReader* reader);
   void addSourceUnit(ReSourceUnit* unit);
   QStack<const ReSourcePosition*> sourcePositionStack() const;
   QStack<ReSourceUnit*>& sourceUnitStack();

   bool startUnit(const char* unit, const ReSourcePosition& caller);
   void pushSourceUnit(ReSourceUnit* unit);
   ReSourceUnit* popSourceUnit(ReReader* reader);
   ReReader* currentReader();
   const ReSourcePosition* newPosition(int colNo);
   void clear();
   const ReSourcePosition* caller() const;
protected:
   void destroy();
protected:
   // stack of the info about the stacked (open) source units:
   QStack<const ReSourcePosition*> m_sourcePositionStack;
   ReSourcePositionBlock* m_sourcePositionBlock;
   int m_countPositionBlock;
   QList<ReReader*> m_readers;
   QList<ReSourceUnit*> m_sourceUnits;
   // setCurrentSourceUnit() pushes one entry, removeSourceUnit() pops it
   // (when end of input has been reached).
   QStack<ReSourceUnit*> m_unitStack;
   ReReader* m_currentReader;
};

class ReStringReader;

class ReStringSourceUnit: public ReSourceUnit {
   friend class ReStringReader;
public:
   ReStringSourceUnit(ReSourceUnitName name,
                      const ReSourceUnitContent& content, ReStringReader* reader);
   virtual ~ReStringSourceUnit();
public:
   int currentPosition() const;
   void setCurrentPosition(int currentPosition);
   ReSourceUnitContent content() const;

private:
   int m_currentPosition;
   QByteArray m_content;
};

class ReStringReader: public ReReader {
public:
   ReStringReader(ReSource& source);
   virtual ~ReStringReader();
   // ReReader interface
public:
   virtual ReSourceUnit* openSourceUnit(ReSourceUnitName unit);
   virtual bool nextLine(int maxSize, QByteArray& buffer, bool& hasMore);
   virtual bool fillBuffer(int maxSize, QByteArray& buffer, bool& hasMore);
public:
   void addSource(ReSourceUnitName name, ReSourceUnitContent content);
   void replaceSource(ReSourceUnitName name, ReSourceUnitContent content);
};

class ReFileReader;

class ReFileSourceUnit: public ReSourceUnit {
   friend class ReFileReader;
public:
   ReFileSourceUnit(ReSourceUnitName filename, ReFileReader* reader);
   virtual ~ReFileSourceUnit();
public:
   bool isOpen() const;
private:
   int m_currentPosition;
   FILE* m_fp;
   QTextStream m_textStream;
   QByteArray m_line;
};

class ReFileReader: public ReReader {
public:
   ReFileReader(ReSource& source);
   virtual ~ReFileReader();
   // ReReader interface
public:
   virtual ReSourceUnit* openSourceUnit(ReSourceUnitName unit);
   virtual bool nextLine(int maxSize, QByteArray& buffer, bool& hasMore);
   virtual bool fillBuffer(int maxSize, QByteArray& buffer, bool& hasMore);
public:
   void addSource(ReSourceUnitName filename);
};

#endif // RPLSOURCE_HPP
