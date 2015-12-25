/*
 * ReQStringUtils.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
I18N::CharSet I18N::m_standardCharSet = I18N::SYSTEM;

const QStringList ReQStringUtils::m_emptyList;
const QString ReQStringUtils::m_empty;

/** @file
 * @brief Missed operation for <code>QString</code>s.
 */
/** @file base/ReQStringUtils.hpp
 *
 * @brief Definitions for missed operation for <code>QString</code>s.
 */
#include "base/rebase.hpp"

/**
 * Removes end of line characters if any.
 *
 * @param text  text to inspect
 * @param cc	character to remove. If '\\n' also '\\r' will be removed
 * @return      <code>text</code> without trailing <code>cc</code>
 */
ReString ReQStringUtils::chomp(const ReString& text, char cc) {
   int last = text.length() - 1;
   if (last < 0)
      return text;
   else {
      if (cc != '\n') {
         return text.at(last) == cc ? text.mid(0, last) : text;
      } else {
         while (last >= 0 && (text[last] == '\n' || text[last] == '\r')) {
            last--;
         }
         return last == text.length() - 1 ? text : text.left(last + 1);
      }
   }
}

/**
 * Counts the occurencies of a character in a string.
 *
 * @param value		string to inspect
 * @param toFind	character to find
 * @param start		first index to search
 *
 * @return the count of occurrencies of the character
 */
int ReQStringUtils::countOf(const QString& value, QChar toFind, int start) {
   int rc = 0;
   if (start >= 0) {
      for (int ix = start; ix < value.length(); ix++)
         if (value.at(ix) == toFind)
            rc++;
   }
   return rc;
}

/**
 * Tests whether a given character is the last of the string and append it if not.
 *
 * @param value     string to test
 * @param lastChar  the last character
 */
QString& ReQStringUtils::ensureLastChar(QString& value, QChar lastChar) {
   if (value.isEmpty() || value.at(value.length() - 1) != lastChar)
      value += lastChar;
   return value;
}

/**
 * @brief Determines the length and value of an integer.
 *
 * @param text      the number as text
 * @param start     the first index to inspect
 * @param radix     the base of the number sytem: 8 (octal), 10 or 16
 * @param pValue    OUT: the value of the integer. May be NULL
 *
 * @return          &lt;=0: no integer found<br>
 *                  otherwise: the length of the integer
 */
int ReQStringUtils::lengthOfUInt64(const ReString& text, int start, int radix,
                                   uint64_t* pValue) {
   int inputLength = text.size();
   int64_t value = 0;
   int ix = start;
   int cc;
   if (radix == 10) {
      while (ix < inputLength) {
         if ((cc = text[ix].unicode()) >= '0' && cc <= '9')
            value = value * 10 + cc - '0';
         else
            break;
         ix++;
      }
   } else if (radix == 16) {
      while (ix < inputLength) {
         if ((cc = text[ix].unicode()) >= '0' && cc <= '9')
            value = value * 16 + cc - '0';
         else if (cc >= 'A' && cc <= 'F')
            value = value * 16 + cc - 'A' + 10;
         else if (cc >= 'a' && cc <= 'f')
            value = value * 16 + cc - 'a' + 10;
         else
            break;
         ix++;
      }
   } else if (radix == 8) {
      while (ix < inputLength) {
         if ((cc = text[ix].unicode()) >= '0' && cc <= '7')
            value = value * 8 + cc - '0';
         else
            break;
         ix++;
      }
   } else {
      throw ReException("ReQStringUtil::lengthOfInt(): wrong radix: %d",
                        radix);
   }
   if (pValue != NULL)
      *pValue = value;
   return ix - start;
}
/**
 * @brief Determines the length and value of an unsigned integer.
 *
 * @param text      the number as text
 * @param start     the first index to inspect
 * @param radix     the base of the number sytem: 8 (octal), 10 or 16
 * @param pValue     OUT: the value of the integer. May be NULL
 *
 * @return          0: no integer found
 *                  otherwise: the length of the integer
 */
