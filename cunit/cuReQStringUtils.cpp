/*
 * cuReQStringUtils.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Unit test of the ReString tools.
 */

#include "../base/rebase.hpp"

class TestReQStringUtil: public ReTest {
public:
   TestReQStringUtil() :
      ReTest("ReQStringUtil") {
      doIt();
   }

public:
   void testCountOf() {
      checkEqu(2, ReQStringUtils::countOf(QString("axbx"), 'x'));
      checkEqu(2, ReQStringUtils::countOf(QString("axbx"), 'x', 1));
      checkEqu(1, ReQStringUtils::countOf(QString("axbx"), 'x', 2));
      checkEqu(1, ReQStringUtils::countOf(QString("axbx"), 'x', 3));
      checkEqu(0, ReQStringUtils::countOf(QString("axbx"), 'x', 4));
      checkEqu(0, ReQStringUtils::countOf(QString("axbx"), 'x', 5));
      checkEqu(0, ReQStringUtils::countOf(QString("axbx"), 'x', -1));
   }

   void testLengthOfUInt64() {
      quint64 value = -3;
      checkEqu(1,
               ReQStringUtils::lengthOfUInt64(ReString("0"), 0, 10, &value));
      checkEqu(int64_t(0), value);
      checkEqu(3, ReQStringUtils::lengthOfUInt64("x432", 1, 10, &value));
      checkEqu(int64_t(432LL), value);
      checkEqu(3, ReQStringUtils::lengthOfUInt64("x432 x", 1, 10, &value));
      checkEqu(int64_t(432LL), value);
      checkEqu(3,
               ReQStringUtils::lengthOfUInt64("x432fabc x", 1, 10, &value));
      checkEqu(int64_t(432LL), value);
      checkEqu(16,
               ReQStringUtils::lengthOfUInt64("a1234567890123567", 1, 10, &value));
      checkEqu(int64_t(1234567890123567LL), value);
      checkEqu(10,
               ReQStringUtils::lengthOfUInt64("x1234abcdef", 1, 16, &value));
      checkEqu(int64_t(0x1234abcdefLL), value);
      checkEqu(3, ReQStringUtils::lengthOfUInt64("432", 0, 8, &value));
      checkEqu(int64_t(0432LL), value);
      checkEqu(6, ReQStringUtils::lengthOfUInt64(" 765432 ", 1, 8, &value));
      checkEqu(int64_t(0765432LL), value);
      checkEqu(0, ReQStringUtils::lengthOfUInt64("1 ", 1, 8, &value));
      checkEqu(0, ReQStringUtils::lengthOfUInt64("", 1, 8, &value));
   }
   void testLengthOfUInt() {
      uint value = 3;
      checkEqu(1, ReQStringUtils::lengthOfUInt(ReString("0"), 0, 10, &value));
      checkEqu(0, value);
      checkEqu(3, ReQStringUtils::lengthOfUInt("x432", 1, 10, &value));
      checkEqu(432, value);
      checkEqu(3, ReQStringUtils::lengthOfUInt("x432 x", 1, 10, &value));
      checkEqu(432, value);
      checkEqu(3, ReQStringUtils::lengthOfUInt("x432fabc x", 1, 10, &value));
      checkEqu(432, value);
      checkEqu(3, ReQStringUtils::lengthOfUInt("432", 0, 8, &value));
      checkEqu(0432, value);
      checkEqu(6, ReQStringUtils::lengthOfUInt(" 765432 ", 1, 8, &value));
      checkEqu(0765432, value);
      checkEqu(0, ReQStringUtils::lengthOfUInt("1 ", 1, 8, &value));
      checkEqu(0, ReQStringUtils::lengthOfUInt("", 1, 8, &value));
   }
   void testLengthOfReal() {
      qreal value;
      checkEqu(4, ReQStringUtils::lengthOfReal(ReString("0.25"), 0, &value));
      checkEqu(0.25, value);
      checkEqu(3, ReQStringUtils::lengthOfReal(ReString("X.25"), 1, &value));
      checkEqu(0.25, value);
      checkEqu(1, ReQStringUtils::lengthOfReal(ReString(" 0"), 1, &value));
      checkEqu(0.0, value);
      checkEqu(17,
               ReQStringUtils::lengthOfReal(ReString("X12345678901234567"), 1,
                                            &value));
      checkEqu(12345678901234567.0, value);
      checkEqu(2, ReQStringUtils::lengthOfReal(ReString(".5"), 0, &value));
      checkEqu(0.5, value);
      checkEqu(5,
               ReQStringUtils::lengthOfReal(ReString("2.5e2x"), 0, &value));
      checkEqu(250.0, value);
      checkEqu(6,
               ReQStringUtils::lengthOfReal(ReString("2.5e+2"), 0, &value));
      checkEqu(250.0, value);
      checkEqu(7,
               ReQStringUtils::lengthOfReal(ReString("2.5E-33"), 0, &value));
      checkEqu(2.5e-33, value);
      checkEqu(3, ReQStringUtils::lengthOfReal(ReString("2.5E"), 0, &value));
      checkEqu(2.5, value);
      checkEqu(3, ReQStringUtils::lengthOfReal(ReString("2.5E+"), 0, &value));
      checkEqu(2.5, value);
      checkEqu(3,
               ReQStringUtils::lengthOfReal(ReString("2.5E-a"), 0, &value));
      checkEqu(2.5, value);
   }

