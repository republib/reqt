/*
 * ReFile.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REFILE_HPP
#define REFILE_HPP

class ReUndoList {
public:
   enum UndoItemType {
      UIT_UNDEF,
      UIT_INSERT_PART,
      UIT_INSERT_LINES,
      UIT_JOIN,
      UIT_REMOVE_PART,
      UIT_REMOVE_LINES,
      UIT_SPLIT,
   };

   class UndoItem {
   public:
      UndoItemType m_type;
      int m_lineNo;
      int m_position;
      int m_length;
      QString m_string;
      ///@ true: the previous item belongs to this item (transaction)
      bool m_isPart;
   };

public:
   ReUndoList();
   ~ReUndoList();
public:
   bool checkSummarySize(qint64 stringSize);
   void clearUndo();
   qint64 maxUndoSize() const;
   UndoItem* pop();
   void setMaxUndoSize(qint64 maxUndoSize);
   void storeInsertPart(int lineNo, int pos, int count);
   void storeInsertLines(int lineNo, int count);
   void storeJoin(int lineNo, int length);
   void storeRemovePart(int lineNo, int pos, const QString& string);
   void storeRemoveLines(int lineNo, int count, const QStringList& list);
   void storeSplit(int lineNo, int pos);
protected:
   QList<UndoItem*> m_list;
   UndoItem* m_current;
   int m_lastLine;
   int m_lastPosition;
   qint64 m_maxUndoSize;
   qint64 m_currentUndoSize;
};

/**
 * Manages a list of lines.
 *
 * The lines will be stored without line terminators, e.g. '\n'.
 */
class ReLines: public ReUndoList, protected QStringList {
public:
   ReLines();
   virtual ~ReLines();
public:
   void clear();
   void insertLines(int lineNo, const QString& text, bool withUndo);
   void insertPart(int lineNo, int col, const QString& text, bool withUndo);
   void insertText(int lineNo, int col, const QString& text);
   bool joinLines(int first);
   /** Returns a line at a given position.
    * @param index  the index of the line (0..N-1)
    * @return       "": invalid index<br>
    *               otherwise: the wanted line
    */
   inline const QString& lineAt(int index) const {
      return index >= 0 && index < length() ? at(index) : m_empty;
   }
   /** Return the number of lines.
    * @return  the number of lines
    */
   inline
   int lineCount() const {
      return length();
   }
   virtual bool removePart(int lineNo, int pos, int count, bool withUndo);
   virtual void removeLines(int start, int count, bool withUndo);
   void splitLine(int lineNo, int col, bool withUndo);
   virtual void undo(int& lineNo, int& col);

protected:
   QString m_empty;
};

class ReLineSource {
public:
   virtual int hasMoreLines(int index) = 0;
};

class ReFile: public ReLineSource, public ReLines {
public:
   ReFile(const QString& filename, bool readOnly = true, ReLogger* logger =
             NULL);
   ~ReFile();
public:
   int64_t blocksize() const;
   void close();
   /**
    * Returns the current line number.
    * @return the current line number
    */
   inline uint32_t currentLineNo() const {
      return m_currentLineNo;
   }
   /** Returns the end of line separator.
    * @return   the end of line separator. Default: windows: "\r\n" linux: "\n"
    */
   const QByteArray& endOfLine() const {
      return m_endOfLine;
   }
   QString filename() const;
   bool findLine(const char* toFind, bool ignoreCase, int& lineNo,
                 QString* line);
   bool findLine(const QString& includePattern, bool includeIsRegExpr,
                 bool includeIgnoreCase, const QString& excludePattern,
                 bool excludeIsRegExpr, bool excludeIgnoreCase, int& lineNo,
                 QString* line);
   virtual int hasMoreLines(int index);
   char* nextLine(int& length);
   char* previousLine(int& length);
   bool read(const QString& filename = "");
   char* remap(int64_t offset, int size, int& length);
   void rewind();
   void setBlocksize(const int64_t& blocksize);
   /**Sets the end of line separator.
    * @param endOfLine  the end of line separator, usually "\n" or "\r\n"
    */
   void setEndOfLine(const char* endOfLine) {
      m_endOfLine = endOfLine;
   }
   void setFilename(const QString& filename);
   bool write(const QString& filename = "");

public:
   static QByteArray tempDir(const char* node, const char* parent = NULL,
                             bool withSeparator = true);
   static QByteArray tempFile(const char* node, const char* parent = NULL,
                              bool deleteIfExists = true);
   static QByteArray& readFromFile(const char* filename, QByteArray& buffer);
   static void writeToFile(const char* filename, const char* content,
                           size_t contentLength = (size_t) - 1, const char* mode = "w");
   static bool deleteTree(const QString& path, bool withBase,
                          ReLogger* logger);
private:
   QByteArray m_endOfLine;
   QString m_filename;
   QFile m_file;
   char* m_block;
   int64_t m_blocksize;
   int64_t m_blockOffset;
   int64_t m_filesize;
   char* m_startOfLine;
   int m_lineLength;
   int64_t m_lineOffset;
   uint32_t m_currentLineNo;
   int m_maxLineLength;
   QByteArray m_content;
   bool m_readOnly;
   ReLogger* m_logger;
};

#endif // REFILE_HPP
