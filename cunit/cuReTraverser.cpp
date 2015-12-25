/*
 * cuReTraverser.cpp
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

class TestReTraverser: public ReTestUnit {
public:
   TestReTraverser() :
      ReTestUnit("ReTraverser", __FILE__),
      m_base(),
      m_buffer(),
      m_logger(ReLogger::globalLogger()) {
      m_base = testDir();
      m_base.append("traverser");
      _mkdir(m_base.str(), ALLPERMS);
      m_base.append(OS_SEPARATOR, -1);
      run();
      ReDirectory::deleteTree(m_base.str(), true);
   }
private:
   ReByteBuffer m_base;
   ReByteBuffer m_buffer;
   ReLogger* m_logger;
private:
   const char* makeDir(const char* relPath) {
      m_buffer = m_base;
      m_buffer.append(relPath);
      m_buffer.replaceAll("/", 1, OS_SEPARATOR, -1);
      _mkdir(m_buffer.str(), ALLPERMS);
      struct stat info;
      if (stat(m_buffer.str(), &info) != 0) {
         logF(true, "cannot create dir %1$s", m_buffer.str());
      }
      return m_buffer.str();
   }
   void makeFile(const char* relPath) {
      ReByteBuffer path(m_base);
      path.append("/").append(relPath);
      path.replaceAll("/", 1, OS_SEPARATOR, -1);
      createFile(path.str(), relPath);
      struct stat info;
      if (stat(path.str(), &info) != 0) {
         logF(true, "cannot create file %1$s", path.str());
      }
   }
   void initTree() {
      makeFile("1.txt");
      makeDir("dir1");
      makeDir("dir2");
      makeDir("dir1/dir1_1");
      makeDir("dir1/dir1_2");
      makeDir("dir1/dir1_2/dir1_2_1");
      makeDir("dir1/cache");
      makeFile("dir1/dir1_2/dir1_2_1/x1.txt");
      makeFile("dir1/dir1_2/dir1_2_1/x2.txt");
      makeFile("dir2/2.x");
      makeFile("dir1/cache/cache.txt");
   }
   void run() {
      testFilter();
      initTree();
      testBasic();
      testList();
   }
   void testFilter() {
      ReDirEntryFilter filter;
   }
   void testList() {
      const char* argv[] = { "list", m_base.str(), NULL };
      ReDirList(m_logger).run(2, argv);
   }
   void testCopyFile() {
#if defined __linux__
      ReByteBuffer src(m_base);
      src.append("dir1/dir1_2/dir1_2_1/x1.txt");
      ReByteBuffer trg(testDir());
      trg.append("copy_x1.txt");
      ReByteBuffer buffer;
      buffer.ensureSize(5);
      ReDirSync::copyFile(src.str(), NULL, trg.str(), buffer,
                          ReLogger::globalLogger());
      checkFileEqu(src.str(), trg.str());
#else
      log(false, "testCopyFile not implemented");
#endif
   }

   void checkRelDate(time_t absTime, int relTime) {
      int diff = int(time(NULL) - relTime - absTime);
      if (diff < 0)
         diff = -diff;
      checkT(diff < 2);
   }

   void checkOneFile(const char* node, const char* parent,
                     const ReHashList& hash) {
      ReByteBuffer path, expected;
      checkT(hash.get(ReByteBuffer(node), path));
      expected.set(parent, -1);
      if (!expected.endsWith(OS_SEPARATOR))
         expected.append(OS_SEPARATOR);
      if (!path.endsWith(expected.str(), -1))
         checkT(false);
   }
   void testBasic() {
      ReTraverser traverser(m_base.str());
      RePatternList patterns;
      // exclude */cache/*
      ReByteBuffer buffer(";*;-cache");
      patterns.set(buffer.str());
      traverser.setDirPattern(&patterns);
      int level = 0;
      ReDirStatus_t* entry;
      ReHashList hashPath;
      ReSeqArray listChanged;
      int state = 0;
      while ((entry = traverser.rawNextFile(level)) != NULL) {
         const char* node = entry->node();
         hashPath.put(ReByteBuffer(node, -1), entry->m_path);
         if (traverser.hasChangedPath(state))
            listChanged.add(-1, node);
         logF(false, "%d: %-12s %2d %s", level, node, int(entry->fileSize()),
              entry->m_path.str());
      }
      checkOneFile("x1.txt", "dir1_2_1", hashPath);
      checkOneFile("x2.txt", "dir1_2_1", hashPath);
      bool changed1 = listChanged.find("x1.txt") != (ReSeqArray::Index) - 1;
      bool changed2 = listChanged.find("x2.txt") != (ReSeqArray::Index) - 1;
      checkT(changed1 != changed2);
      checkOneFile("dir1_2_1", "dir1_2", hashPath);
      checkT(listChanged.find("dir1_2_1") >= 0);
      checkOneFile("dir1_1", "dir1", hashPath);
      checkOneFile("dir1_2", "dir1", hashPath);
      changed1 = listChanged.find("dir1_1") != (ReSeqArray::Index) - 1;
      changed2 = listChanged.find("dir1_2") != (ReSeqArray::Index) - 1;
      checkT(changed1 != changed2);
      checkF(hashPath.get("cache.txt", buffer));
   }
};
extern void testReTraverser(void);

void testReTraverser(void) {
   TestReTraverser unit;
}
