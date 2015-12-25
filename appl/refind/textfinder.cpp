/*
 * textfinder.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "utils.hpp"
#include "textfinder.hpp"

/**
 * Constructor.
 */
TextFinder::TextFinder() :
   m_ignoreBinary(false),
   m_filename(),
   m_length(0),
   m_file(),
   m_valid(false),
   m_regExpr(NULL),
   m_isRegExpr(false),
   m_ignoreCase(false),
   m_ownsRegExpr(false),
   m_text() {
}

TextFinder::TextFinder(const QString& fullName, int64_t length) :
   m_ignoreBinary(false),
   m_filename(fullName),
   m_length(length),
   m_file(fullName),
   m_valid(false),
   m_regExpr(NULL),
   m_isRegExpr(false),
   m_ignoreCase(false),
   m_ownsRegExpr(false),
   m_text() {
   m_valid = m_file.open(QIODevice::ReadOnly);
}

/**
 * Destructor.
 */
TextFinder::~TextFinder() {
   if (m_ownsRegExpr) {
      delete m_regExpr;
      m_regExpr = NULL;
   }
}

/**
 * Search a text pattern in the given file.
 *
 * @return              <code>true</code>: the pattern was found
 */
bool TextFinder::contains() {
   bool rc = false;
   if (!m_ignoreBinary || !isBinary()) {
      m_file.seek(0);
      QTextStream stream(&m_file);
      QString line;
      if (m_regExpr != NULL) {
         QRegularExpressionMatch match;
         while (!stream.atEnd()) {
            line = stream.readLine();
            match = m_regExpr->match(line);
            if (match.hasMatch()) {
               rc = true;
               break;
            }
         }
      } else {
         Qt::CaseSensitivity mode =
            m_ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive;
         while (!stream.atEnd()) {
            line = stream.readLine();
            if (line.indexOf(m_text, 0, mode) >= 0) {
               rc = true;
               break;
            }
         }
      }
   }
   return rc;
}

/**
 * Gets the search specific parameters from another instance.
 *
 * @param source    the source of the parameter
 */
void TextFinder::getSearchParameter(const TextFinder& source) {
   m_ignoreBinary = source.m_ignoreBinary;
   m_regExpr = source.m_regExpr;
   m_isRegExpr = source.m_isRegExpr;
   m_ignoreCase = source.m_ignoreCase;
   m_text = source.m_text;
}

/**
 * Tests whether the file is a binary file.
 *
 * @return  <code>true</code>: the file is a binary file
 */
bool TextFinder::isBinary() {
   QByteArray data = m_file.read(64 * 1024);
   int length = data.length();
   const char* ptr = reinterpret_cast <const char*>(memchr(data.constData(),
                     '\0', length));
   bool rc = ptr != NULL;
   return rc;
}

/**
 * Tests whether a string contains only text characters.
 *
 * @param data      data to inspect
 * @param trueAscii OUT: <code>true</code>: only 7-bit ASCII character are present
 * @return          <code>true</code>: the file is a text file
 */
bool TextFinder::isText(const QByteArray& data, bool* trueAscii) {
   const uint8_t* ptr = reinterpret_cast <const uint8_t*>(data.constData());
   bool isAscii = true;
   uint8_t byte1;
   bool rc = true;
   while ((byte1 = *ptr++) != '\0') {
      // ASCII with control chars:
      if (byte1 < ' ' && byte1 != '\t' && byte1 != '\n' && byte1 != '\r') {
         rc = false;
         break;
      }
      if (byte1 > 0x7f)
         isAscii = false;
   }
   if (trueAscii != NULL)
      *trueAscii = rc && isAscii;
   return rc;
}

/**
 * Tests whether the file is a binary file.
 *
 * @param data      data to inspect
 * @param trueAscii OUT: <code>true</code>: only 7-bit ASCII character are present
 * @return          <code>true</code>: the file is an UTF-8 file
 */
