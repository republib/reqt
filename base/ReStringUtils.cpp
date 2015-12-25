/*
 * ReStringUtils.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Missed operations for <code>QByteArray</code>s.
 */
/** @file base/ReStringUtils.cpp
 *
 * @brief Definitions for missed operations for <code>QByteArray</code>s.
 */
#include "base/rebase.hpp"

/** @class ReStringUtil ReStringUtils.hpp "base/ReStringUtils.hpp"
 *
 * @brief Implements some services around strings.
 *
 * This is a class with static members only.
 */

const char ReStringUtils::AUTO_SEPARATOR = '\0';

const QByteArray ReStringUtils::m_empty;

/**
 * Removes a given character from the end of the string if it is there.
 *
 * @param string	string to change
 * @param cc		character to remove. Default: end of line ('\n')<br>
 *					If <code>cc</code> is '\n' then also '\\r' will be removed
 *					at the end of the string
 * @return			<code>string</code>: for chaining
 */
QByteArray& ReStringUtils::chomp(QByteArray& string, char cc) {
   int length;
   if (string.length() > 0 && string.at(length = string.length() - 1) == cc) {
      string.resize(length);
      if (cc == '\n' && length > 0 && string.at(--length) == '\r')
         string.resize(length);
   }
   return string;
}
/**
 * @brief Counts the occurrences of a given char in a string.
 *
 * @param line	the text to inspect
 * @param cc	the char to count
 * @return		the number of <code>cc</code> in the text
 */

int ReStringUtils::countChar(const char* line, char cc) {
   const char* ptr = line;
   int rc = 0;
   while ((ptr = strchr(ptr, cc)) != NULL) {
      rc++;
      ptr++;
   }
   return rc;
}
/**
 * Counts the occurrences of a string in a string.
 *
 * @param source    in this string will be searched
 * @param item      this item will be searched
 * @return          the count of occurrences
 */
int ReStringUtils::count(const char* source, const char* item) {
   const char* end = source;
   int rc = 0;
   int lengthItem = strlen(item);
   while (true) {
      const char* start = end;
      end = strstr(start, item);
      if (end == NULL)
         break;
      else {
         rc++;
         end += lengthItem;
      }
   }
   return rc;
}

/**
 * Returns a string with a given maximum length.
 *
 * @param source        the source
 * @param maxLength     the maximum length of the result
 * @param buffer        Out: used if length of the result is shorter
 * @param appendix      if the result is cut this string will be appended.<br>
 *                      May be NULL.
 * @return              source: the source is enough short<br>
 *                      the prefix of source with the given length
 */
const QByteArray& ReStringUtils::cutString(const QByteArray& source,
      int maxLength, QByteArray& buffer, const char* appendix) {
   QByteArray& rc =
      source.length() <= maxLength ? (QByteArray&) source : buffer;
   if (source.length() > maxLength) {
      buffer = source.left(maxLength);
      if (appendix != NULL && appendix[0] != '\0')
         buffer.append(appendix);
   }
   return rc;
}
static char s_fileSeparator = 0;

/**
 * @brief Returns the os specific file path separator.
 * @return the file path separator, e.g. "/" for linux
 */
const char* ReStringUtils::fileSeparator() {
   return fileSeparatorChar() == '/' ? "/" : "\\";
}

/**
 * @brief Returns the os specific file path separator.
 * @return the file path separator, e.g. '/' for linux
 */
char ReStringUtils::fileSeparatorChar() {
   if (s_fileSeparator == 0) {
      const char* path = getenv("PATH");
      if (path != NULL) {
         s_fileSeparator =
            strchr(path, ';') != NULL || strchr(path, '\\') != NULL ?
            '\\' : '/';
      } else {
         if (getenv("windows") != NULL)
            s_fileSeparator = '\\';
         else
            s_fileSeparator = '/';
      }
   }
   return s_fileSeparator;
}

/** @brief Tests whether a phrase is in a phrase list.
 *
 * @param phrase		the word to search
 * @param list			the list to search. All phrases of the list are
 *						 separated by <code>separator</code>
 * @param ignoreCase	<code>true</code>: The search is case insensitive<br>
 *						<code>false</code>: The search is case sensitive
 * @param separator		the separator in <code>list</code>.
 *						If <code>AUTO_SEPARATOR</code> the separator will
 * 						be taken from the list itself (the first character)
 */