int ReQStringUtils::lengthOfUInt(const ReString& text, int start, int radix,
                                 uint* pValue) {
   quint64 value;
   int rc = lengthOfUInt64(text, start, radix, &value);
   if (pValue != NULL)
      *pValue = (uint) value;
   return rc;
}

/**
 * Returns the length of a date in a string.
 *
 * The syntax of a time is 'dd.mm.yyyy' or 'yyyy.mm.dd'.
 *
 * @param text  text to inspect
 * @param start the first index in <code>text</code> to inspect
 * @param value OUT: the value of the found date. Not changed if result is 0.<br>
 *              May be NULL
 * @return      0: no date found<br>
 *              otherwise: the length of the date in the string
 */
int ReQStringUtils::lengthOfDate(const ReString& text, int start,
                                 QDate* value) {
   uint day = 0;
   uint month = 0;
   uint year = 0;
   int length = lengthOfUInt(text, start, 10, &year);
   switch (length) {
   case 1:
   case 2:
      day = year;
      year = 0;
      break;
   case 4:
      break;
   default:
      length = 0;
      break;
   }
   int length2;
   start += length;
   skipExpected(text, '.', start, length);
   if (length > 0) {
      length2 = lengthOfUInt(text, start, 10, &month);
      if (length2 < 1 || length2 > 2)
         length = 0;
      else {
         start += length2;
         length += length2;
      }
   }
   skipExpected(text, '.', start, length);
   if (year > 0) {
      length2 = lengthOfUInt(text, start, 10, &day);
      if (length2 < 1 || length2 > 2)
         length = 0;
      else {
         start += length2;
         length += length2;
      }
   } else {
      length2 = lengthOfUInt(text, start, 10, &year);
      if (length2 != 4)
         length = 0;
      else {
         start += length2;
         length += length2;
      }
   }
   if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1970
         || year > 2100)
      length = 0;
   if (length != 0 && value != NULL)
      *value = QDate(year, month, day);
   return length;
}

/**
 * Returns the length of a date and/or time in a string.
 *
 * @param text          text to inspect
 * @param start         the first index in <code>text</code> to inspect
 * @param allowDateOnly <code>false</code>: if the date is not followed by
 *                      a time the result will be 0
 * @param allowTimeOnly <code>false</code>: if no date is found at the given
 *                      text position the result will be 0
 * @param value         the value of the found date. Not changed if result is 0.<br>
 *                      May be NULL
 * @return              0: no date found<br>
 *                      otherwise: the length of the date in the string
 */
int ReQStringUtils::lengthOfDateTime(const ReString& text, int start,
                                     bool allowDateOnly, bool allowTimeOnly, QDateTime* value) {
   QDate date;
   QTime time;
   int length = lengthOfDate(text, start, &date);
   if (length == 0) {
      if (allowTimeOnly) {
         date = QDate::currentDate();
         length = lengthOfTime(text, start, &time);
      }
   } else {
      if (start + length + 1 + 3 <= text.length()) {
         start += length;
         int length2 = 0;
         if (!text[start].isDigit()) {
            QTime time2;
            length2 = lengthOfTime(text, start + 1, &time2);
            if (length2 == 0 && !allowDateOnly)
               length = 0;
            else if (length2 > 0) {
               length += 1 + length2;
               time = time2;
            }
         }
      }
   }
   if (length > 0 && value != NULL)
      *value = QDateTime(date, time);
   return length;
}
/**
 * Returns the length of a time in a string.
 *
 * The syntax of a time is hh:mm[:ss]
 *
 * @param text  text to inspect
 * @param start the first index in <code>text</code> to inspect
 * @param value OUT: the value of the found time. Not changed if result is 0.<br>
 *              May be NULL
 * @return  0: no date found<br>
 *          otherwise: the length of the date in the string
 */
