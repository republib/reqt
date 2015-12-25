/*
 * cuReStringUtils.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the QByteArray tools.
 */
#include "base/rebase.hpp"
/**
 * @brief Unit test for <code>ReStringUtil</code>.
 */
class TestReStringUtil: public ReTest {
public:
   TestReStringUtil() :
      ReTest("ReStringUtil") {
      doIt();
   }

public:
   void testCountChar() {
      checkEqu(1, ReStringUtils::countChar("x", 'x'));
      checkEqu(0, ReStringUtils::countChar("X", 'x'));
      checkEqu(2, ReStringUtils::countChar("xbxxbxx", 'b'));
   }

   void testCount() {
      checkEqu(0, ReStringUtils::count("abc", " "));
      checkEqu(1, ReStringUtils::count("abc", "b"));
      checkEqu(2, ReStringUtils::count("axx", "x"));
      checkEqu(0, ReStringUtils::count("abbc", "bbb"));
      checkEqu(1, ReStringUtils::count("\n\n", "\n\n"));
      checkEqu(2, ReStringUtils::count("  a  ", "  "));
   }

   void testCutString() {
      QByteArray source("123");
      QByteArray buffer;
      checkEqu(QByteArray("123"), ReStringUtils::cutString(source, 4, buffer));
      checkEqu(QByteArray("123"), ReStringUtils::cutString(source, 3, buffer));
      checkEqu(QByteArray("12..."),
               ReStringUtils::cutString(source, 2, buffer));
      checkEqu(QByteArray("12"),
               ReStringUtils::cutString(source, 2, buffer, ""));
   }

   void testHexDump() {
      QByteArray data("abc123\nxyz");
      checkEqu(QByteArray("61 62 63 31  abc1\n"
                          "32 33 0a 78  23.x\n"
                          "79 7a        yz\n"),
               ReStringUtils::hexDump((uint8_t*) data.constData(), data.length(),
                                      4));
      checkEqu(QByteArray("61 62 63 31 32 33 0a 78 79 7a  abc123.xyz"),
               ReStringUtils::hexDump((uint8_t*) data.constData(), data.length(),
                                      10));
      checkEqu(QByteArray("61 62 63 31 32 33 0a 78 79 7a        abc123.xyz"),
               ReStringUtils::hexDump((uint8_t*) data.constData(), data.length(),
                                      12));
   }

   void testReadWrite() {
      QByteArray fn = getTempFile("test.dat");
      const char* content = "Hello world\nLine2\n";
      checkT(ReStringUtils::write(fn, content));
      checkEqu(content, ReStringUtils::read(fn, false));
      checkEqu(content, ReStringUtils::read(fn, true) + "\n");
   }

   void testToArray() {
      QList < QByteArray > array = ReStringUtils::toArray("1 abc 3", " ");
      checkEqu(3, array.size());
      checkEqu("1", array.at(0));
      checkEqu("abc", array.at(1));
      checkEqu("3", array.at(2));
   }

   void testToNumber() {
      checkEqu("3", ReStringUtils::toNumber(3));
      checkEqu("-33", ReStringUtils::toNumber(-33));
      checkEqu("003", ReStringUtils::toNumber(3, "%03d"));
   }

   void testLengthOfNumber() {
      checkEqu(3, ReStringUtils::lengthOfNumber("0.3xxx"));
      checkEqu(5, ReStringUtils::lengthOfNumber(" \t0.3xxx"));
      checkEqu(3, ReStringUtils::lengthOfNumber("-.3xxx"));
      checkEqu(2, ReStringUtils::lengthOfNumber(".3exxx"));
      checkEqu(2, ReStringUtils::lengthOfNumber(".3e+xxx"));
      checkEqu(16, ReStringUtils::lengthOfNumber("1234567.9012E+77"));
      checkEqu(17, ReStringUtils::lengthOfNumber("-1234567.9012E+77 "));
      checkEqu(18, ReStringUtils::lengthOfNumber("-1234567.9012E+77 ", true));
      checkEqu(18, ReStringUtils::lengthOfNumber("-1234567.9012E+77 x", true));
      checkEqu(20,
               ReStringUtils::lengthOfNumber("  -1234567.9012E+77 x", true));
   }

   void checkCsv(const char* content, char expected) {
      QByteArray fn = getTempFile("testrplstring.csv");
      ReStringUtils::write(fn, content);
      FILE* fp = fopen(fn, "r");
      checkNN(fp);
      char buffer[256];
      checkEqu(expected,
               ReStringUtils::findCsvSeparator(fp, buffer, sizeof buffer));
      fclose(fp);
   }