bool ReStringUtils::isInList(const char* phrase, const char* list,
                             bool ignoreCase, char separator) {
   if (separator == AUTO_SEPARATOR)
      separator = *list++;
   const char* end = strchr(list, separator);
   int phraseLength = strlen(phrase);
   bool rc = false;
   while (!rc && end != NULL) {
      if (end - list == phraseLength) {
         if (ignoreCase)
            rc = strnicmp(list, phrase, phraseLength) == 0;
         else
            rc = strncmp(list, phrase, end - list) == 0;
         if (rc)
            break;
      }
      list = end + 1;
      end = strchr(list, separator);
   }
   if (!rc) {
      if (ignoreCase)
         rc = strnicmp(list, phrase, end - list) == 0;
      else
         rc = strncmp(list, phrase, end - list) == 0;
   }
   return rc;
}

/**
 * Builds a hexadecimal dump.
 *
 * Format: a sequence of hex digits followed by the ascii interpretation.
 *
 * Example: "42 30 61  B0a"
 *
 * @param data              data to convert
 * @param length            length of data
 * @param bytesPerLine      one line containes so many bytes of data
 * @return                  the hex dump
 */
QByteArray ReStringUtils::hexDump(uint8_t* data, int length, int bytesPerLine) {
   QByteArray rc;
   int fullLines = length / bytesPerLine;
   int expectedLength = (bytesPerLine * 4 + 2) * (fullLines + 1);
   rc.reserve(expectedLength + 100);
   int ixData = 0;
   int col;
   char buffer[16];
   for (int lineNo = 0; lineNo < fullLines; lineNo++) {
      for (col = 0; col < bytesPerLine; col++) {
         qsnprintf(buffer, sizeof buffer, "%02x ", data[ixData + col]);
         rc.append(buffer);
      }
      rc.append(' ');
      for (col = 0; col < bytesPerLine; col++) {
         uint8_t cc = data[ixData + col];
         rc.append(cc > ' ' && cc < 128 ? (char) cc : '.');
      }
      ixData += bytesPerLine;
      rc.append('\n');
   }
   // incomplete last line:
   int restBytes = length - ixData;
   if (restBytes > 0) {
      for (col = 0; col < restBytes; col++) {
         qsnprintf(buffer, sizeof buffer, "%02x ", data[ixData + col]);
         rc.append(buffer);
      }
      for (col = restBytes; col < bytesPerLine; col++) {
         rc.append("   ");
      }
      rc.append(' ');
      for (col = 0; col < restBytes; col++) {
         uint8_t cc = data[ixData + col];
         rc.append(cc > ' ' && cc < 128 ? (char) cc : '.');
      }
      rc.append('\n');
   }
   return rc;
}

/**
 * Reads a file into a string.
 *
 * @param file              file to read
 * @param removeLastNewline true: if the last character is a newline
 *                          the result will not contain this
 * @return                  the file's content
 */
QByteArray ReStringUtils::read(const char* file, bool removeLastNewline) {
   QByteArray rc;
   struct stat info;
   size_t size;
   if (stat(file, &info) == 0 && (size = info.st_size) > 0) {
      FILE* fp = fopen(file, "r");
      if (fp != NULL) {
         rc.resize(info.st_size);
         fread(rc.data(), 1, size, fp);
         fclose(fp);
         if (removeLastNewline && rc.at(size - 1) == '\n') {
            rc.resize(size - 1);
         }
      }
   }
   return rc;
}

QByteArray ReStringUtils::replaceNode(const char* source, const char* newNode) {
   char sep = fileSeparatorChar();
   const char* ptr = strrchr(source, sep);
   QByteArray rc;
   rc.reserve(strlen(source) + strlen(newNode) + 1);
   if (ptr == NULL) {
      rc.append(source).append(sep).append(newNode);
   } else if (ptr[0] == '\0') {
      rc.append(source).append(newNode);
   } else {
      rc.append(source, ptr - source + 1).append(newNode);
   }
   return rc;
}

/**
 * Converts a string into an array of strings.
 *
 * @param source        string to convert
 * @param separator     the separator between the items to split
 * @return              an array with the splitted source
 */
