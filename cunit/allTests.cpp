/*
 * allTests.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "../base/rebase.hpp"
#include "../math/remath.hpp"
#include "../net/renet.hpp"
#include "../expr/reexpr.hpp"
//#include "os/reos.hpp"
#include <QApplication>

static bool s_allTest = false;

static void testGui() {
   char* argv[2] = { (char*) "dummy", NULL };
   int argc = 1;
   QApplication a(argc, argv);
   void testReStateStorage();
   void testReEdit();
   void testReSettings();
   testReSettings();
   testReStateStorage();
   testReEdit();
}

static void testBase() {
   void testReProgArgs();
   void testReProcess();
   void testReRandomizer();
   void testReByteStorage();
   void testReCharPtrMap();
   void testReConfig();
   void testReContainer();
   void testReException();
   void testReQStringUtil();
   void testReStringUtil();
   void testReFileUtils();
   void testReWriter();
   void testReFile();
   void testReMatcher();
   testReProgArgs();
   testReProcess();
   testReRandomizer();
   testReFileUtils();
   testReMatcher();
   testReQStringUtil();
   testReFile();
   if (s_allTest) {
      testReProcess();
      testReRandomizer();
      testReByteStorage();
      testReCharPtrMap();
      testReConfig();
      testReContainer();
      testReException();
      testReQStringUtil();
      testReStringUtil();
      testReWriter();
   }
}
static void testMath() {
}
static void testExpr() {
   extern void testReMFParser();
   extern void testRplBenchmark();
   extern void testReVM();
   extern void testReSource();
   extern void testReLexer();
   extern void testReMFParser();
   extern void testReASTree();
   extern void testReVM();
   testReLexer();
   /*
   //testRplBenchmark();
   if (s_allTest){
   testReVM();
   testReSource();
   testReLexer();
   testReMFParser();
   testReASTree();
   testReVM();
   }
   */
}
static void testNet() {
}
static void testOs() {
   void testReFileSystem();
   void testReCryptFileSystem();
   testReFileSystem();
   testReCryptFileSystem();
}
void allTests() {
   testOs();
   testExpr();
   testBase();
   testGui();
   if (s_allTest) {
      testBase();
      testMath();
      testNet();
      testOs();
   }
}

