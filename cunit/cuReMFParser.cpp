/*
 * cuReMFParser.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Unit test of the parser for the language "MF".
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

class TestReMFParser: public ReTest {
private:
   ReSource m_source;
   ReASTree m_tree;
   ReStringReader m_reader;
   ReFileReader m_fileReader;
   QByteArray m_currentSource;
public:
   TestReMFParser() :
      ReTest("ReMFParser"),
      m_source(),
      m_tree(),
      m_reader(m_source),
      m_fileReader(m_source) {
      m_source.addReader(&m_reader);
      doIt();
   }
protected:
   void setSource(const char* content) {
      ReASItem::reset();
      m_currentSource = content;
      m_tree.clear();
      m_source.clear();
      m_reader.clear();
      m_reader.addSource("<test>", content);
      m_source.addReader(&m_reader);
      m_source.addSourceUnit(m_reader.currentSourceUnit());
   }
   void setFileSource(const char* filename) {
      ReASItem::reset();
      m_currentSource = ReStringUtil::read(filename);
      m_tree.clear();
      m_source.clear();
      m_fileReader.clear();
      m_fileReader.addSource(filename);
      m_source.addReader(&m_fileReader);
      m_source.addSourceUnit(m_fileReader.currentSourceUnit());
   }

private:
   void checkAST(const char* fileExpected, int lineNo) {
      QByteArray fnExpected = "test";
      fnExpected += QDir::separator().toLatin1();
      fnExpected += "mfparser";
      fnExpected += (char) QDir::separator().toLatin1();
      fnExpected += fileExpected;
      QByteArray fnCurrent = getTempFile(fileExpected, "rplmfparser");
      m_tree.dump(fnCurrent, ReASTree::DMP_NO_GLOBALS, m_currentSource);
      assertEqualFiles(fnExpected.constData(), fnCurrent.constData(),
                       __FILE__, lineNo);
   }

public:
   void fileClassTest() {
      setFileSource("test/rplmfparser/string1.mf");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("string1.txt", __LINE__);
   }

   void baseTest() {
      setSource("2+3*4");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("baseTest.txt", __LINE__);
   }

   void varDefTest() {
      setSource("const lazy Str s = 'Hi';\nconst List l;\nInt i = 3;");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("varDefTest.txt", __LINE__);
   }

   void ifTest() {
      setSource(
         "Int a;\nInt b;\na = b = 2;\nif 11 < 12\nthen a = 13 * 14\nelse a = 15 / 16\nfi");
      // setSource("Int a; if 11 < 12 then a = 13 * 14 else a = 15 / 16 fi");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("ifTest1.txt", __LINE__);
      setSource("Str x;\nif 7 < 6\nthen x = '123';\nfi");
      parser.parse();
      checkAST("ifTest2.txt", __LINE__);
   }
   void whileTest() {
      setSource("Int a = 20;\nwhile 3 < 5 do\n a = 7\nod");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("whileTest.txt", __LINE__);
   }

   void repeatTest() {
      setSource("Int a;\nrepeat\na++;\nuntil a != 2 * 3;");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("repeatTest.txt", __LINE__);
   }
   void forCTest() {
      setSource("Int a;\nfor b from 10 to 1 step -2 do\na += 1;\nod");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("forC1.txt", __LINE__);
      setSource("Int a; for to 10 do a += 1 od");
      parser.parse();
      checkAST("forC2.txt", __LINE__);
   }
   void opTest() {
      checkEqu(25, ReMFParser::O_QUESTION);
      checkEqu(37, ReMFParser::O_RSHIFT2);
      checkEqu(41, ReMFParser::O_DEC);
      checkEqu(48, ReMFParser::O_RBRACE);
      setSource(
         "Int a = 1;\nInt b = 100;\n--a;\nb++;\na--*++b**(8-3);\na=b=(a+(b-2)*3)");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("opTest1.txt", __LINE__);
   }
   void forItTest() {
      setSource("Map a;\nfor x in a do\na += 1;\nod");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("forIt1.txt", __LINE__);
   }
   void listTest() {
      ReMFParser parser(m_source, m_tree);
      setSource("List b = [];");
      parser.parse();
      checkAST("list1.txt", __LINE__);
      setSource("List a = [2+3, 3.14, 7, 'hi', a]; List b = [];");
      parser.parse();
      checkAST("list2.txt", __LINE__);
   }
   void mapTest() {
      setSource("Map a = {};");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("map1.txt", __LINE__);
      setSource(
         "Map a = {'a': 2+3,'bcd':3.14,'ccc':7, 'hi':'world'};\nMap b = {};");
      parser.parse();
      checkAST("map2.txt", __LINE__);
   }
   void methodCallTest() {
      //setSource("max(4,3.14);");
      setSource("rand();\nsin(a);\nmax(1+2*3,4**(5-4));");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("methc1.txt", __LINE__);
   }
   void fieldTest() {
      setSource("file.find('*.c')[0].name;\n[1,2,3].join(' ');\n3.14.trunc;");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("field1.txt", __LINE__);
   }

   void methodTest() {
      setSource("func Float pi: 3.1415; endf func Str delim(): '/' endf;");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("meth1.txt", __LINE__);
      setSource("func Int fac(const Int n):\n"
                "Int rc; if rc <= 1 then rc = 1 else rc = n*fac(n-1) fi\n"
                "rc endf");
      parser.parse();
      checkAST("meth2.txt", __LINE__);
      setSource("func Int max(Int a, Int b):\n Int rc = a;\n"
                "if a < b then rc = b; fi\nrc\n"
                "endf\n"
                "func Int max(const Int a, Int b, Int c):\n"
                "max(a, max(b, c))\n"
                "endf");
      parser.parse();
      checkAST("meth3.txt", __LINE__);
      setSource("func Int max(const Int a, Int b, Int c):\n"
                "func Int max(Int a, Int b):\n Int rc = a;\n"
                "if a < b then rc = b; fi\nrc\n"
                "endf\n"
                "max(a, max(b, c))\n"
                "endf");
      parser.parse();
      checkAST("meth4.txt", __LINE__);
   }
   void mainTest() {
      setSource("Int a=2+3*4;\nfunc Void main():\na;\nendf");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      checkAST("main1.txt", __LINE__);
   }

   virtual void run(void) {
      mainTest();
      varDefTest();
      repeatTest();
      baseTest();
      whileTest();
      methodTest();
      fieldTest();
      methodCallTest();
      mapTest();
      forItTest();
      forCTest();
      listTest();
      opTest();
      fileClassTest();
   }
};
void testReMFParser() {
   TestReMFParser test;
}