QList<QByteArray> ReStringUtils::toArray(const char* source,
      const char* separator) {
   const char* end = source;
   QList < QByteArray > rc;
   rc.reserve(count(source, separator) + 1);
   int lengthItem = strlen(separator);
   while (*end != '\0') {
      const char* start = end;
      end = strstr(start, separator);
      if (end == NULL) {
         end = start + strlen(start);
      }
      rc.append(QByteArray(start, end - start));
      if (end[0] != '\0')
         end += lengthItem;
   }
   return rc;
}

QByteArray ReStringUtils::toCString(const char* source, int maxLength) {
   if (maxLength <= 0)
      maxLength = strlen(source);
   int binaries = 0;
   int ix;
   for (ix = 0; ix < maxLength; ix++)
      if (source[ix] < ' ') {
         binaries++;
      }
   QByteArray rc;
   rc.reserve(maxLength + 3 * binaries + 1);
   char cc;
   for (ix = 0; ix < maxLength; ix++)
      if ((cc = source[ix]) >= ' ') {
         rc += source[ix];
      } else {
         switch (cc) {
         case '\0':
            // stop looping:
            ix = maxLength;
            break;
         case '\n':
            rc += "\\n";
            break;
         case '\r':
            rc += "\\r";
            break;
         case '\t':
            rc += "\\t";
            break;
         default: {
            char buffer[5];
            qsnprintf(buffer, sizeof buffer, "\\x%02x",
                      ((unsigned int) cc) % 0xff);
            rc += buffer;
            break;
         }
         }
      }
   return rc;
}

/**
 * Return an integer as an QByteArray.
 *
 * @param value     value to convert
 * @param format    format like in sprintf()
 * @return          the ascii form of the value
 */
QByteArray ReStringUtils::toNumber(int value, const char* format) {
   char buffer[128];
   qsnprintf(buffer, sizeof buffer, format, value);
   return QByteArray(buffer);
}

/**
 * Writes a string to a file.
 *
 * @param file      the file's name
 * @param content   NULL or the file's content
 * @param mode      the file open mode: "w" for write, "a" for append
 * @return          true: successful<br>
 *                  false: error occurred
 */
bool ReStringUtils::write(const char* file, const char* content,
                          const char* mode) {
   FILE* fp = fopen(file, mode);
   if (fp != NULL) {
      fputs(content, fp);
      fclose(fp);
   }
   return fp != NULL;
}
/**
 * @brief Returns the length of the number string.
 *
 * @param text                  a text to inspect
 * @param skipTrailingSpaces    true: if spaces are behind the number
 *                              the result contains the length of these
 * @return		0: not a number<br>
 * 				otherwise: the length of the number string
 */
int ReStringUtils::lengthOfNumber(const char* text, bool skipTrailingSpaces) {
   int rc = 0;
   bool found = false;
   const char* ptr = text;
   while (isspace(*ptr))
      ptr++;
   if ((*ptr == '+' || *ptr == '-'))
      ptr++;
   found = isdigit(*ptr);
   while (isdigit(*ptr)) {
      ptr++;
   }
   if (*ptr == '.') {
      ptr++;
      if (isdigit(*ptr)) {
         found = true;
         while (isdigit(*ptr))
            ptr++;
      }
   }
   if (found && toupper(*ptr) == 'E') {
      const char* ptrToE = ptr;
      ptr++;
      if (*ptr == '+' || *ptr == '-')
         ptr++;
      if (!isdigit(*ptr))
         ptr = ptrToE;
      else {
         while (isdigit(*ptr))
            ptr++;
      }
   }
   if (found && skipTrailingSpaces) {
      while (isspace(*ptr)) {
         ptr++;
      }
   }
   rc = !found ? 0 : ptr - text;
   return rc;
}
/**
 * @brief Adds the count of the possible separators.
 *
 * @param countCommas		IN/OUT: number of ','
 * @param countSemicolons	IN/OUT: number of ';'
 * @param countPipes		IN/OUT: number of '|'
 * @param countBlanks		IN/OUT: number of ' '
 */
static void addSeparators(const char* line, int& commas, int& semicolons,
                          int& pipes, int& blanks) {
   commas += ReStringUtils::countChar(line, ',');
   semicolons += ReStringUtils::countChar(line, ';');
   pipes += ReStringUtils::countChar(line, '|');
   blanks += ReStringUtils::countChar(line, ' ');
}

