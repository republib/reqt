/*
 * cuReFileUtils.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"

/** @file
 * @brief Unit test of the basic exceptions.
 */

class TestReFileUtils: public ReTest {
public:
   TestReFileUtils() :
      ReTest("ReFileUtils") {
      doIt();
   }

public:
   void testTempFile() {
      QByteArray fn(ReFileUtils::tempFile("node.txt", "subdir", true));
      QByteArray content;
      ReFileUtils::writeToFile(fn, "123");
      struct stat info;
      checkEqu(0, stat(fn.constData(), &info));
      checkEqu(3, (int ) info.st_size);
      ReFileUtils::tempFile("node.txt", "subdir", true);
      checkEqu(-1, stat(fn.constData(), &info));
   }
   void testTempDir() {
      QByteArray dir(ReFileUtils::tempDir("subdir", "cuReFileUtils", false));
      checkT(dir.endsWith("subdir"));
      checkT(dir.endsWith("cuReFileUtils/subdir"));
      struct stat info;
      checkEqu(0, stat(dir, &info));
      checkT(S_ISDIR(info.st_mode));
   }
   void testTempDirEmpty() {
      QByteArray dir(
         ReFileUtils::tempDirEmpty("subdir2", "cuReFileUtils", true));
      QByteArray subdir(dir);
      subdir.append("subdirX");
      mkdir(subdir.constData(), ALLPERMS);
      struct stat info;
      checkEqu(0, stat(subdir.constData(), &info));
      ReFileUtils::tempDirEmpty("subdir2", "cuReFileUtils", true);
      checkEqu(-1, stat(subdir.constData(), &info));
   }
   void testWriteRead() {
      QByteArray fn(ReFileUtils::tempFile("node.txt", "subdir", true));
      ReFileUtils::writeToFile(fn, "123");
      QByteArray content;
      ReFileUtils::readFromFile(fn, content);
      checkEqu("123", content);
      ReFileUtils::writeToFile(fn, "abcdef", 2);
      ReFileUtils::readFromFile(fn, content);
      checkEqu("ab", content);
   }
   QByteArray buildTree() {
      QByteArray base = ReFileUtils::tempDir("ReFileUtils");
      for (char cc = 'a'; cc < 'f'; cc++) {
         QByteArray subdir(base + cc);
         mkdir(subdir.constData(), ALLPERMS);
         for (char cc2 = '1'; cc2 < '5'; cc2++) {
            QByteArray name(subdir);
            name.append(OS_SEPARATOR_STR).append(&cc2, 1);
            ReFileUtils::writeToFile(name, name);
            name += "dir";
            mkdir(name.constData(), ALLPERMS);
            name.append(OS_SEPARATOR_STR).append("x.txt");
            ReFileUtils::writeToFile(name, name);
         }
      }
      // remove the separator:
      base.remove(base.length() - 1, 1);
      return base;
   }
   void testDeleteTree() {
      QByteArray base = buildTree();
      checkT(ReFileUtils::deleteTree(QString(base), false, &m_logger));
      struct stat info;
      // the dir must exist:
      checkEqu(0, stat(base, &info));
      // rmdir() works only if the dir is empty:
      checkEqu(0, rmdir(base));
      buildTree();
      checkT(ReFileUtils::deleteTree(QString(base), false, &m_logger));
      checkEqu(0, stat(base, &info));
   }
   void testIsAbsolutePath() {
#ifdef __linux__
      checkT(ReFileUtils::isAbsolutPath("/abc/def/xyz.123"));
      checkT(ReFileUtils::isAbsolutPath("/"));
      checkF(ReFileUtils::isAbsolutPath("../abc/x.y"));
      checkF(ReFileUtils::isAbsolutPath("./abc"));
#else
      checkT(ReFileUtils::isAbsolutPath("e:\\abc\\def\\xyz.123"));
      checkT(ReFileUtils::isAbsolutPath("a:\\"));
      checkF(ReFileUtils::isAbsolutPath("e:\\abc\\def\\xyz.123"));
      checkF(ReFileUtils::isAbsolutPath("a:\\"));
#endif
      checkF(ReFileUtils::isAbsolutPath(""));
   }
   void testSeekTell() {
      QByteArray fn(ReFileUtils::tempFile("seektest.txt", NULL, false));
      ReFileUtils::writeToFile(fn.constData(), "0123456789");
      FILE* fp = fopen(fn.constData(), "rb");
      checkNN(fp);
      if (fp != NULL) {
         checkEqu(0LL, ReFileUtils::tell(fp));
         checkEqu(0, ReFileUtils::seek(fp, 3, SEEK_SET));
         checkEqu(3LL, ReFileUtils::tell(fp));
         char cc;
         checkEqu(1, fread(&cc, 1, 1, fp));
         checkEqu('3', cc);
         checkEqu(4LL, ReFileUtils::tell(fp));
         checkEqu(0, ReFileUtils::seek(fp, -2, SEEK_CUR));
         checkEqu(2LL, ReFileUtils::tell(fp));
         checkEqu(0, ReFileUtils::seek(fp, -2, SEEK_END));
         checkEqu(8LL, ReFileUtils::tell(fp));
      }
   }
   void testSetTimes() {
      QByteArray fn(ReFileUtils::tempFile("timetest.txt", NULL, true));
      ReFileUtils::writeToFile(fn.constData(), "");
      QDateTime time = QDateTime::fromString("03.09.2015 07:14:24.432",
                                             "dd.MM.yyyy hh:mm:ss.zzz");
      checkT(
         ReFileUtils::setTimes(fn.constData(), time,
                               ReFileUtils::m_undefinedTime, &m_logger));
      QFileInfo info(fn);
      checkEqu(time, info.lastModified());
   }
   void testCleanPath() {
#if defined __linux__
      // no change:
      checkEqu("/x/y/z.x", ReFileUtils::cleanPath("/x/y/z.x"));
      checkEqu("x/y/z.x", ReFileUtils::cleanPath("./x/y/z.x"));
      checkEqu("x/y/z.x/", ReFileUtils::cleanPath("x/y/z.x/"));
      // remove duplicated slashes:
      checkEqu("x/y/z.x/", ReFileUtils::cleanPath("x//y/////z.x//"));
      // remove "./"
      checkEqu("x/y/z.x", ReFileUtils::cleanPath("./x/././y/z.x"));
      checkEqu("/x/y/z.x", ReFileUtils::cleanPath("/x/././y/z.x"));
      // remove "..":
      // inside...
      checkEqu("x/a/b", ReFileUtils::cleanPath("x/y/../a/b"));
      checkEqu("x/y/a/b", ReFileUtils::cleanPath("x/y/z/../a/b"));
      checkEqu("x/a/b", ReFileUtils::cleanPath("x/y/z/../../a/b"));
      // at the end..
      checkEqu("x", ReFileUtils::cleanPath("x/y/z/../.."));
      checkEqu("x/", ReFileUtils::cleanPath("x/y/z/../../"));
      // wrong forms:
      checkEqu("..", ReFileUtils::cleanPath(".."));
      checkEqu("../..", ReFileUtils::cleanPath("../.."));
      checkEqu("..", ReFileUtils::cleanPath("../x/.."));
#elif defined __WIN32__
#error "not implemented"
#endif
   }
   void checkExtensionOf(const char* expected, const char* arg1, int lineNo) {
      assertEquals(QString(expected), ReFileUtils::extensionOf(QString(arg1)),
                   __FILE__, lineNo);
      assertEquals(expected, ReFileUtils::extensionOf(QString(arg1)),
                   __FILE__, lineNo);
      QByteArray exp(expected);
      QByteArray sArg1(arg1);
      exp.replace("/", "\\");
      sArg1.replace("/", "\\");
      assertEquals(QString(exp), ReFileUtils::extensionOf(QString(sArg1)),
                   __FILE__, lineNo);
      assertEquals(exp.constData(),
                   ReFileUtils::extensionOf(sArg1.constData()),
                   __FILE__, lineNo);
   }

