/*
 * ReStringUtils.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RPLSTRING_HPP
#define RPLSTRING_HPP

#ifdef __linux__
#define strnicmp(trg, src, len) strncasecmp(trg, src, len)
#elif __WIN32__
#define strnicmp(trg, src, len) _strnicmp(trg, src, len)
#endif
class ReCharSet {
public:
   ReCharSet(const char* indexToChar, int* charToIndex,
             char minChar = 0, char maxChar = 0, int count = -1);
public:
   inline char charOf(int index) {
      return index >= 0 && index < m_count ? m_charOf[index] : 0;
   }
   inline char indexOf(char cc) {
      return cc < m_minChar || cc > m_maxChar ? -1 : m_indexOf[cc - m_minChar];
   }
   QByteArray dumpIndexOf();

public:
   static void getMinMax(const char* charSet, char& minChar, char& maxChar);
   static bool fillIndexOf(const char* charSet, char minChar, char maxChar,
                           int* indexOf, size_t sizeIndexOf);
protected:
   int m_count;
   char m_minChar;
   char m_maxChar;
   const char* m_charOf;
   int* m_indexOf;
   bool m_ownsCharOf;
};

class ReStringUtils {
public:
   static QByteArray& chomp(QByteArray& string, char cc = '\n');
   static int countChar(const char* line, char cc);
   static int count(const char* source, const char* item);
   static const QByteArray& cutString(const QByteArray& source, int maxLength,
                                      QByteArray& buffer, const char* appendix = "...");
   static const char* fileSeparator();
   static char fileSeparatorChar();
   static QByteArray hexDump(uint8_t* data, int length, int bytesPerLine = 16);
   static QByteArray hexDump(const void* data, int length, int bytesPerLine =
                                16) {
      return hexDump((uint8_t*) data, length, bytesPerLine);
   }
   static bool isInList(const char* phrase, const char* list, bool ignoreCase,
                        char separator = AUTO_SEPARATOR);
   static QByteArray read(const char* file, bool removeLastNewline = true);
   static QByteArray replaceNode(const char* source, const char* newNode);
   static bool write(const char* file, const char* content = NULL,
                     const char* mode = "w");
   static QList<QByteArray> toArray(const char* source, const char* separator);
   static QByteArray toCString(const char* source, int maxLength = -1);
   static QByteArray toNumber(int value, const char* format = "%d");
   static int lengthOfNumber(const char* text,
                             bool skipTrailingSpaces = false);
   static char findCsvSeparator(FILE* fp, char* buffer, size_t bufferSize);
   static int lengthOfUInt64(const char* text, int radix, quint64* pValue);
   static int lengthOfReal(const char* text, qreal* pValue);
   /** Returns the integer value of a hexadecimal digit.
    * @param cc	the digit to convert
    * @return		-1: invalid digit<br>
    *				otherwise: the integer value of the digit
    */
   inline static int valueOfHexDigit(char cc) {
      int rc = isdigit(cc) ? cc - '0'
               : (cc = tolower(cc)) >= 'A' && cc <= 'F' ? cc - 'A' + 10 : -1;
      return rc;
   }

public:
   static const QByteArray m_empty;
public:
   /** If used in <code>isInList()</code> the first character of the list
    * will be the separator.
    */
   static const char AUTO_SEPARATOR;
};

#endif // RPLSTRING_HPP
