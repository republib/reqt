/*
 * ReTest.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief A testing tool like JUnit.
 */
/** @file base/ReTest.hpp
 *
 * @brief Definitions for a testing tool like JUnit.
 */
#include "base/rebase.hpp"

/** @class ReTest ReTest.hpp "base/ReTest"
 *
 * @brief Implements an unit test base class similar JUnit for java.
 *
 */
class ReTest;

/**
 * @brief Constructor.
 *
 * @param name
 */
ReTest::ReTest(const char* name) :
   m_errors(0),
   m_name(name),
   m_logger(),
   m_memoryAppender(1024),
   m_memoryLogger() {
   m_memoryAppender.setAutoDelete(false);
   m_logger.buildStandardAppender(getTempDir("retest"));
   log(QByteArray("Start of ") + m_name);
   m_memoryLogger.addAppender(&m_memoryAppender);
}
void ReTest::doIt() {
   try {
      runTests();
   } catch (ReException e) {
      error("unexpected RplException: %s", e.getMessage().constData());
   } catch (...) {
      error("unknown Exception");
   }
   if (m_errors > 0) {
      error("Unit %s has %d error(s)", m_name.data(), m_errors);
      // error() increments, we decrement:
      m_errors--;
   }
}

/**
 * @brief Destructor.
 */
ReTest::~ReTest() {
}

/**
 * Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(int expected, int current, const char* file,
                          int lineNo) {
   if (expected != current)
      error("%s-%d: error: %d != %d / %x != %x)", file, lineNo, expected,
            current, (unsigned int) expected, (unsigned int) current);
   return expected == current;
}

/**
 * Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(int64_t expected, int64_t current, const char* file,
                          int lineNo) {
   if (expected != current)
      error("%s-%d: error: %lld != %lld / %llx != %llx)", file, lineNo,
            expected, current, (quint64) expected, (quint64) current);
   return expected == current;
}

/**
 * Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(qreal expected, qreal current, const char* file,
                          int lineNo) {
   if (expected != current)
      error("%s-%d: error: %d != %d / %x != %x)", file, lineNo, expected,
            current, (unsigned int) expected, (unsigned int) current);
   return expected == current;
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const char* expected, const ReString& current,
                          const char* file, int lineNo) {
   bool equal = assertEquals(expected, I18N::s2b(current).constData(), file,
                             lineNo);
   return equal;
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const ReString& expected, const ReString& current,
                          const char* file, int lineNo) {
   bool equal = assertEquals(I18N::s2b(expected).constData(),
                             I18N::s2b(current).constData(), file, lineNo);
   return equal;
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const char* expected, const char* current,
                          const char* file, int lineNo) {
   bool equal = strcmp(expected, current) == 0;
   if (!equal) {
      if (strchr(expected, '\n') != NULL || strchr(current, '\n')) {
         QList < QByteArray > exp = ReStringUtils::toArray(expected, "\n");
         QList < QByteArray > cur = ReStringUtils::toArray(current, "\n");
         equal = assertEquals(exp, cur, file, lineNo);
      } else {
         int ix = 0;
         while (expected[ix] == current[ix] && expected[ix] != '\0')
            ix++;
         char pointer[12 + 1];
         char* ptr = pointer;
         int maxIx = ix > 10 ? 10 : ix;
         for (int ii = 0; ii < maxIx - 1; ii++)
            *ptr++ = '-';
         *ptr++ = '^';
         *ptr = '\0';
         if (ix < 10)
            error("%s-%d: error: diff at index %d\n%s\n%s\n%s", file,
                  lineNo, ix, expected, current, pointer);
         else
            error("%s-%d: error: diff at index %d\n%s\n...%s\n...%s\n%s",
                  file, lineNo, ix, current, expected + ix - 10 + 3,
                  current + ix - 10 + 3, pointer);
      }
   }
   return equal;
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const QList<QByteArray>& expected,
                          const QList<QByteArray>& current, const char* file, int lineNo) {
   int nMax = expected.size();
   bool rc = true;
   if (current.size() < nMax)
      nMax = current.size();
   for (int ix = 0; ix < nMax; ix++) {
      if (expected.at(ix) != current.at(ix)) {
         error("%s-%d: difference in line %d", file, lineNo, ix + 1);
         m_errors--;
         assertEquals(expected.at(ix).constData(),
                      current.at(ix).constData(), file, lineNo);
         rc = false;
         break;
      }
   }
   if (rc) {
      if (expected.size() > nMax)
         error("%s-%d: less lines than expected (%d):\n%s", file, lineNo,
               nMax, expected.at(nMax).constData());
      else if (expected.size() < nMax)
         error("%s-%d: more lines than expected (%d):\n%s", file, lineNo,
               nMax, current.at(nMax).constData());
   }
   return rc;
}
/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const QByteArray& expected, const QByteArray& current,
                          const char* file, int lineNo) {
   return assertEquals(expected.constData(), current.constData(), file, lineNo);
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const char* expected, const QByteArray& current,
                          const char* file, int lineNo) {
   return assertEquals(expected, current.constData(), file, lineNo);
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const QDate& expected, const QDate& current,
                          const char* file, int lineNo) {
   return assertEquals(expected.toString("yyyy.MM.dd"),
                       current.toString("yyyy.MM.dd"), file, lineNo);
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const QDateTime& expected, const QDateTime& current,
                          const char* file, int lineNo) {
   return assertEquals(expected.toString("yyyy.MM.dd hh:mm:ss"),
                       current.toString("yyyy.MM.dd hh:mm:ss"), file, lineNo);
}

/**
 * @brief Tests the equality of two values.
 *
 * Differences will be logged.
 *
 * @param expected      the expected value
 * @param current       the current value
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: equal
 */