   void testExtensionOf() {
      checkExtensionOf("", "", __LINE__);
      // full path
      checkExtensionOf(".x", "/abc/def.x", __LINE__);
      // prior node with extension too:
      checkExtensionOf(".xyz", "/abc.z/def.xyz", __LINE__);
      // sizeof (ext) == 2:
      checkExtensionOf(".x", "/abc.z/def.xx.x", __LINE__);
      // sizeof (ext) == 1:
      checkExtensionOf(".x", "def.x", __LINE__);
      checkExtensionOf(".", "def.", __LINE__);
      // empty extension:
      checkExtensionOf("", "/abc.x/def", __LINE__);
      // empty last node:
      checkExtensionOf("", "/abc.x/", __LINE__);
      // node starting with "."
      checkExtensionOf("", "/abc.x/.x", __LINE__);
      checkExtensionOf("", ".xyz", __LINE__);
      // Special:
      checkExtensionOf("", "", __LINE__);
   }
   void checkNodeOf(const char* expected, const char* toTest, int lineNo) {
      assertEquals(QString(expected), ReFileUtils::nodeOf(QString(toTest)),
                   __FILE__, lineNo);
      assertEquals(expected, ReFileUtils::nodeOf(QString(toTest)),
                   __FILE__, lineNo);
      QByteArray exp(expected);
      QByteArray toTst(toTest);
      exp.replace("/", "\\");
      toTst.replace("/", "\\");
      assertEquals(QString(exp), ReFileUtils::nodeOf(QString(toTst)),
                   __FILE__, lineNo);
      assertEquals(exp, ReFileUtils::nodeOf(toTst.constData()),
                   __FILE__, lineNo);
   }

   void testNodeOf() {
      checkNodeOf("abc.def", "/abc.def", __LINE__);
      checkNodeOf("abc.def", "abc.def", __LINE__);
      checkNodeOf("abc.def", "x/y/abc.def", __LINE__);
      checkNodeOf("abc", "x/y/abc", __LINE__);
      checkNodeOf("", "", __LINE__);
   }

