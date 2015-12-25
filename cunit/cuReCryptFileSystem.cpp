/*
 * cuReCryptFileSystem.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"
#include "os/reos.hpp"

/** @file
 * @brief Unit test of the basic exceptions.
 */

class MyReCryptFileSystem : private ReCryptFileSystem, private ReTest {
public:
   MyReCryptFileSystem(ReLocalFileSystem& hostFs, ReRandomizer& random,
                       ReLogger* logger) :
      ReCryptFileSystem(hostFs, random, logger),
      ReTest("MyReCryptFileSystem") {
   }
public:
   virtual void runTests(void) {
      log("run");
   }

   void testDirRead() {
      readMetaFile();
      ReFileMetaDataList list;
      checkEqu(3, listInfos(ReIncludeExcludeMatcher::allMatcher(), list));
      ReFileMetaData entry = list.at(0);
      checkEqu("Homunculus.txt", entry.m_node);
      entry = list.at(1);
      checkEqu("NewYork.png", entry.m_node);
      entry = list.at(2);
      checkEqu("tiger.in.india.mov", entry.m_node);
   }
   void testDirWrite() {
      addFile("Homunculus.txt");
      addFile("NewYork.png");
      addFile("tiger.in.india.mov");
      writeMetaFile();
   }

};

class TestReCryptFileSystem: public ReTest {
public:
   TestReCryptFileSystem() :
      ReTest("ReCryptFileSystem"),
      m_sourceBase(),
      m_hostBase(),
      m_sourceFs(NULL),
      m_hostFs(NULL),
      m_cryptFs(NULL) {
      doIt();
   }
   ~TestReCryptFileSystem() {
      destroy();
   }

private:
   QByteArray m_sourceBase;
   QByteArray m_hostBase;
   ReLocalFileSystem* m_sourceFs;
   ReLocalFileSystem* m_hostFs;
   ReCryptFileSystem* m_cryptFs;
   ReKISSRandomizer m_contentRandom;
protected:
   void init() {
      m_hostBase = ReFileUtils::tempDir("cryptfs");
      m_sourceBase = ReFileUtils::tempDir("sourcefs");
      m_hostFs = new ReLocalFileSystem("/", &m_logger);
      m_hostFs->setDirectory(m_hostBase);
      m_cryptFs = new ReCryptFileSystem(*m_hostFs, m_contentRandom, &m_logger);
   }
   void destroy() {
      delete m_sourceFs;
      delete m_hostFs;
      delete m_cryptFs;
      m_sourceFs = NULL;
      m_hostFs = NULL;
      m_cryptFs = NULL;
   }
   void testDirWriteRead() {
      MyReCryptFileSystem cryptFs1(*m_hostFs, m_contentRandom, &m_logger);
      cryptFs1.testDirWrite();
      MyReCryptFileSystem cryptFs2(*m_hostFs, m_contentRandom, &m_logger);
      cryptFs2.testDirRead();
   }

   virtual void runTests() {
      init();
      testDirWriteRead();
      destroy();
   }
};
void testReCryptFileSystem() {
   TestReCryptFileSystem test;
}