/**
 * @brief Finds the separator of the CSV file.
 *
 * If the file contain TABs the result is TAB.
 * If not:
 * Inspects the first 5 lines and counts the possible separators.
 * The most found separator will be returned.
 *
 * @param fp			CSV file
 * @param buffer		a line buffer
 * @param bufferSize	the size of <code>buffer[]</code>
 */
char ReStringUtils::findCsvSeparator(FILE* fp, char* buffer, size_t bufferSize) {
   char rc = '\0';
   int lineNo = 0;
   int maxLines = 5;
   const char* line;
   int commas = 0;
   int semicolons = 0;
   int pipes = 0;
   int blanks = 0;
   while (++lineNo < maxLines && (line = fgets(buffer, bufferSize, fp)) != NULL) {
      if (strchr(line, '\t') != NULL) {
         rc = '\t';
         break;
      }
      addSeparators(line, commas, semicolons, pipes, blanks);
   }
   fseek(fp, 0, SEEK_SET);
   if (rc != '\t') {
      if (semicolons > 0 && commas > 0) {
         // if ',' is decimal separator and ';' is the column separator:
         // Add one semicolon per line because of number of values is
         // 1 greater than the number of separators
         semicolons += lineNo;
      }
      if (commas + semicolons + pipes == 0) {
         rc = blanks > 0 ? ' ' : '\0';
      } else if (semicolons >= commas && semicolons >= pipes)
         rc = ';';
      else if (commas > semicolons && commas > pipes)
         rc = ',';
      else if (pipes > commas && pipes > semicolons)
         rc = '|';
   }
   return rc;
}

/**
 * @brief Determines the length and vlaue of an integer.
 *
 * @param text      the number as text
 * @param radix     the base of the number system: 8 (octal), 10 or 16
 * @param pValue    OUT: the value of the integer. May be NULL
 *
 * @return          <=0: no integer found
 *                  otherwise: the length of the integer
 */
int ReStringUtils::lengthOfUInt64(const char* text, int radix, quint64* pValue) {
   int64_t value = 0;
   int length = 0;
   int cc;
   if (radix == 10) {
      while ((cc = text[length]) >= '0' && cc <= '9') {
         value = value * 10 + cc - '0';
         length++;
      }
   } else if (radix == 16) {
      while (true) {
         if ((cc = text[length]) >= '0' && cc <= '9')
            value = value * 16 + cc - '0';
         else if (cc >= 'A' && cc <= 'F')
            value = value * 16 + cc - 'A' + 10;
         else if (cc >= 'a' && cc <= 'f')
            value = value * 16 + cc - 'a' + 10;
         else
            break;
         length++;
      }
   } else if (radix == 8) {
      while (true) {
         if ((cc = text[length]) >= '0' && cc <= '7')
            value = value * 8 + cc - '0';
         else
            break;
         length++;
      }
   } else {
      throw ReException("ReStringUtil::lengthOfInt(): wrong radix: %d",
                        radix);
   }
   if (pValue != NULL)
      *pValue = value;
   return length;
}

/**
 * @brief Determines the length and value of a floting point number.
 *
 * @param text      the number as text
 * @param pValue     OUT: the value of the integer. May be NULL
 *
 * @return          <=0: no real number found
 *                  otherwise: the length of the floating point number
 */
int ReStringUtils::lengthOfReal(const char* text, qreal* pValue) {
   qreal value = 0.0;
   int cc;
   int length = 0;
   while (true) {
      if ((cc = text[length]) >= '0' && cc <= '9')
         value = value * 10 + (cc - '0');
      else
         break;
      length++;
   }
   // found: a digit has been found (in front of or behind the '.'
   bool found = length > 0;
   if (text[length] == '.') {
      length++;
   }
   if (isdigit(text[length])) {
      found = true;
      qreal divisor = 1;
      qreal precision = 0;
      while ((cc = text[length]) >= '0' && cc <= '9') {
         divisor *= 10;
         precision = precision * 10 + cc - '0';
         length++;
      }
      value += precision / divisor;
   } else if (!found) {
      length = 0;
   }
   if (found && toupper(text[length]) == 'E') {
      int savePoint = length;
      length++;
      bool negative = false;
      if ((cc = text[length]) == '+')
         length++;
      else if (cc == '-') {
         length++;
         negative = true;
      }
      if (!isdigit(text[length]))
         length = savePoint;
      else {
         int exponent = 0;
         while (isdigit(text[length])) {
            exponent = exponent * 10 + text[length] - '0';
            length++;
         }
         if (negative)
            value /= qPow(10, exponent);
         else
            value *= qPow(10, exponent);
      }
   }
   if (pValue)
      *pValue = value;
   return found ? length : 0;
}