int ReQStringUtils::lengthOfTime(const ReString& text, int start,
                                 QTime* value) {
   uint hour = 0;
   uint minute = 0;
   uint sec = 0;
   int length = lengthOfUInt(text, start, 10, &hour);
   if (length > 0 && hour > 23)
      length = 0;
   if (length > 0) {
      start += length;
   }
   int length2;
   skipExpected(text, ':', start, length);
   if (length > 0) {
      length2 = lengthOfUInt(text, start, 10, &minute);
      if (length2 < 1 || length2 > 2 || minute >= 60)
         length = 0;
      else
         start += length2, length += length2;
   }
   if (length > 0 && start < text.length() && text[start] == ':') {
      length++;
      start++;
      length2 = lengthOfUInt(text, start, 10, &sec);
      if (length2 < 1 || length2 > 2 || sec >= 60)
         length = 0;
      else
         start += length2, length += length2;
   }
   if (length != 0 && value != NULL)
      *value = QTime(hour, minute, sec);
   return length;
}

/**
 * @brief Determines the length and value of a floting point number.
 *
 * @param text      the number as text
 * @param start     the first index to inspect
 * @param pValue     OUT: the value of the integer. May be NULL
 *
 * @return          <=0: no real number found
 *                  otherwise: the length of the floating point number
 */
int ReQStringUtils::lengthOfReal(const ReString& text, int start,
                                 qreal* pValue) {
   int inputLength = text.size();
   qreal value = 0.0;
   int cc;
   int ix = start;
   while (ix < inputLength) {
      if ((cc = text[ix].unicode()) >= '0' && cc <= '9')
         value = value * 10 + (cc - '0');
      else
         break;
      ix++;
   }
   // found: a digit has been found (in front of or behind the '.'
   bool found = ix > start;
   if (ix < inputLength && text[ix].unicode() == '.') {
      ix++;
   }
   if (ix < inputLength && text[ix].isDigit()) {
      found = true;
      qreal divisor = 1;
      qreal precision = 0;
      while (ix < inputLength && (cc = text[ix].unicode()) >= '0' && cc <= '9') {
         divisor *= 10;
         precision = precision * 10 + cc - '0';
         ix++;
      }
      value += precision / divisor;
   } else if (!found) {
      ix = start;
   }
   if (found && ix + 1 < inputLength && toupper(text[ix].unicode()) == 'E') {
      int savePoint = ix;
      ix++;
      bool negative = false;
      if ((cc = text[ix].unicode()) == '+')
         ix++;
      else if (cc == '-') {
         ix++;
         negative = true;
      }
      if (ix >= inputLength || !text[ix].isDigit())
         ix = savePoint;
      else {
         int exponent = 0;
         while (ix < inputLength && text[ix].isDigit()) {
            exponent = exponent * 10 + text[ix].unicode() - '0';
            ix++;
         }
         if (negative)
            value /= qPow(10, exponent);
         else
            value *= qPow(10, exponent);
      }
   }
   if (pValue)
      *pValue = value;
   return found ? ix - start : 0;
}

/**
 * Returns the longest common prefix of a string list.
 *
 * @param list	list to inspect
 *
 * @return the longest prefix which is common to all list entries
 */
QString ReQStringUtils::longestPrefix(const QStringList& list) {
   QString rc;
   if (list.length() > 0) {
      rc.reserve(max(32, list.at(0).length()));
      QChar common;
      bool found = true;
      for (int ixPrefix = 0; found; ixPrefix++) {
         if (ixPrefix >= list.at(0).length())
            found = false;
         else {
            common = list.at(0).at(ixPrefix);
            for (int ixList = 1; found && ixList < list.length(); ++ixList) {
               if (list.at(ixList).length() <= ixPrefix
                     || list.at(ixList).at(ixPrefix) != common)
                  found = false;
            }
            if (found)
               rc.append(common);
         }
      }
   }
   return rc;
}

