/*
 * cuReFileSystem.cpp
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
class TestReFileSystem: public ReTest {
public:
   TestReFileSystem() :
      ReTest("ReFileSystem") {
      doIt();
   }
private:
   QByteArray m_base;
   QByteArray m_subDir1;
protected:
   void init() {
      m_base = ReFileUtils::tempDir("refilesystem");
      m_subDir1 = ReFileUtils::tempDir("dir1", "refilesystem");
      ReFileUtils::tempDir("dir2", "refilesystem");
      QString node;
      for (int ix = 1; ix <= 7; ix++) {
         node.sprintf("test%d.txt", ix);
         QByteArray fn = ReFileUtils::tempFile(I18N::s2b(node).constData(),
                                               "refilesystem", false);
         ReFileUtils::writeToFile(fn.constData(), I18N::s2b(node).constData());
         fn = m_subDir1;
         fn.append("text").append(QByteArray::number(ix));
         ReFileUtils::writeToFile(fn.constData(), fn.constData());
      }
      ReFileUtils::tempFile("abc.txt", "refilesytem", true);
   }
   void testContains(const char* name, ReFileMetaDataList nodes) {
      bool rc = false;
      ReFileMetaDataList::const_iterator it;
      for (it = nodes.cbegin(); it != nodes.cend(); ++it) {
         if ((*it).m_node == name)
            rc = true;
      }
      if (!rc)
         checkT(rc);
   }

   void testReListInfos() {
      ReLocalFileSystem fs(m_base, &m_logger);
      checkEqu(QString(m_base), fs.directory());
      checkEqu(QString(m_base), fs.basePath());
      ReFileMetaDataList nodes;
      ReIncludeExcludeMatcher matcher(ReListMatcher::allMatchingList(),
                                      ReQStringUtils::m_emptyList, Qt::CaseInsensitive, false);
      fs.listInfos(matcher, nodes, ReFileSystem::LO_UNDEF);
      testContains("dir1", nodes);
      testContains("test1.txt", nodes);
      testContains("test7.txt", nodes);
   }
   void testReadWrite() {
      ReLocalFileSystem fs(m_base, &m_logger);
      QByteArray buffer;
      const char* node1 = "abc.txt";
      buffer.append("abcdefghijklmnopqrstuvwxyz");
      QByteArray full(fs.fullNameAsUTF8(QString(node1)));
      ReFileUtils::writeToFile(full.constData(), buffer.constData());
      QByteArray buffer2;
      ReFileMetaDataList nodes;
      QStringList names;
      names.append(node1);
      names.append("not_exists.txt");
      ReIncludeExcludeMatcher matcher(names, ReQStringUtils::m_emptyList,
                                      Qt::CaseInsensitive, true);
      checkEqu(1, fs.listInfos(matcher, nodes, ReFileSystem::LO_FILES));
      checkEqu(1, nodes.size());
      QByteArray content("This is a content\nLine 2");
      const char* node2 = "new.txt";
      ensureNotExist(fs.fullNameAsUTF8(node2));
      checkEqu(0, fs.createFile(node2, false));
      ReFileMetaData meta;
      checkT(fs.exists(node1, &meta));
      ReLeafFile* leaf1 = fs.buildFile(meta);
      checkEqu(0, leaf1->open(true));
      checkEqu(0, leaf1->write(content));
      checkEqu(0, leaf1->close());
      delete leaf1;
      checkT(fs.exists(node1, &meta));
      ReLeafFile* leaf2 = fs.buildFile(meta);
      checkEqu(0, leaf2->open(false));
      checkEqu(0, leaf2->read(500, buffer));
      checkEqu(0, leaf2->close());
      checkEqu(content.constData(), buffer);
      checkEqu(0, leaf2->open(false));
      checkEqu(0, leaf2->read(3, buffer));
      checkEqu(content.mid(0, 3).constData(), buffer);
      checkEqu(0, leaf2->read(8, buffer));
      checkEqu(content.mid(3, 8).constData(), buffer);
      checkEqu(0, leaf2->close());
      delete leaf2;
   }
   void testSetProperties() {
      ReLocalFileSystem fs(m_base, &m_logger);
      ReFileMetaData meta1;
      const char* node1 = "later.txt";
      const char* node2 = "properties.txt";
      QByteArray full(fs.fullNameAsUTF8(node1));
      ensureNotExist(full);
      full = fs.fullNameAsUTF8(node2);
      ReFileUtils::writeToFile(full, node2);
      ReFileUtils::tempFile(node1, "refilesystem", true);
      QDateTime modified = QDateTime::fromString("2015.09.12 11:44:55.765",
                           "yyyy.MM.dd hh:mm:ss.zzz");
      ReFileMetaData meta2(node1, modified, ReFileUtils::m_undefinedTime,
                           -1, -1, (mode_t) - 1, 1);
      checkT(fs.first(node2, meta1, ReFileSystem::LO_FILES));
      checkEqu(0, fs.setProperties(meta2, meta1, true));
      ReFileMetaData meta3;
      checkT(fs.first(node1, meta3));
      checkEqu(meta3.m_modified, modified);
   }
   void testSetPropertiesOwner() {
#ifdef __linux__
      if (geteuid() == 0) {
         ReLocalFileSystem fs(m_base, &m_logger);
         ReFileMetaData meta1;
         ReFileUtils::tempFile("later2.txt", "refilesystem", true);
         int rights = S_IRUSR | S_IWGRP | S_IRGRP| S_IROTH | S_IWOTH;
         QDateTime modified = QDateTime::fromString("2015.08.13 10:34:55.765",
                              "yyyy.MM.dd hh:mm:ss:zzz");
         ReFileMetaData meta2("later2.txt",
                              modified, ReFileUtils::m_undefinedTime,
                              1001, 1002, rights, 1);
         checkT(fs.first("test2.txt", meta1));
         checkEqu(0, fs.setProperties(meta2, meta1, true));
         ReFileMetaData meta3;
         checkT(fs.first("later2.txt", meta3));
         checkEqu(meta3.m_modified, modified);
         checkEqu(meta3.m_owner, 1001);
         checkEqu(meta3.m_group, 1002);
         checkEqu(meta3.m_mode & ALLPERMS, rights);
      }
#endif
   }
   void compareMeta(ReFileMetaData& meta1, ReFileMetaData& meta2) {
      checkEqu(meta1.m_node, meta2.m_node);
      checkEqu(meta1.m_modified, meta2.m_modified);
      checkEqu((int ) meta1.m_mode, (int ) meta2.m_mode);
      checkEqu(meta1.m_group, meta2.m_group);
      checkEqu(meta1.m_owner, meta2.m_owner);
      checkEqu(meta1.m_size, meta2.m_size);
   }

   void testCopy() {
      ReLocalFileSystem fsSource(m_base, &m_logger);
      QByteArray base2 = ReFileUtils::tempDir("refilesystem.trg", NULL,
                                              false);
      ReFileUtils::deleteTree(base2, false, &m_logger);
      ReLocalFileSystem fsTarget(base2, &m_logger);
      ReFileMetaData metaSource;
      const char* node = "test3.txt";
      checkT(fsSource.first(node, metaSource));
      QByteArray full(fsTarget.fullNameAsUTF8(node));
      ensureNotExist(full.constData());
      checkEqu(0, fsTarget.copy(metaSource, fsSource));
      ReFileMetaData metaTarget;
      checkT(fsTarget.first(node, metaTarget));
      compareMeta(metaSource, metaTarget);
      checkEqu(0, fsTarget.makeDir("dir.01"));
      checkEqu(0, fsTarget.setDirectory("dir.01"));
      checkEqu(0, fsTarget.copy(metaSource, fsSource));
      QString path = fsTarget.fullName(metaSource.m_node);
      checkT(path.indexOf("dir.01"));
      struct stat info;
      checkEqu(0, stat(I18N::s2b(path).constData(), &info));
   }
   void testReOSPermissions() {
      ReOSPermissions p1;
      int owner, group;
#if defined __linux__
      owner = getuid();
      group = getgid();
#else
      owner = -1;
      group = -1;
#endif
      checkEqu(owner, p1.m_user);
      checkEqu(group, p1.m_group);
      checkEqu( S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH, p1.m_fileMode);
      checkEqu(S_IWUSR | S_IRUSR | S_IXUSR | S_IWGRP | S_IRGRP
               | S_IXGRP | S_IROTH | S_IXOTH | __S_IFDIR, p1.m_dirMode);
      ReOSPermissions p2(p1);
      checkEqu(owner, p2.m_user);
      checkEqu(group, p2.m_group);
      checkEqu( S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH, p2.m_fileMode);
      checkEqu(S_IWUSR | S_IRUSR | S_IXUSR | S_IWGRP | S_IRGRP
               | S_IXGRP | S_IROTH | S_IXOTH | __S_IFDIR, p2.m_dirMode);
      p2.m_user = 0x4711;
      p2.m_group = 0x1147;
      p2.m_dirMode = 123;
      p2.m_fileMode = 7766;
      p1 = p2;
      checkEqu(0x4711, p2.m_user);
      checkEqu(0x1147, p2.m_group);
      checkEqu(123, p2.m_dirMode);
      checkEqu(7766, p2.m_fileMode);
   }
   void checkMove(const char* node1, const char* node2) {
      ReLocalFileSystem fsSource(m_base, &m_logger);
      QByteArray base2 = ReFileUtils::tempDir("refilesystem.trg", NULL,
                                              false);
      ReFileUtils::deleteTree(base2, false, &m_logger);
      ReLocalFileSystem fsTarget(base2, &m_logger);
      ReFileMetaData metaSource;
      const char* content = "content move1.txt";
      QByteArray fullSrc(fsSource.fullNameAsUTF8(node1));
      ReFileUtils::writeToFile(fullSrc, content);
      checkT(fsSource.first(node1, metaSource));
      QByteArray fullTrg(fsTarget.fullNameAsUTF8(node2 == NULL ? node1 : node2));
      ensureNotExist(fullTrg.constData());
      checkT(fsSource.exists(node1, &metaSource));
      if (node2 == NULL)
         checkEqu(0, fsTarget.move(metaSource, fsSource));
      else
         checkEqu(0, fsTarget.move(metaSource, fsSource, node2));
      checkF(exists(fullSrc));
      checkT(exists(fullTrg));
      checkF(fsSource.exists(node1));
      checkT(fsTarget.exists(node2 == NULL ? node1 : node2));
      QByteArray buffer;
      checkEqu(content, ReFileUtils::readFromFile(fullTrg, buffer));
   }

   void testMove() {
      checkMove("move1.txt", "move2.txt");
      checkMove("move1.txt", NULL);
   }

   virtual void runTests() {
      testReOSPermissions();
      init();
      testReListInfos();
      testSetProperties();
      testSetPropertiesOwner();
      testCopy();
      testReadWrite();
      testMove();
   }
};
void testReFileSystem() {
   TestReFileSystem test;
}

