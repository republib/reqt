/*
 * cuReSource.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the input media reader.
 */

#include "base/rebase.hpp"
#include "expr/reexpr.hpp"

class TestReSource: public ReTest {
public:
   TestReSource() :
      ReTest("TestReSource") {
      doIt();
   }

private:
   QByteArray m_content1_1;
   QByteArray m_content1_2;
   QByteArray m_content2;
   ReSource m_source;

protected:
   void init() {
      m_content1_1 = "# test\nimport source2\n";
      m_content1_2 = "a=1;\nveeeeeeeeery looooooooooooooong\n";
      m_content2 = "x=2";
   }

   void testReStringSourceUnit() {
      ReStringReader reader(m_source);
      QByteArray content("a=1;\nveeeeeeeeery looooooooooooooong\n");
      ReStringSourceUnit unit("test", content, &reader);
      unit.setLineNo(144);
      checkEqu(144, unit.lineNo());
      checkEqu("test", unit.name());
   }
   void checkOne(int maxSize, ReReader& reader) {
      QByteArray total;
      QByteArray buffer;
      int lineNo = 0;
      bool hasMore;
      checkF(reader.openSourceUnit("unknownSource"));
      checkT(reader.openSourceUnit("source1"));
      while (reader.nextLine(maxSize, buffer, hasMore)) {
         lineNo++;
         total += buffer;
         buffer.clear();
         while (hasMore && reader.fillBuffer(maxSize, buffer, hasMore)) {
            total += buffer;
            buffer.clear();
         }
         bool isImport = total.endsWith("source2\n");
         if (isImport) {
            reader.openSourceUnit("source2");
            checkEqu("source2", reader.currentSourceUnit()->name());
            while (reader.nextLine(maxSize, buffer, hasMore)) {
               lineNo++;
               while (hasMore
                      && reader.fillBuffer(maxSize, buffer, hasMore)) {
                  total += buffer;
                  buffer.clear();
               }
            }
            checkEqu("source1", reader.currentSourceUnit()->name());
         }
      }
      checkEqu(5, lineNo);
      checkEqu(m_content1_1 + m_content2 + m_content1_2, total);
   }

   void testReStringReader() {
      ReStringReader reader(m_source);
      reader.addSource("source1", m_content1_1 + m_content1_2);
      reader.addSource("source2", m_content2);
      ReSourceUnit* unit = reader.openSourceUnit("source1");
      checkNN(unit);
      checkEqu("source1", unit->name());
      checkEqu(0, unit->lineNo());
      checkOne(6, reader);
      checkOne(100, reader);
      reader.replaceSource("source2", "content2");
      unit = reader.openSourceUnit("source2");
      QByteArray buffer;
      bool hasMore;
      checkT(reader.nextLine(50, buffer, hasMore));
      checkEqu("content2", buffer);
      checkF(hasMore);
   }

public:
   void run(void) {
      init();
      testReStringSourceUnit();
      testReStringReader();
   }
};
void testReSource() {
   TestReSource test;
}