/**
 * Replaces placeholders by their values.
 *
 * Example for a placeholder: ${path}
 *
 * @param text          IN/OUT: the text to inspect/change
 * @param placeholders  a hashmap with (name, value) pairs, e.g. ("path", "/")
 * @param error         OUT: NULL or the error message (unknown name)
 * @return              <code>true</code>: success<br>
 *                      <code>false</code>: unknown name found (not replaced)
 */
bool ReQStringUtils::replacePlaceholders(QString& text,
      const QMap<QString, QString>& placeholders, QString* error) {
   int start = 0;
   bool rc = true;
   QString name;
   QMap<QString, QString>::const_iterator it;
   while (start >= 0) {
      start = text.indexOf("${", start);
      if (start < 0)
         break;
      int end = text.indexOf('}', start + 1);
      if (end < 0)
         break;
      name = text.mid(start + 2, end - start - 2);
      it = placeholders.find(name);
      if (it == placeholders.end()) {
         rc = false;
         if (error != NULL) {
            *error = QObject::tr("unknown placeholder: ") + name;
         }
      } else {
         text = text.replace("${" + name + "}", *it);
      }
      start += (*it).length();
   }
   return rc;
}

/**
 * Skips a character in a text at a given position if it has an expected value.
 *
 * @param text      text to inspect
 * @param expected  the character which is expected
 * @param index     IN/OUT: the position of the expected character.
 *                  Will be incremented if the expected character is found
 * @param length    IN/OUT: IN: 0: do nothing<br>
 *                  OUT: 0: the expected character was not found.
 *                  otherwise: the length is incremented
 */
void ReQStringUtils::skipExpected(const ReString& text, QChar expected,
                                  int& index, int& length) {
   if (length == 0) {
      // error state, do nothing
   } else if (index >= text.length() || text[index] != expected) {
      length = 0;
   } else {
      index++;
      length++;
   }
}

/**
 * @brief Converts a ReString into an utf-8 string
 *
 * The expression <code>qstring.toUtf8().constData()</code> is not allowed
 * in a variable argument list like sprintf. This is a thread save workaround.
 *
 * @param source        string to convert
 * @param buffer        OUT: target buffer
 * @param bufferSize    size of the target buffer
 * @return              <code>buffer</code>
 */
char* ReQStringUtils::utf8(const ReString& source, char buffer[],
                           size_t bufferSize) {
   QByteArray val = source.toUtf8();
   if (val.length() < (int) bufferSize)
      bufferSize = val.length() + 1;
   memcpy(buffer, val.constData(), bufferSize - 1);
   buffer[bufferSize - 1] = '\0';
   return buffer;
}

class ReParserException: public ReException {
public:
   ReParserException(const QString& message) :
      ReException(),
      m_message(message) {
   }
public:
   QString m_message;
};
/**
 * Constructor.
 *
 * @param expr       an expression, e.g. "10*1024kByte+5MiByte"
 * @param unitList   description of the allowed units with its factor<br>
 *                   example: "kibyte:1024;kbyte:1000;mibyte:1048576;mbyte:1000000"
 * @param parseAtOnce	<code>true</code>: the parse process will be started
 */
ReUnitParser::ReUnitParser(const QString& expr, const char* unitList,
                           bool parseAtOnce) :
   m_result(0),
   m_expr(expr),
   m_message(),
   m_unitList(unitList) {
   normalize();
   if (parseAtOnce)
      parse();
}

/**
 * Returns the result of the expression as a 64 bit integer.
 *
 * @param defaultValue  the result if the expression was not valid
 * @return              <code>defaultValue</code>: the result was not valid<br>
 *                      the result as a 64 bit integer
 */
