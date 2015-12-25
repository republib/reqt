/*
 * cuReASTree.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the abstract syntax tree.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

class TestReASTree: public ReTest {
private:
   ReSource m_source;
   ReStringReader m_reader;
   ReStringSourceUnit m_unit;
   ReASTree m_tree;
public:
   TestReASTree() :
      ReTest("ReASTree"),
      m_source(),
      m_reader(m_source),
      m_unit("<main>", "", &m_reader),
      m_tree() {
   }
public:
   void testReASException() {
      try {
         m_reader.addSource("<main>", "12");
         m_source.addReader(&m_reader);
         m_source.addSourceUnit(m_reader.currentSourceUnit());
         const ReSourcePosition* pos = m_source.newPosition(2);
         throw ReASException(pos, "simple string: %s", "Hi");
         checkF(true);
      } catch (ReASException exc) {
         checkEqu("<main>:0:2: simple string: Hi",
                  exc.getMessage().constData());
      }
   }
   void testReASVariant() {
      ReASVariant val1;
      val1.setFloat(2.5E-2);
      checkEqu(2.5E-2, val1.asFloat());
      ReASVariant val2(val1);
      checkEqu(2.5E-2, val2.asFloat());
      val1.setInt(4321);
      checkEqu(4321, val1.asInt());
      val2 = val1;
      checkEqu(4321, val2.asInt());
      val1.setBool(false);
      checkF(val1.asBool());
      val2 = val1;
      checkF(val2.asBool());
      val1.setBool(true);
      checkT(val1.asBool());
      val2 = val1;
      checkT(val2.asBool());
      val1.setString("High noon!");
      checkEqu("High noon!", *val1.asString());
      val2 = val1;
      val1.setString("Bye");
      checkEqu("High noon!", *val2.asString());
      ReASVariant val3(val1);
      checkEqu("Bye", *val3.asString());
   }
   void testReASConstant() {
      ReASConstant constant;
      //constant.value().setString("Jonny");
      ReASVariant value;
      //constant.calc(value);
      //checkEqu("Jonny", *value.asString());
   }
   void testReASNamedValue() {
      ReASNamedValue value(NULL, m_tree.symbolSpaces()[0], "gugo",
                           ReASNamedValue::A_GLOBAL);
      checkEqu("gugo", value.name());
   }
   virtual void run() {
      testReASNamedValue();
      testReASConstant();
      testReASException();
      testReASVariant();
   }
};
void testReASTree() {
   TestReASTree test;
}

