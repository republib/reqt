/*
 * ReQStringUtils.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#ifndef REQSTRINGUTILS_HPP
#define REQSTRINGUTILS_HPP
/**
 * A pure static class for internationalization.
 */
class I18N {
public:
   enum CharSet { SYSTEM, UTF8, LATIN };
public:
   /** Converts a <code>QString</code> into a <code>QByteArray</code>.
    * The character set is a global setting: <code>m_standardCharSet</code>.
    * @param source	the string to convert
    * @return	the converted string
    */
   inline static QByteArray s2b(const QString& source) {
      if (m_standardCharSet == UTF8)
         return source.toUtf8();
      else if (m_standardCharSet == LATIN)
         return source.toLatin1();
      else
         return source.toLocal8Bit();
   }
public:
   static CharSet m_standardCharSet;
};

/**
 * Some useful static functions handling <code>QString</code> instances.
 */
class ReQStringUtils {
public:
   static ReString chomp(const ReString& text, char cc = '\n');
   static int countOf(const QString& value, QChar toFind, int start = 0);
   static QString& ensureLastChar(QString& value, QChar lastChar);
   static int lengthOfDate(const ReString& text, int start = 0, QDate* value =
                              NULL);
   static int lengthOfDateTime(const ReString& text, int start = 0,
                               bool allowDateOnly = true, bool allowTimeOnly = true, QDateTime* value =
                                  NULL);
   static int lengthOfReal(const ReString& text, int start = 0, qreal* value =
                              NULL);
   static int lengthOfTime(const ReString& text, int start = 0, QTime* value =
                              NULL);
   static int lengthOfUInt64(const ReString& text, int start = 0, int radix =
                                10, uint64_t* value = NULL);
   static int lengthOfUInt(const ReString& text, int start, int radix,
                           uint* pValue);
   static QString longestPrefix(const QStringList& list);
   static bool match(const QString& heap, const QStringList& needles);
   /**
    * Returns the path with native path separators.
    *
    * @param path   the path to change
    * @return       the path with native path separators
    */
   inline static QString nativePath(const QString& path) {
#if defined WIN32
      QString rc(path);
      return rc.replace("/", "\\");
#else
      return path;
#endif
   }
   static bool replacePlaceholders(QString& text,
                                   const QMap<QString, QString>& placeholders, QString* error);
   static void skipExpected(const ReString& text, QChar expected, int& index,
                            int& length);
   /**
    * @brief Returns the value of a hexadecimal digit.
    *
    * @param digit     a (unicode) character
    * @return          -1: not a hexadecimal digit<br>
    *                  otherwise: the value, e.g. 10 for 'a'
    */
   inline static int valueOfHexDigit(int digit) {
      return digit >= '0' && digit <= '9' ? digit - '0' :
             digit >= 'A' && digit <= 'F' ? digit - 'A' + 10 :
             digit >= 'a' && digit <= 'f' ? digit - 'a' + 10 : -1;
   }
   static char* utf8(const ReString& source, char buffer[], size_t bufferSize);
public:
   static const QStringList m_emptyList;
   static const QString m_empty;
};

/**
 * Calculates expressions with simple arithmetic and units.
 *
 * Allowed operators are '+', '-', '*', '/' and '^'.
 * Parenthesis are not allowed.
 *
 * Example: 3*7k+8 means 21008
 */
class ReUnitParser {
public:
   ReUnitParser(const QString& expr, const char* unitList, bool parseAtOnce =
                   true);
public:
   bool isValid() const;
   const QString& errorMessage() const;
   int64_t asInt64(int64_t defaultValue = -1ll);
   int asInt(int defaultValue = -1);
   real_t asReal(real_t defaultValue = -1.0);
protected:
   void parse();
   void normalize();
   uint64_t valueOf(const QString& value) const;
protected:
   int64_t m_result;
   QString m_expr;
   QString m_message;
   const char* m_unitList;
};

class ReSizeParser: public ReUnitParser {
public:
   ReSizeParser(const QString& expr);
};

/**
 * Calculates time expressions.
 *
 * Syntax: { "now" | DATE [TIME] | TIME } [ { '+' | '-' } SECOND_EXPR]
 *          | SECOND_EXPR
 * Example: now+3weeks-5*30days
 */
class ReDateTimeParser: public ReUnitParser {
public:
   ReDateTimeParser(const QString& expr);
public:
   QDateTime parseDateTime(const QString& expr);
   QDateTime asDateTime() const;
private:
   QDateTime m_dateTime;
};

#endif // REQSTRINGUTILS_HPP