int64_t ReUnitParser::asInt64(int64_t defaultValue) {
   return m_message.isEmpty() ? m_result : defaultValue;
}
/**
 * Returns the result of the expression as an integer.
 *
 * @param defaultValue  the result if the expression was not valid
 * @return              <code>defaultValue</code>: the result was not valid<br>
 *                      the result as an integer
 */
int ReUnitParser::asInt(int defaultValue) {
   return m_message.isEmpty() ? (int) m_result : defaultValue;
}
/**
 * Returns the result of the expression as floating point number.
 *
 * @param defaultValue  the result if the expression was not valid
 * @return              <code>defaultValue</code>: the result was not valid<br>
 *                      the result as a floating point
 */
real_t ReUnitParser::asReal(real_t defaultValue) {
   return m_message.isEmpty() ? (real_t) m_result : defaultValue;
}

/**
 * Returns an empty string or the error message.
 *
 * @return  "": no error occurred<br>
 *          otherwise: the error message
 */
const QString& ReUnitParser::errorMessage() const {
   return m_message;
}

/**
 * Returns whether the given expression is valid.
 *
 * @return  <code>true</code>: the expression is valid, a result was calculated
 */
bool ReUnitParser::isValid() const {
   return m_message.isEmpty();
}

/**
 * @brief Normalizes the internal stored unit expression.
 */
void ReUnitParser::normalize() {
   // Remove the blanks:
   for (int ii = m_expr.length() - 1; ii >= 0; ii--) {
      if (m_expr[ii].isSpace())
         m_expr.remove(ii, 1);
   }
   // Replace the '-' operator by '+' as operator and '-' as sign:
   // This makes the syntax easier to parse: only one sum operator ('+').
   for (int ii = m_expr.length() - 1; ii > 0; ii--) {
      if (m_expr[ii] == '-' && m_expr[ii - 1] != '+'
            && m_expr[ii - 1] != '*') {
         m_expr.insert(ii, '+');
      }
   }
}

/**
 * Evaluate the expression.
 */
void ReUnitParser::parse() {
   QStringList addends = m_expr.split("+");
   QStringList::const_iterator it;
   try {
      m_result = 0;
      for (it = addends.begin(); it != addends.end(); ++it) {
         QStringList factors = it->split("*");
         QStringList::const_iterator it2;
         int64_t product = 1;
         for (it2 = factors.begin(); it2 != factors.end(); ++it2) {
            QStringList powerOperands = it2->split("^");
            if (powerOperands.count() > 2)
               throw ReParserException(
                  QObject::tr(
                     "more than 2 power operators, e.g. '2^3^4'"));
            QStringList::const_iterator it3 = powerOperands.begin();
            QString op = *it3;
            bool isNeg = op.startsWith("-");
            if (isNeg)
               op = op.mid(1);
            uint64_t value = valueOf(op);
            if (powerOperands.count() > 1) {
               uint64_t fac = value;
               uint64_t exponent = valueOf(*++it3);
               if (qLn(value) * qLn(exponent) >= qLn(qPow(2.0, 64)))
                  throw ReParserException(
                     QObject::tr(
                        "number overflow while power operation"));
               for (int ii = 1; ii < (int) exponent; ii++)
                  value = value * fac;
            }
            product *= value;
            if (isNeg)
               product = -product;
         }
         m_result += product;
      }
   } catch (ReParserException& e) {
      m_message = e.m_message;
   }
}

/**
 * Calculates the value of a number or a (number, unit) pair.
 *
 * @param value   a non negative number or a number followed by a unit<br>
 *                only units defined in m_unitList are allowed<br>
 *                examples: "4kByte" returns 4000, "4kibyte" returns 4096
 * @return        the value of the number multiplied by the factor given by the unit
 * @throws  ReParserException
 */
