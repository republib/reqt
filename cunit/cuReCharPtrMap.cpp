/*
 * cuReCharPtrMap.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"

class TestReCharPtrMap: public ReTest {
public:
   TestReCharPtrMap() :
      ReTest("ReCharPtrMap") {
      doIt();
   }
protected:
   void testBasic() {
      ReCharPtrMap<const char*> map;
      map["x"] = "x1";
      checkT(map.contains("x"));
      checkF(map.contains("y"));
      checkEqu("x1", map["x"]);
   }
public:
   virtual void runTests(void) {
      testBasic();
   }
};
void testReCharPtrMap() {
   TestReCharPtrMap test;
}
