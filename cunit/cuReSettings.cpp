/*
 * cuReSettings.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

/** @file
 * @brief Unit test of the ReSettings tools.
 */
#include "base/rebase.hpp"
#include "gui/regui.hpp"
/**
 * @brief Unit test for <code>ReSettings</code>.
 */
class TestReSettings: public ReTest {
public:
   TestReSettings() :
      ReTest("ReSettings") {
      doIt();
   }

public:
   void testAddHistoryEntry() {
      QByteArray dir(ReFile::tempDir("resettings", NULL, false));
      ReFile::deleteTree((QString) dir, false, &m_logger);
      {
         ReSettings settings(dir, "test", &m_memoryLogger);
         settings.addHistoryEntry("digits", "4", ' ', 3);
         checkT(logContains("cannot open .*resettings.test\\.history: \\d+"));
         settings.addHistoryEntry("digits", "3", ' ', 3);
         settings.addHistoryEntry("digits", "2", ' ', 3);
         settings.addHistoryEntry("digits", "1", ' ', 3);
         m_memoryAppender.clear();
      }
      ReSettings settings(dir, "test", &m_logger);
      QStringList list;
      settings.historyAsList("digits", list);
      checkEqu("1 2 3", list.join(' '));
   }
   void setSettings(ReSettings& settings) {
      settings.insertProperty(
         new ReProperty("level1.boolVal", "Boolean",
                        "Boolean value for test", ReSettings::TRUE, PT_BOOL));
      settings.insertProperty(
         new ReProperty("level1.intVal", "Integer", "Integer value for test",
                        "4711", PT_INT, "[0,9999]"));
      settings.insertProperty(
         new ReProperty("level2.strVal", "String", "String value for test",
                        "crazy horse", PT_STRING));
   }

   void testBasic() {
      QByteArray dir(ReFile::tempDir("resettings", NULL, false));
      ReFile::deleteTree((QString) dir, false, &m_logger);
      ReSettings settings(dir, "test", &m_memoryLogger);
      setSettings(settings);
      settings.writeSettings();
      checkT(settings.boolValue("level1.boolVal"));
      checkEqu(4711, settings.intValue("level1.intVal"));
      checkEqu("crazy horse", settings.stringValue("level2.strVal"));
      // unknown names:
      checkF(settings.boolValue("level2.boolVal"));
      logContains("level2.boolValue");
      m_memoryAppender.clear();
      checkEqu(0, settings.intValue("level2.intVal"));
      logContains("level2.intVal");
      m_memoryAppender.clear();
      checkEqu("", settings.stringValue("strVal"));
      logContains("stringVal");
      m_memoryAppender.clear();
      ReSettings settings2(dir, "test", &m_memoryLogger);
      setSettings(settings2);
      settings2.changeValue("level1.boolVal", ReSettings::FALSE);
      settings2.changeValue("level1.intVal", "1234");
      settings2.changeValue("level2.strVal", "pretty woman");
      settings2.writeSettings();
      settings.readSettings();
      checkF(settings.boolValue("level1.boolVal"));
      checkEqu(1234, settings.intValue("level1.intVal"));
      checkEqu("pretty woman", settings.stringValue("level2.strVal"));
   }

   void testTopOfHistory() {
      QByteArray dir(ReFile::tempDir("resettings", NULL, false));
      ReFile::deleteTree((QString) dir, false, &m_logger);
      {
         ReSettings settings(dir, "test", &m_memoryLogger);
         settings.addHistoryEntry("fluid", "beer", ' ', 3);
         checkT(logContains("cannot open .*resettings.test\\.history: \\d+"));
         settings.addHistoryEntry("fluid", "wine", ' ', 3);
         m_memoryAppender.clear();
      }
      ReSettings settings(dir, "test", &m_logger);
      checkEqu("wine", settings.topOfHistory("fluid"));
      checkEqu("???", settings.topOfHistory("unknown", "???"));
   }

   virtual void runTests() {
      testTopOfHistory();
      testBasic();
      testAddHistoryEntry();
   }
};

void testReSettings() {
   TestReSettings test;
}

