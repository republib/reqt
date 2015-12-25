/*
 * cuReException.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"

/** @file
 * @brief Unit test of the basic exceptions.
 */

class TestReException: public ReTest {
public:
   TestReException() :
      ReTest("ReException") {
      doIt();
   }

public:
   void testBasic() {
      try {
         throw ReException("simple");
         checkF(true);
      } catch (ReException exc) {
         checkEqu("simple", exc.getMessage().constData());
      }
      try {
         throw ReException("String: %s and int %d", "Hi", -333);
         checkF(true);
      } catch (ReException exc) {
         checkEqu("String: Hi and int -333", exc.getMessage().constData());
      }
      try {
         throw ReException(LOG_INFO, 1234, &m_memoryLogger,
                           "String: %s and int %d", "Hi", -333);
         checkF(true);
      } catch (ReException exc) {
         checkT(logContains("^ .*\\(1234\\): String: Hi and int -333"));
      }
      log("ok");
   }
   virtual void runTests() {
      testBasic();
   }
};
void testReException() {
   TestReException test;
}

