/*
 * cuReVM.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

class TestReVM: public ReTest {
private:
   ReSource m_source;
   ReASTree m_tree;
   ReStringReader m_reader;
   const char* m_currentSource;
public:
   TestReVM() :
      ReTest("ReVM"),
      m_source(),
      m_tree(),
      m_reader(m_source) {
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

private:
   void checkAST(const char* fileExpected, int lineNo) {
      QByteArray fnExpected = "test";
      fnExpected += QDir::separator().toLatin1();
      fnExpected += "ReVM";
      fnExpected += (char) QDir::separator().toLatin1();
      fnExpected += fileExpected;
      QByteArray fnCurrent = getTempFile(fileExpected, "ReVM");
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      ReVirtualMachine vm(m_tree, m_source);
      vm.setFlag(ReVirtualMachine::VF_TRACE_STATEMENTS);
      ReFileWriter writer(fnCurrent);
      vm.setTraceWriter(&writer);
      writer.write(m_currentSource);
      vm.executeModule("<test>");
      assertEqualFiles(fnExpected.constData(), fnCurrent.constData(),
                       __FILE__, lineNo);
   }
public:
   void baseTest() {
      setSource("Int a=2+3*4;\nfunc Void main():\na;\nendf");
      checkAST("baseTest.txt", __LINE__);
   }
   virtual void run(void) {
      baseTest();
   }
};
void testReVM() {
   TestReVM test;
}