   void testValueOfHexDigit() {
      checkEqu(0, ReQStringUtils::valueOfHexDigit('0'));
      checkEqu(9, ReQStringUtils::valueOfHexDigit('9'));
      checkEqu(10, ReQStringUtils::valueOfHexDigit('a'));
      checkEqu(15, ReQStringUtils::valueOfHexDigit('f'));
      checkEqu(10, ReQStringUtils::valueOfHexDigit('A'));
      checkEqu(15, ReQStringUtils::valueOfHexDigit('F'));
      checkEqu(-1, ReQStringUtils::valueOfHexDigit('0' - 1));
      checkEqu(-1, ReQStringUtils::valueOfHexDigit('9' + 1));
      checkEqu(-1, ReQStringUtils::valueOfHexDigit('A' - 1));
      checkEqu(-1, ReQStringUtils::valueOfHexDigit('F' + 1));
      checkEqu(-1, ReQStringUtils::valueOfHexDigit('a' - 1));
      checkEqu(-1, ReQStringUtils::valueOfHexDigit('f' + 1));
   }
   void testUtf8() {
      ReString name = "Heinz Müller";
      char buffer[32];
      checkEqu("Heinz Müller",
               ReQStringUtils::utf8(name, buffer, sizeof buffer));
      memset(buffer, 'x', sizeof buffer);
      checkEqu("Heinz", ReQStringUtils::utf8(name, buffer, (size_t)(5 + 1)));
      checkEqu(buffer[6], 'x');
   }

   void testUnitParser() {
      ReUnitParser parser("-1-2*3*4+2^3*4", NULL);
      checkT(parser.isValid());
      checkEqu(7, parser.asInt());
      checkEqu(7LL, parser.asInt64());
      checkEqu(7.0, parser.asReal());
   }
   void testSizeParser() {
      ReSizeParser parser("2^3byte+2*1k+1m+1g+1t");
      checkT(parser.isValid());
      checkEqu(1001001002008LL, parser.asInt64());
      ReSizeParser parser2("1ki+1mi+1gi+1ti");
      checkT(parser2.isValid());
      checkEqu(1100586419200ll, parser2.asInt64());
   }
   void testDateTimeParser() {
      ReDateTimeParser parser("3.4.2014");
      checkEqu(QDateTime(QDate(2014, 4, 3)), parser.asDateTime());
      ReDateTimeParser parser2("21.4.2014-2w");
      checkEqu(QDateTime(QDate(2014, 4, 7)), parser2.asDateTime());
      ReDateTimeParser parserB2("1+1min+1h+1day+1week");
      checkT(parserB2.isValid());
      checkEqu(-694861, parserB2.asInt());
   }
   void testLengtOfDate() {
      QDate date;
      checkEqu(8, ReQStringUtils::lengthOfDate("1.2.2001", 0, &date));
      checkEqu(QDate(2001, 2, 1), date);
      checkEqu(9, ReQStringUtils::lengthOfDate("5.12.2001xxx", 0, &date));
      checkEqu(QDate(2001, 12, 5), date);
      checkEqu(10, ReQStringUtils::lengthOfDate("011.10.2001xxx", 1, &date));
      checkEqu(QDate(2001, 10, 11), date);
      checkEqu(8, ReQStringUtils::lengthOfDate("2001.2.1", 0, &date));
      checkEqu(QDate(2001, 2, 1), date);
      checkEqu(9, ReQStringUtils::lengthOfDate("2001.12.5xxx", 0, &date));
      checkEqu(QDate(2001, 12, 5), date);
      checkEqu(10, ReQStringUtils::lengthOfDate("02001.03.01xxx", 1, &date));
      checkEqu(QDate(2001, 3, 1), date);
   }
   void testLengtOfTime() {
      QTime time;
      checkEqu(3, ReQStringUtils::lengthOfTime("1:2", 0, &time));
      checkEqu(QTime(1, 2, 0), time);
      checkEqu(5, ReQStringUtils::lengthOfTime("301:02", 1, &time));
      checkEqu(QTime(1, 2, 0), time);
      checkEqu(7, ReQStringUtils::lengthOfTime("301:02:9", 1, &time));
      checkEqu(QTime(1, 2, 9), time);
      checkEqu(8, ReQStringUtils::lengthOfTime("301:02:09x", 1, &time));
      checkEqu(QTime(1, 2, 9), time);
   }
   void testLongestPrefix() {
      QStringList list;
      list << "def" << "demask" << "destruct";
      checkEqu("de", ReQStringUtils::longestPrefix(list));
      list.clear();
      list << "demask" << "def" << "destruct";
      checkEqu("de", ReQStringUtils::longestPrefix(list));
      list.clear();
      list << "demask" << "destruct" << "de";
      checkEqu("de", ReQStringUtils::longestPrefix(list));
      list.clear();
      list << "de" << "demask" << "destruct";
      checkEqu("de", ReQStringUtils::longestPrefix(list));
      list.clear();
      list << "destruct";
      checkEqu("destruct", ReQStringUtils::longestPrefix(list));
      list.clear();
      checkEqu("", ReQStringUtils::longestPrefix(list));
   }

   virtual void runTests(void) {
      testLongestPrefix();
      testLengtOfTime();
      testLengtOfDate();
      testDateTimeParser();
      testUnitParser();
      testSizeParser();
      testUtf8();
      testLengthOfUInt64();
      testLengthOfUInt();
      testLengthOfReal();
      testValueOfHexDigit();
   }
};
void testReQStringUtil() {
   TestReQStringUtil test;
}

