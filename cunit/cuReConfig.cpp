/*
 * cuReConfig.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
/**
 * @brief Unit test of ReConfig.
 */
class TestReConfig: public ReTest {
public:
   TestReConfig() :
      ReTest("ReConfig") {
      doIt();
   }

public:
   void testBasic() {
      QByteArray fn = getTempFile("test.data", "config");
      ReStringUtils::write(fn, "#comment\na=1\nb.1==x\n#=\nB=zzz");
      ReConfig config(fn.constData());
      checkEqu(3, config.size());
      checkEqu("1", config["a"]);
      checkEqu("=x", config["b.1"]);
      checkEqu("zzz", config["B"]);
   }
   void testAsX() {
      QByteArray fn = getTempFile("test.data", "config");
      ReStringUtils::write(fn, "i=123\nb=1\nb2=true\nb3=yes\ns=abc");
      ReConfig config(fn.constData());
      checkEqu(5, config.size());
      checkEqu(123, config.asInt("i", -1));
      checkEqu(-1, config.asInt("I", -1));
      checkT(config.asBool("b", false));
      checkT(config.asBool("b2", false));
      checkT(config.asBool("b3", false));
      checkT(config.asBool("-", true));
      checkF(config.asBool("-", false));
      checkEqu("abc", config.asString("s", "x"));
      checkEqu("x", config.asString("S", "x"));
   }

   virtual void runTests() {
      testAsX();
      testBasic();
   }
};

void testReConfig() {
   TestReConfig test;
}