bool TextFinder::isUTF8(const QByteArray& data, bool* trueAscii) const {
   const uint8_t* ptr = reinterpret_cast <const uint8_t*>(data.constData());
   bool isAscii = true;
   uint8_t byte1;
   bool rc = true;
   while ((byte1 = *ptr++) != '\0') {
      // ASCII with control chars:
      if ((0x20 <= byte1 && byte1 <= 0x7E) || byte1 == '\t' || byte1 == '\n'
            || byte1 == '\r') {
         continue;
      }
      isAscii = false;
      uint8_t byte2 = *ptr++;
      // non-overlong 2-byte
      if (0xC2 <= byte1 && byte1 <= 0xDF && 0x80 <= byte2 && byte2 <= 0xBF) {
         continue;
      }
      uint8_t byte3 = *ptr++;
      // excluding overlongs
      if ((byte1 == 0xE0 && 0xA0 <= byte2 && byte2 <= 0xBF && 0x80 <= byte3
            && byte3 <= 0xBF)
            // straight 3-byte
            || (((0xE1 <= byte1 && byte1 <= 0xEC) || byte1 == 0xEE || byte1 == 0xEF)
                && 0x80 <= byte2 && byte2 <= 0xBF && 0x80 <= byte3 && byte3 <= 0xBF)
            // excluding surrogates
            || (byte1 == 0xED && (0x80 <= byte2 && byte2 <= 0x9F)
                && (0x80 <= byte3 && byte3 <= 0xBF))) {
         continue;
      }
      uint8_t byte4 = *ptr++;
      if ( // planes 1-3
         (byte1 == 0xF0 && 0x90 <= byte2 && byte2 <= 0xBF && 0x80 <= byte3
          && byte3 <= 0xBF && 0x80 <= byte4 && byte4 <= 0xBF)
         // planes 4-15
         || (0xF1 <= byte1 && byte1 <= 0xF3 && 0x80 <= byte2 && byte2 <= 0xBF
             && 0x80 <= byte3 && byte3 <= 0xBF && 0x80 <= byte4 && byte4 <= 0xBF)
         // plane 16
         || (byte1 == 0xF4 && 0x80 <= byte2 && byte2 <= 0x8F && 0x80 <= byte3
             && byte3 <= 0xBF && 0x80 <= byte4 && byte4 <= 0xBF)) {
         continue;
      }
      rc = false;
      break;
   }
   if (trueAscii != NULL)
      *trueAscii = rc && isAscii;
   return rc;
}
/**
 * Tests wether the file was opened correctly
 * @return      <code>true</code>: the file can be read
 */
bool TextFinder::isValid() const {
   return m_valid;
}

/**
 * Set the search parameters.
 *
 * @param text          the text to search
 * @param ignoreCase    <code>true</code>: the search is case insensitive
 * @param isRegExpr     <code>true</code>: the search uses a regular expression
 * @param ignoreBinary  <code>true</code>: binary files will not processed
 */
void TextFinder::setSearchParameter(const QString& text, bool ignoreCase,
                                    bool isRegExpr, bool ignoreBinary) {
   m_text = text;
   m_ignoreCase = ignoreCase;
   m_isRegExpr = isRegExpr;
   m_ignoreBinary = ignoreBinary;
   if (!isRegExpr || text.isEmpty()) {
      delete m_regExpr;
      m_regExpr = NULL;
   } else {
      QRegularExpression::PatternOption option =
         ignoreCase ?
         QRegularExpression::CaseInsensitiveOption :
         QRegularExpression::NoPatternOption;
      m_ownsRegExpr = true;
      m_regExpr = new QRegularExpression(text, option);
   }
}

/**
 * Returns the error text of a wrong regular expression.
 *
 * @return  "": the regular expression is correct.
 *          otherwise: the error message
 */
QString TextFinder::regExprError() {
   QString rc;
   if (m_regExpr != NULL && !m_regExpr->isValid())
      rc = m_regExpr->errorString();
   return rc;
}

