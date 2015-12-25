/*
 * testrplexample.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
include "project.hpp"

#include "base/rebase.hpp"
// Code to test:
int add(int a, int b) {
   return a+b;
}
QByteArray concat(const char* a, const char* b) {
   return QByteArray(a) + " " + b;
}
const char* firstDot(const char* s) {
   return strchr(s, '.');
}
/**
 * @brief Example for usage of the class ReTest.
 */
class TestRplExample: public ReTest {
public:
   TestRplExample() :
      ReTest("RplExample") {
   }

public:
   void testInt() {
      log("testing add...");
      // compare 2 integers:
      checkEqu(2, add(1, 1));
   }
   void testString() {
      // compare 2 strings:
      checkEqu("Be good", concat("Be", "good"));
      // test for not NULL:
      checkN(firstDot("Hi."));
      // test for  NULL:
      checkNN(firstDot("Hi"));
   }
   virtual void doIt() {
      testInt();
      testString();
   }
};
void testRplExample() {
   TestRplExample test;
   test.run();
}
