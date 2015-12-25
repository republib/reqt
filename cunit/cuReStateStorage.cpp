/*
 * cuReStateStorage.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the ReStateStorage tools.
 */
#include "base/rebase.hpp"
#include "gui/regui.hpp"
/**
 * @brief Unit test for <code>ReStateStorage</code>.
 */
class TestReStateStorage: public ReTest {
public:
   TestReStateStorage() :
      ReTest("ReStateStorage") {
      doIt();
   }

public:
   void testBasic() {
      QByteArray fn(ReFile::tempFile("state.basic.$$$.txt", NULL, true));
      unlink(fn);
      {
         ReStateStorage store(fn, &m_logger);
         store.setForm("singles");
         store.store("int", "4711");
         store.store("string", "\"with delimiters\"");
         store.setForm("arrays");
         store.store("int", "111", 0);
         store.store("int", "222", 1);
         store.store("string", "abc", 0);
         store.store("string", "xyz", 1);
         // no explicite close()
      }
      ReStateStorage store(fn, &m_logger);
      store.setForm("singles");
      checkEqu("4711", store.restore("int"));
      checkEqu("\"with delimiters\"", store.restore("string"));
      // unknown key:
      checkEqu("", store.restore("unknown"));
      store.setForm("arrays");
      checkEqu("111", store.restore("int", 0));
      checkEqu("222", store.restore("int", 1));
      // unknown index:
      checkEqu("", store.restore("int", 2));
      checkEqu("abc", store.restore("string", 0));
      checkEqu("xyz", store.restore("string", 1));
      checkEqu("", store.restore("string", 2));
      // unknown index:
      store.close();
   }
   void testAddHistoryEntry() {
      QByteArray fn(ReFile::tempFile("state.hist.$$$.txt", NULL, true));
      {
         ReStateStorage store(fn, &m_logger);
         store.setForm("common");
         store.addHistoryEntry("version", "v4", ';', 3);
         store.addHistoryEntry("version", "v2", ';', 3);
         // form as parameter:
         store.setForm("disturbance");
         store.addHistoryEntry("version", "v3", ';', 3, "common");
         // move position of v2:
         store.addHistoryEntry("version", "v2", ';', 3);
         store.addHistoryEntry("version", "v1", ';', 3);
         store.flushMap();
      }
      ReStateStorage store(fn, &m_logger);
      QStringList list;
      store.setForm("common");
      checkEqu(3, store.historyAsList("version", list).size());
      checkEqu("v1", list.at(0));
      checkEqu("v2", list[1]);
      checkEqu("v3", list[2]);
      // wrong form:
      store.setForm("nothing");
      checkEqu(0, store.historyAsList("version", list).size());
      // form as parameter:
      checkEqu(3, store.historyAsList("version", list, "common").size());
   }
   virtual void runTests() {
      testAddHistoryEntry();
      testBasic();
   }
};

void testReStateStorage() {
   TestReStateStorage test;
}

