/*
 * cuReWriter.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the output media writers.
 */

#include "base/rebase.hpp"
/**
 * @brief Unit test for <code>ReStringUtil</code>.
 */
class TestReWriter: public ReTest {
public:
   TestReWriter() :
      ReTest("ReWriter") {
      doIt();
   }

private:
   void testFileWriter() {
      QByteArray fn = getTempFile("rplwriter.txt");
      ReFileWriter writer(fn);
      writer.writeLine("abc");
      writer.formatLine("%04d", 42);
      writer.writeIndented(3, "123");
      writer.indent(2);
      writer.write("pi");
      writer.format("%3c%.2f", ':', 3.1415);
      writer.writeLine();
      writer.close();
      QByteArray current = ReStringUtils::read(fn, false);
      checkEqu("abc\n0042\n\t\t\t123\n\t\tpi  :3.14\n", current);
   }

public:
   virtual void runTests(void) {
      testFileWriter();
   }
};
void testReWriter() {
   TestReWriter test;
}