/**
 * Constructor.
 *
 * @param charOf	a string containing all member characters of the char set.
 * @param indexOf	an array containing the index of the char at the position
 *					of the char, starting with the minimum character
 * @param minChar	the lowest character of the set
 * @param maxChar	the highest character of the set
 * @param count		-1: strlen(charOf) is taken<br>
 *
 *					otherwise: the length of charOf[]
 */
ReCharSet::ReCharSet(const char* charOf, int* indexOf,
                     char minChar, char maxChar, int count) :
   m_count(count < 0 ? strlen(charOf) : count),
   m_minChar(minChar),
   m_maxChar(maxChar),
   m_charOf(charOf),
   m_indexOf(indexOf),
   m_ownsCharOf(false) {
   if (minChar == 0 && maxChar == 0)
      getMinMax(m_charOf, m_minChar, m_maxChar);
   if (indexOf == NULL) {
      int length = m_maxChar - m_minChar + 1;
      m_indexOf = new int[length];
      m_ownsCharOf = true;
      fillIndexOf(m_charOf, m_minChar, m_maxChar, m_indexOf,
                  length * sizeof *m_indexOf);
   }
}

/**
 * Writes the definition of indexOf into a string.
 *
 * @return the indexOf array as C text
 */
QByteArray ReCharSet::dumpIndexOf() {
   QByteArray rc;
   int length = m_maxChar - m_minChar + 1;
   rc.reserve(100 + length * 10);
   rc.append("// minChar = ").append(QByteArray::number(m_minChar)).append('\n');
   rc.append("// maxChar = ").append(QByteArray::number(m_maxChar)).append('\n');
   rc.append("int* indexOf[] = {\n");
   for (int ix = 0; ix < length; ix++) {
      rc.append(QString::number(m_indexOf[ix])).append(',');
      if (m_indexOf[ix] >= 0)
         rc.append("\t// '").append(m_charOf[m_indexOf[ix]]).append('\'');
      rc.append('\n');
   }
   rc.append("};\n");
   return rc;
}

/**
 * Finds the minimum and the maximum char of an ascii string.
 *
 * The minimum of "badc" is 'a', the maximum is 'd'.
 *
 * @param charSet	a string with the characters to inspect
 * @param minChar	OUT: the minimum character
 * @param maxChar	OUT: the maximum character
 */
void ReCharSet::getMinMax(const char* charSet, char& minChar, char& maxChar) {
   minChar = maxChar = charSet[0];
   while(*charSet != '\0') {
      if (*charSet < minChar)
         minChar = *charSet;
      else if (*charSet > maxChar)
         maxChar = *charSet;
      ++charSet;
   }
}

/**
 * Calculates the indexOf table from the character set.
 *
 * Invalid character positions will be set to -1.
 * For all valid characters of the set is:
 * <code>assert(indexOf[charset[ix] - minChar] == ix
 * && charSet[indexOf[cc - minChar] == cc)</code>
 *
 * @param charSet		the character set
 * @param minChar		the character of the set with the minimal value:
 *						If 0 and maxChar == 0: the values will be calculated
 * @param maxChar		the character of the set with the maximal value
 * @param indexOf		OUT: the table to fill
 * @param sizeIndexOf	the size of <code>indexOf</code>
 * @return				<code>true</code>: success<br>
 *						<code>false</code>: minChar or maxChar invalid or wrong size
 */
bool ReCharSet::fillIndexOf(const char* charSet, char minChar, char maxChar,
                            int* indexOf, size_t sizeIndexOf) {
   bool rc = true;
   int length = maxChar - minChar + 1;
   if (length != int(sizeIndexOf / sizeof*indexOf))
      rc = false;
   else {
      int ix = 0;
      memset(indexOf, -1, sizeIndexOf);
      while(*charSet != '\0') {
         char cc = *charSet++;
         if (cc < minChar || cc > maxChar)
            rc = false;
         else
            indexOf[cc - minChar] = ix;
         ix++;
      }
   }
   return rc;
}