uint64_t ReUnitParser::valueOf(const QString& value) const {
   uint64_t rc = 0;
   int ix = ReQStringUtils::lengthOfUInt64(value, 0, 10, &rc);
   if (ix == 0)
      throw ReParserException(QObject::tr("number expected: ") + value);
   QString unit = value.mid(ix);
   if (!unit.isEmpty()) {
      QStringList units = QString(m_unitList).split(";");
      QStringList::const_iterator it;
      bool found = false;
      for (it = units.begin(); it != units.end(); ++it) {
         QStringList pair = it->split(":");
         if (pair.count() == 0)
            throw ReParserException(
               QObject::tr(
                  "missing ':' in unit definition, e.g. 'k:1000': ")
               + *it);
         if (pair.count() > 2)
            throw ReParserException(
               QObject::tr("too many  ':' in unit definition: ") + *it);
         bool ok = false;
         QString unit2 = *pair.begin();
         QString factor = *++pair.begin();
         uint64_t nFactor = factor.toLongLong(&ok);
         if (!ok)
            throw ReParserException(QObject::tr("not a number: ") + factor);
         if (unit2.startsWith(unit, Qt::CaseInsensitive)) {
            rc *= nFactor;
            found = true;
            break;
         }
      }
      if (!found)
         throw ReParserException(
            QObject::tr("unknown unit '$1'. Allowed: ").arg(unit)
            + QString(m_unitList));
   }
   return rc;
}

/**
 * Constructor.
 *
 * @param expr       an expression, e.g. "10*1024kByte+5MiByte"
 */
ReSizeParser::ReSizeParser(const QString& expr) :
   ReUnitParser(expr, "byte:1;kbyte:1000;kibyte:1024;"
                "mbyte:1000000;mibyte:1048576;"
                "gbyte:1000000000;gibyte:1073741824;"
                "tbyte:1000000000000;tibyte:1099511627776") {
}
/**
 * Constructor.
 *
 * @param expr     an expression, e.g. "3*3days-5min+3weeks"
 */
ReDateTimeParser::ReDateTimeParser(const QString& expr) :
   ReUnitParser("", "minutes:60;hours:3600;days:86400;weeks:604800", false) {
   parseDateTime(expr);
}

/**
 * Returns the parser result as a <code>QDateTime</code> instance.
 *
 * @return the parse result. If invalid input the result is the begin of the epoche
 */
QDateTime ReDateTimeParser::asDateTime() const {
   return m_dateTime;
}

/**
 * Parses a date/time expression.
 *
 * Syntax: { "now" | DATE [TIME] | TIME } [ { '+' | '-' } SECOND_EXPR]
 *          | SECOND_EXPR
 * @param expr  expression to parse, e.g. "now+3weeks-5*30days"
 * @return      start of the epoche: no valid input<br>
 *              othewise: the calculated date/time
 */
QDateTime ReDateTimeParser::parseDateTime(const QString& expr) {
   m_expr = expr;
   normalize();
   QDateTime rc = QDateTime::currentDateTime();
   int64_t relativeSeconds = 0;
   if (m_expr.isEmpty())
      m_message = QObject::tr("empty string is not a date/time");
   else {
      QDateTime dateTime;
      int length2 = 0;
      bool checkSum = true;
      if (m_expr.startsWith("now", Qt::CaseInsensitive)) {
         m_expr.remove(0, 3);
      } else if ((length2 = ReQStringUtils::lengthOfDateTime(m_expr, 0, true,
                            true, &dateTime)) > 0) {
         rc = dateTime;
         m_expr.remove(0, length2);
      } else {
         checkSum = false;
         parse();
         // meaning is "older than x seconds"
         relativeSeconds = m_result = -m_result;
      }
      if (checkSum) {
         if (m_expr.startsWith("+")) {
            m_expr.remove(0, 1);
         }
         if (!m_expr.isEmpty()) {
            parse();
            relativeSeconds = m_result;
         }
      }
   }
   rc.setMSecsSinceEpoch(
      isValid() ? rc.toMSecsSinceEpoch() + 1000 * relativeSeconds : 0);
   m_dateTime = rc;
   return rc;
}

