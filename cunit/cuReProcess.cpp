/*
 * cuReProcess.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Unit test of the byte and C string storage.
 */

#include "base/rebase.hpp"

class TestReProcess: public ReTest {
public:
   TestReProcess() :
      ReTest("ReProcess") {
      doIt();
   }
private:
   void testExecAndRead() {
#if defined __linux__
      QStringList args;
      args.append("-u");
      QByteArray id = ReProcess::executeAndRead(QString("/usr/bin/id"), args);
      checkF(id.isEmpty());
#else
#error "missing test"
#endif
   }

public:
   virtual void runTests() {
      testExecAndRead();
   }
};
void testReProcess() {
   TestReProcess test;
}




