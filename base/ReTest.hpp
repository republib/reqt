/*
 * ReTest.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RETEST_HPP
#define RETEST_HPP

// the sources generated from QT include this file directly:
class ReTest {
public:
   ReTest(const char* name);
   virtual ~ReTest();
private:
   // No copy constructor: no implementation!
   ReTest(const ReTest& source);
   // Prohibits assignment operator: no implementation!
   ReTest& operator =(const ReTest& source);
protected:
   void doIt();
public:
   bool assertEquals(int expected, int current, const char* file, int lineNo);
   bool assertEquals(int64_t expected, int64_t current, const char* file,
                     int lineNo);
   bool assertEquals(qreal expected, qreal current, const char* file,
                     int lineNo);
   bool assertEquals(const char* expected, const ReString& current,
                     const char* file, int lineNo);
   bool assertEquals(const ReString& expected, const ReString& current,
                     const char* file, int lineNo);
   bool assertEquals(const char* expected, const char* current,
                     const char* file, int lineNo);
   bool assertEquals(const QByteArray& expected, const QByteArray& current,
                     const char* file, int lineNo);
   bool assertEquals(const char* expected, const QByteArray& current,
                     const char* file, int lineNo);
   bool assertEquals(const QDate& expected, const QDate& current,
                     const char* file, int lineNo);
   bool assertEquals(const QDateTime& expected, const QDateTime& current,
                     const char* file, int lineNo);
   bool assertEquals(const QTime& expected, const QTime& current,
                     const char* file, int lineNo);
   bool assertEquals(const QList<QByteArray>& expected,
                     const QList<QByteArray>& current, const char* file, int lineNo);
   bool assertTrue(bool condition, const char* file, int lineNo);
   bool assertFalse(bool condition, const char* file, int lineNo);
   bool assertNull(const void* ptr, const char* file, int lineNo);
   bool assertNotNull(const void* ptr, const char* file, int lineNo);
   bool assertEqualFiles(const char* expected, const char* current,
                         const char* file, int lineNo);
   void ensureNotExist(const char* fullname);
   bool error(const char* message, ...);
   bool exists(const char* fullname, bool isDir = false);
   bool log(const char* message);
   bool logv(const char* format...);
   QByteArray getTempDir(const char* node, const char* parent = NULL,
                         bool withSeparator = true);
   QByteArray getTempFile(const char* node, const char* parent = NULL,
                          bool deleteIfExists = true);
   bool logContains(const char* pattern);
   virtual void runTests(void) = 0;

protected:
   int m_errors;
   QByteArray m_name;
   ReLogger m_logger;
   // for testing of logging code:
   ReMemoryAppender m_memoryAppender;
   ReLogger m_memoryLogger;
   char m_separator;
};
#define checkEqu(expected, current) assertEquals(expected, current, __FILE__, __LINE__)
#define checkT(current) assertTrue(current, __FILE__, __LINE__)
#define checkF(current) assertFalse(current, __FILE__, __LINE__)
#define checkN(current) assertNull(current, __FILE__, __LINE__)
#define checkNN(current) assertNotNull(current, __FILE__, __LINE__)
#define checkFiles(expected, current) assertEqualFiles(expected, current, __FILE__, __LINE__)
#endif // RETEST_HPP