   void testParentOf() {
      checkEqu("/abc/", ReFileUtils::parentOf("/abc/def"));
      checkEqu("/abc/def/x.y/", ReFileUtils::parentOf("/abc/def/x.y/"));
      checkEqu("/", ReFileUtils::parentOf("/"));
      checkEqu("", ReFileUtils::parentOf("abc.def"));
   }

   void checkPathAppend(const char* expected, const char* arg1,
                        const char* arg2, int lineNo) {
      assertEquals(QString(expected),
                   ReFileUtils::pathAppend(QString(arg1), QString(arg2)),
                   __FILE__, lineNo);
      assertEquals(expected, ReFileUtils::pathAppend(arg1, arg2),
                   __FILE__, lineNo);
      QByteArray exp(expected);
      QByteArray sArg1(arg1);
      QByteArray sArg2(arg2);
      exp.replace("/", "\\");
      sArg1.replace("/", "\\");
      sArg2.replace("/", "\\");
      assertEquals(QString(exp),
                   ReFileUtils::pathAppend(QString(sArg1), QString(sArg2)),
                   __FILE__, lineNo);
      assertEquals(exp,
                   ReFileUtils::pathAppend(sArg1.constData(), sArg2.constData()),
                   __FILE__, lineNo);
   }
   void testPathAppend() {
      // no base:
      checkPathAppend("abc/def", "", "abc/def", __LINE__);
      checkPathAppend("abc/def", NULL, "abc/def", __LINE__);
      // abs path:
      checkPathAppend("/abc/def", "", "/abc/def", __LINE__);
      checkPathAppend("/abc/def", NULL, "/abc/def", __LINE__);
      checkPathAppend("/abc/def", "xyz", "/abc/def", __LINE__);
      // true combination:
      checkPathAppend("/abc/bef", "/abc", "bef", __LINE__);
      checkPathAppend("/abc/bef", "/abc/", "bef", __LINE__);
      checkPathAppend("/abc", "/", "bef", __LINE__);
   }
   void checkReplaceExt(const char* expected, const char* arg1,
                        const char* arg2, int lineNo) {
      assertEquals(QString(expected),
                   ReFileUtils::replaceExtension(QString(arg1), QString(arg2)),
                   __FILE__, lineNo);
      assertEquals(expected, ReFileUtils::replaceExtension(arg1, arg2),
                   __FILE__, lineNo);
      QByteArray exp(expected);
      QByteArray sArg1(arg1);
      QByteArray sArg2(arg2);
      exp.replace("/", "\\");
      sArg1.replace("/", "\\");
      sArg2.replace("/", "\\");
      assertEquals(QString(exp),
                   ReFileUtils::replaceExtension(QString(sArg1), QString(sArg2)),
                   __FILE__, lineNo);
      assertEquals(exp,
                   ReFileUtils::replaceExtension(sArg1.constData(), sArg2.constData()),
                   __FILE__, lineNo);
   }
   void testReplaceExtension() {
      checkReplaceExt("/abc/def.123", "/abc/def.xyz", ".123", __LINE__);
      checkReplaceExt("def.123", "def.xyz", ".123", __LINE__);
      checkReplaceExt("/abc.1/def.123", "/abc.1/def.xyz", ".123", __LINE__);
      checkReplaceExt("/abc.1/def.123", "/abc.1/def", ".123", __LINE__);
   }
   void checkUrl(const char* url, const char* expProto, const char* expHost,
                 const char* expPath, const char* expNode, const char* expParams) {
      QString protocol, host, path, node, params;
      ReFileUtils::splitUrl(QString(url), &protocol, &host, &path, &node, &params);
      checkEqu(expProto, protocol);
      checkEqu(expHost, host);
      checkEqu(expPath, path);
      checkEqu(expNode, node);
      checkEqu(expParams, params);
   }
   void testSplitUrl() {
      checkUrl("file:///abc/def.x", "file:", "//", "/abc/", "def.x", "");
      checkUrl("file:/abc/def.x", "file:", "", "/abc/", "def.x", "");
      checkUrl("///abc/def.x", "", "//", "/abc/", "def.x", "");
      checkUrl("/abc/def.x", "", "", "/abc/", "def.x", "");
      checkUrl("/def.x", "", "", "/", "def.x", "");
      checkUrl("def.x", "", "", "", "def.x", "");
      checkUrl("http://b2.de/public/index.htm?id=1", "http:", "//b2.de", "/public/",
               "index.htm", "?id=1");
      checkUrl("http://b2.de/index.htm?id=1", "http:", "//b2.de", "/",
               "index.htm", "?id=1");
      checkUrl("http:index.htm?id=1", "http:", "", "",
               "index.htm", "?id=1");
      checkUrl("http:index.htm", "http:", "", "",
               "index.htm", "");
   }

   virtual void runTests() {
      testSplitUrl();
      testParentOf();
      testCleanPath();
      testReplaceExtension();
      testNodeOf();
      testExtensionOf();
      testSetTimes();
      testSeekTell();
      testIsAbsolutePath();
      testDeleteTree();
      testTempDir();
      testTempFile();
      testWriteRead();
   }
};
void testReFileUtils() {
   TestReFileUtils test;
}