bool ReTest::assertEquals(const QTime& expected, const QTime& current,
                          const char* file, int lineNo) {
   return assertEquals(expected.toString("hh:mm:ss"),
                       current.toString("hh:mm:ss"), file, lineNo);
}

/**
 * @brief Tests whether a value is true.
 *
 * A value of false will be logged.
 *
 * @param condition     value to test
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              <code>condition</code>
 */
bool ReTest::assertTrue(bool condition, const char* file, int lineNo) {
   if (!condition)
      error("%s-%d: not TRUE", file, lineNo);
   return condition;
}

/**
 * @brief Tests whether a value is false.
 *
 * A value of true will be logged.
 *
 * @param condition     value to test
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              <code>! condition</code>
 */
bool ReTest::assertFalse(bool condition, const char* file, int lineNo) {
   if (condition)
      error("%s-%d: not FALSE", file, lineNo);
   return !condition;
}

/**
 * @brief Tests whether a value is NULL.
 *
 * A value of not NULL will be logged.
 *
 * @param ptr           value to test
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: ptr is NULL
 */
bool ReTest::assertNull(const void* ptr, const char* file, int lineNo) {
   if (ptr != NULL)
      error("%s-%d: not NULL", file, lineNo);
   return ptr == NULL;
}

/**
 * @brief Tests whether a value is not NULL.
 *
 * A value of NULL will be logged.
 *
 * @param ptr           value to test
 * @param file          the file containing the test
 * @param lineNo        the line number containing the test
 * @return              true: ptr is not NULL
 */
bool ReTest::assertNotNull(const void* ptr, const char* file, int lineNo) {
   if (ptr == NULL)
      error("%s-%d: is NULL", file, lineNo);
   return ptr != NULL;
}

/**
 * @brief Compares two files line by line.
 *
 * @param expected  the file with the expected content
 * @param current   the file with the current content
 * @param file      the source file (point of the comparison)
 * @param lineNo    the source position (point of the comparison)
 * @return          true: the files are equal<br>
 *                  false: otherwise
 */
bool ReTest::assertEqualFiles(const char* expected, const char* current,
                              const char* file, int lineNo) {
   bool rc = false;
   QByteArray expectedContent = ReStringUtils::read(expected, true);
   QByteArray currentContent = ReStringUtils::read(current, true);
   if (expectedContent.isEmpty()) {
      char buffer[512];
      qsnprintf(buffer, sizeof buffer, "%s has no content. Does it exist?",
                expected);
      error(buffer);
   } else if (currentContent.isEmpty()) {
      char buffer[512];
      qsnprintf(buffer, sizeof buffer, "%s has no content. Does it exist?",
                current);
      error(buffer);
   } else {
      QList < QByteArray > expLines = expectedContent.split('\n');
      QList < QByteArray > curLines = currentContent.split('\n');
      rc = assertEquals(expLines, curLines, file, lineNo);
   }
   return rc;
}