   void testFindCsvSeparator() {
      const char* content = ",,,\t;;;||||";
      checkCsv(content, '\t');
      content = "col1,col2\n1.5,3,5\n";
      checkCsv(content, ',');
      content = "col1;col2\n1,50;3.5\n"
                "7;8\n10;12\n13;14";
      checkCsv(content, ';');
      content = "0.3 7.8 8.9\n7.8 9.4 8.3";
      checkCsv(content, ' ');
      content = "0.3|7.8|8.9\n7.8|         9.4|8.3";
      checkCsv(content, '|');
      content = "0,3;7.8;8.9";
      checkCsv(content, ';');
   }
   void testLengthOfUInt64() {
      quint64 value = -3;
      checkEqu(1, ReStringUtils::lengthOfUInt64("0", 10, &value));
      checkEqu((int64_t ) 0LL, value);
      checkEqu(3, ReStringUtils::lengthOfUInt64("432", 10, &value));
      checkEqu((int64_t ) 432LL, value);
      checkEqu(3, ReStringUtils::lengthOfUInt64("432 x", 10, &value));
      checkEqu((int64_t ) 432LL, value);
      checkEqu(3, ReStringUtils::lengthOfUInt64("432fabc x", 10, &value));
      checkEqu((int64_t ) 432LL, value);
      checkEqu(16,
               ReStringUtils::lengthOfUInt64("1234567890123567", 10, &value));
      checkEqu((int64_t ) 1234567890123567LL, value);
      checkEqu(10, ReStringUtils::lengthOfUInt64("1234abcdef", 16, &value));
      checkEqu((int64_t ) 0x1234abcdefLL, value);
      checkEqu(3, ReStringUtils::lengthOfUInt64("432", 8, &value));
      checkEqu((int64_t ) 0432LL, value);
      checkEqu(6, ReStringUtils::lengthOfUInt64("765432 ", 8, &value));
      checkEqu((int64_t ) 0765432LL, value);
      checkEqu(0, ReStringUtils::lengthOfUInt64(" ", 8, &value));
      checkEqu(0, ReStringUtils::lengthOfUInt64("", 8, &value));
   }
   void testLengthOfReal() {
      qreal value;
      checkEqu(1, ReStringUtils::lengthOfReal("0", &value));
      checkEqu(0.0, value);
      checkEqu(1, ReStringUtils::lengthOfReal("0%", &value));
      checkEqu(0.0, value);
      checkEqu(4, ReStringUtils::lengthOfReal("0.25", &value));
      checkEqu(0.25, value);
      checkEqu(3, ReStringUtils::lengthOfReal(".25", &value));
      checkEqu(0.25, value);
      checkEqu(17, ReStringUtils::lengthOfReal("12345678901234567", &value));
      checkEqu(12345678901234567.0, value);
      checkEqu(2, ReStringUtils::lengthOfReal(".5", &value));
      checkEqu(0.5, value);
      checkEqu(5, ReStringUtils::lengthOfReal("2.5e2x", &value));
      checkEqu(250.0, value);
      checkEqu(6, ReStringUtils::lengthOfReal("2.5e+2", &value));
      checkEqu(250.0, value);
      checkEqu(7, ReStringUtils::lengthOfReal("2.5E-33", &value));
      checkEqu(2.5e-33, value);
      checkEqu(3, ReStringUtils::lengthOfReal("2.5E", &value));
      checkEqu(2.5, value);
      checkEqu(3, ReStringUtils::lengthOfReal("2.5E+", &value));
      checkEqu(2.5, value);
      checkEqu(3, ReStringUtils::lengthOfReal("2.5E-a", &value));
      checkEqu(2.5, value);
   }

   void testChomp() {
      QByteArray buffer;
      buffer = "abc\n";
      checkEqu("abc", ReStringUtils::chomp(buffer).constData());
      buffer = "abc\r\n";
      checkEqu("abc", ReStringUtils::chomp(buffer).constData());
      buffer = "abc/";
      checkEqu("abc", ReStringUtils::chomp(buffer, '/').constData());
      buffer = "\n";
      checkEqu("", ReStringUtils::chomp(buffer).constData());
      buffer = "";
      checkEqu("", ReStringUtils::chomp(buffer).constData());
      buffer = "";
      checkEqu("", ReStringUtils::chomp(buffer, 'x').constData());
   }

   void testIsInList() {
      // case sensitive, auto separator:
      checkT(ReStringUtils::isInList("yes", ";ja;yes", false));
      checkT(ReStringUtils::isInList("yes", ";ja;yes;si", false));
      checkT(ReStringUtils::isInList("yes", ";yes;si", false));
      // case sensitive, explicite separator:
      checkT(ReStringUtils::isInList("yes", "ja;yes;si", false, ';'));
      checkT(ReStringUtils::isInList("yes", "yes;si", false, ';'));
      checkT(ReStringUtils::isInList("yes", "ja;yes", false, ';'));
      // case insensitive, auto separator:
      checkT(ReStringUtils::isInList("yes", ";ja;Yes", true));
      checkT(ReStringUtils::isInList("YES", ";ja;yes;si", true));
      checkT(ReStringUtils::isInList("yEs", ";yeS;si", true));
      // case sensitive, explicite separator:
      checkT(ReStringUtils::isInList("Yes", "ja;yes;si", true, ';'));
      checkT(ReStringUtils::isInList("yes", "Yes;si", true, ';'));
      checkT(ReStringUtils::isInList("YES", "ja;yes", true, ';'));
      // substring
      checkF(ReStringUtils::isInList("y", "ja;yes;si", true, ';'));
      // sensitive
      checkF(ReStringUtils::isInList("yes", "ja;Yes;si", false, ';'));
      // 1 element list
      checkT(ReStringUtils::isInList("yes", "yes", false, ';'));
      checkT(ReStringUtils::isInList("yes", ";yes", false));
   }

   virtual void runTests() {
      testIsInList();
      testChomp();
      testLengthOfReal();
      testLengthOfUInt64();
      testCountChar();
      testCount();
      testCutString();
      testToNumber();
      testToArray();
      testHexDump();
      testReadWrite();
      testLengthOfNumber();
      testFindCsvSeparator();
   }
};

void testReStringUtil() {
   TestReStringUtil test;
}