/**
  * Ensures that the file (or the directory) does not exist.
  *
 * @param fullname	filename with path
 */
void ReTest::ensureNotExist(const char* fullname) {
   if (exists(fullname))
      unlink(fullname);
   if (exists(fullname) && exists(fullname, true))
      rmdir(fullname);
   if (exists(fullname))
      error("cannot delete (%d): %s", errno, fullname);
}

/**
 * @brief Writes an error.
 *
 * @param format    message to show. With placeholders like <code>std::printf()</code>
 * @param ...       the values for the placeholders in <code>format</code>
 * @return          false (for chaining)
 */
bool ReTest::error(const char* format, ...) {
   m_errors++;
   va_list ap;
   va_start(ap, format);
   m_logger.log(LOG_ERROR, 0, format, ap);
   va_end(ap);
   return false;
}

/**
 * Tests whether a file exists.
 * @param fullname	the filename with path
 * @param isDir		<code>true</code>: the file must exist and be a directory
 * @return			<code>true</code>: the file exists.
 */
bool ReTest::exists(const char* fullname, bool isDir) {
   struct stat info;
   bool rc = stat(fullname, &info) == 0;
   if (! rc && isDir && ! S_ISDIR(info.st_mode))
      rc = false;
   return rc;
}

/**
 * @brief Writes an info.
 *
 * @param message   message to show
 * @return          true (for expressions)
 */
bool ReTest::log(const char* message) {
   m_logger.log(LOG_INFO, 0, message);
   return true;
}
/**
 * @brief Writes a message with arguments.
 *
 * @param format    message to show. With placeholders like <code>std::printf()</code>
 * @param ...       the values for the placeholders in <code>format</code>
 * @return          true (for expressions)
 */
bool ReTest::logv(const char* format, ...) {
   va_list ap;
   va_start(ap, format);
   m_logger.log(LOG_INFO, 0, format, ap);
   va_end(ap);
   return true;
}


/**
 * @brief Tests whether the m_memoryLogger has a message containing a given pattern.
 *
 * @param pattern   regular expression to search
 * @return          true: pattern has been found<br>
 *                  false: otherwise
 */
bool ReTest::logContains(const char* pattern) {
   const QList<QByteArray>& lines = m_memoryAppender.getLines();
   QRegularExpression rexpr(pattern);
   bool rc = false;
   QRegularExpressionMatch match;
   for (int ii = 0; ii < lines.size(); ii++) {
      const QByteArray& line = lines.at(ii);
      match = rexpr.match(line);
      if (match.hasMatch()) {
         rc = true;
         break;
      }
   }
   return rc;
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
QByteArray ReTest::getTempDir(const char* node, const char* parent,
                              bool withSeparator) {
   QByteArray temp("c:\\temp");
   struct stat info;
   const char* ptr;
   if ((ptr = getenv("TMP")) != NULL)
      temp = ptr;
   else if ((ptr = getenv("TEMP")) != NULL)
      temp = ptr;
   else if (stat("/tmp", &info) == 0)
      temp = "/tmp";
   char sep = m_separator = temp.indexOf('/') >= 0 ? '/' : '\\';
   if (temp.at(temp.length() - 1) != sep)
      temp += sep;
   if (parent != NULL) {
      temp += parent;
      if (stat(temp.constData(), &info) != 0)
         mkdir(temp.constData(), (-1));
      temp += sep;
   }
   if (node != NULL) {
      temp += node;
      temp += sep;
      if (stat(temp.data(), &info) != 0)
         mkdir(temp.data(), -1);
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
QByteArray ReTest::getTempFile(const char* node, const char* parent,
                               bool deleteIfExists) {
   QByteArray dir = getTempDir(parent);
   QByteArray rc = dir;
   if (!rc.endsWith(m_separator))
      rc += m_separator;
   rc += node;
   struct stat info;
   if (deleteIfExists && stat(rc.constData(), &info) == 0)
      unlink(rc.constData());
   return rc;
}
