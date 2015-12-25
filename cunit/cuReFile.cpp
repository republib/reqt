/*
 * cuReFile.cpp
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

class TestReFile: public ReTest {
public:
   TestReFile() :
      ReTest("ReFile") {
      doIt();
   }

public:
   void testBasic() {
      QByteArray fn(ReFile::tempFile("big.txt", NULL, true));
      const char* content =
         "123456789 123456789 123456789 123456789 123456789\n";
      int contentLength = strlen(content);
      ReFile::writeToFile(fn.constData(), content);
      ReFile file(fn.constData());
      int size = 4;
      file.setBlocksize(2 * size);
      int length = -2;
      char* ptr = file.remap(0, size, length);
      checkNN(ptr);
      checkEqu(4, length);
      checkEqu(0,
               strncmp(content, reinterpret_cast<const char*>(ptr), length));
      int part = size / 2;
      ptr = file.remap(contentLength - part, size, length);
      checkEqu(size / 2, length);
      checkEqu(content + contentLength - part,
               reinterpret_cast<const char*>(ptr));
      for (int ix = 0; ix < contentLength - size; ix++) {
         ptr = file.remap(ix, size, length);
         checkNN(ptr);
         checkEqu(length, size);
         checkEqu(0,
                  strncmp(content + ix, reinterpret_cast<const char*>(ptr),
                          length));
      }
   }
   void testTempFile() {
      QByteArray fn(ReFile::tempFile("node.txt", "subdir", true));
      QByteArray content;
      ReFile::writeToFile(fn, "123");
      struct stat info;
      checkEqu(0, stat(fn.constData(), &info));
      checkEqu(3, (int ) info.st_size);
      ReFile::tempFile("node.txt", "subdir", true);
      checkEqu(-1, stat(fn.constData(), &info));
   }
   void testTempDir() {
      QByteArray dir(ReFile::tempDir("subdir", "curefile", false));
      checkT(dir.endsWith("subdir"));
      checkT(dir.endsWith("curefile/subdir"));
      struct stat info;
      checkEqu(0, stat(dir, &info));
      checkT(S_ISDIR(info.st_mode));
   }
   void testWriteRead() {
      QByteArray fn(ReFile::tempFile("node.txt", "subdir", true));
      ReFile::writeToFile(fn, "123");
      QByteArray content;
      ReFile::readFromFile(fn, content);
      checkEqu("123", content);
      ReFile::writeToFile(fn, "abcdef", 2);
      ReFile::readFromFile(fn, content);
      checkEqu("ab", content);
   }
   void countLinesReFile(const char* filename, int64_t blocksize) {
      clock_t start = clock();
      int lines = 0;
      {
         // enforce the destructor inside measurement:
         ReFile file(filename);
         file.setBlocksize(blocksize);
         int length;
         while (file.nextLine(length)) {
            lines++;
         }
      }
      double duration = double(clock() - start) / CLOCKS_PER_SEC;
      printf("linecount (ReFile, %d kB): %d lines %.3f sec\n",
             int(blocksize / 1024), lines, duration);
   }
   void countLinesFopen(const char* filename) {
      clock_t start = clock();
      FILE* fp = fopen(filename, "r");
      int lines = 0;
      char line[64000];
      if (fp != NULL) {
         while (fgets(line, sizeof line, fp) != NULL)
            lines++;
      }
      fclose(fp);
      double duration = double(clock() - start) / CLOCKS_PER_SEC;
      printf("linecount (fopen): %d lines %.3f sec\n", lines, duration);
   }

   void testPerformance() {
      const char* fn = "/opt/bench/long_html.txt";
      if (QFileInfo(fn).exists()) {
         countLinesReFile(fn, 60 * 1024 * 1024);
         countLinesFopen(fn);
         countLinesReFile(fn, 100 * 1024);
         countLinesReFile(fn, 1024 * 1024);
         countLinesReFile(fn, 10 * 1024 * 1024);
      }
   }
   void testWritableFile() {
      QByteArray fn(ReFile::tempFile("test.txt", "cuReFile", true));
      ReFile::writeToFile(fn, "123\nabc\nxyz");
      ReFile file(fn, false);
      checkEqu(3, file.lineCount());
      checkEqu("123\n", file.lineAt(0));
      checkEqu("abc\n", file.lineAt(1));
      checkEqu("xyz", file.lineAt(2));
      QByteArray fn2(ReFile::tempFile("test2.txt", "cuReFile", true));
      file.write(fn2);
      file.close();
      file.clear();
      checkEqu(0, file.lineCount());
      ReFile file2(fn2, false);
      checkEqu(3, file2.lineCount());
      checkEqu("123", file2.lineAt(0));
      checkEqu("abc", file2.lineAt(1));
      checkEqu("xyz", file2.lineAt(2));
   }
   void testReLinesInsert() {
      ReLines lines;
      // multiple lines in an empty list:
      lines.insertText(0, 0, "123\nabcdefg\nABCDE");
      checkEqu(3, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("abcdefg", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
      // simple string without newlines:
      lines.insertText(1, 2, "xx");
      checkEqu(3, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("abxxcdefg", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
      // no insertText because of wrong parameters:
      lines.insertText(4, 99, "Y");
      checkEqu(3, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("abxxcdefg", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
      // an empty line
      lines.insertText(1, 0, "\n");
      checkEqu(4, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("", lines.lineAt(1));
      checkEqu("abxxcdefg", lines.lineAt(2));
      checkEqu("ABCDE", lines.lineAt(3));
      // 2 lines with text before and after insertText point:
      lines.insertText(2, 5, "Z\nNewline\nY");
      checkEqu(6, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("", lines.lineAt(1));
      checkEqu("abxxcZ", lines.lineAt(2));
      checkEqu("Newline", lines.lineAt(3));
      checkEqu("Ydefg", lines.lineAt(4));
      checkEqu("ABCDE", lines.lineAt(5));
   }
   void testReLinesRemove() {
      ReLines lines;
      lines.insertText(0, 0, "123\nabcdefg\nABCDE");
      checkEqu(3, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("abcdefg", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
      // at line start:
      lines.removePart(0, 0, 2, true);
      checkEqu(3, lines.lineCount());
      checkEqu("3", lines.lineAt(0));
      checkEqu("abcdefg", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
      // at line end (precisely):
      lines.removePart(1, 5, 2, true);
      checkEqu(3, lines.lineCount());
      checkEqu("3", lines.lineAt(0));
      checkEqu("abcde", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
      // at line end (too many chars):
      lines.removePart(1, 3, 99, true);
      checkEqu(3, lines.lineCount());
      checkEqu("3", lines.lineAt(0));
      checkEqu("abc", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
      // no remove because of wrong arguments:
      lines.removePart(-1, 3, 1, true);
      checkEqu(3, lines.lineCount());
      lines.removePart(3, 1, 1, true);
      checkEqu(3, lines.lineCount());
      checkEqu("3", lines.lineAt(0));
      checkEqu("abc", lines.lineAt(1));
      checkEqu("ABCDE", lines.lineAt(2));
   }
   void testRelLinesInsertLines() {
      ReLines lines;
      // inserts into an empty instance:
      lines.insertLines(0, QString("123\nline2-abc\n"), true);
      checkEqu(2, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("line2-abc", lines.lineAt(1));
      // inserts at first line, one line:
      lines.insertLines(0, QString("line-0"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("line-0", lines.lineAt(0));
      checkEqu("123", lines.lineAt(1));
      checkEqu("line2-abc", lines.lineAt(2));
      // inserts in the middle, 2 lines, no '\n' at the end
      lines.insertLines(1, QString("BCDE\nCDEF"), true);
      checkEqu(5, lines.lineCount());
      checkEqu("line-0", lines.lineAt(0));
      checkEqu("BCDE", lines.lineAt(1));
      checkEqu("CDEF", lines.lineAt(2));
      checkEqu("123", lines.lineAt(3));
      checkEqu("line2-abc", lines.lineAt(4));
      // appends at the end, one line, ending with '\n':
      lines.insertLines(6, QString("xyz\n"), true);
      checkEqu(6, lines.lineCount());
      checkEqu("line-0", lines.lineAt(0));
      checkEqu("BCDE", lines.lineAt(1));
      checkEqu("CDEF", lines.lineAt(2));
      checkEqu("123", lines.lineAt(3));
      checkEqu("line2-abc", lines.lineAt(4));
      checkEqu("xyz", lines.lineAt(5));
      // lineno outside:
      lines.insertLines(-1, QString("bad\n"), true);
      checkEqu(6, lines.lineCount());
      lines.insertLines(9999, QString("last\n"), true);
      checkEqu(7, lines.lineCount());
      checkEqu("last", lines.lineAt(6));
   }
   void testRelLinesInsertPart() {
      ReLines lines;
      // inserts into an empty instance:
      lines.insertLines(0, QString("123\nabc\nA"), true);
      // first position:
      lines.insertPart(0, 0, QString("x"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abc", lines.lineAt(1));
      checkEqu("A", lines.lineAt(2));
      // in the middle of the lines, in the middle of the line
      lines.insertPart(1, 2, QString("YY"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abYYc", lines.lineAt(1));
      checkEqu("A", lines.lineAt(2));
      // in the middle of the lines, at the end of the line
      lines.insertPart(1, 5, QString("!?!"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abYYc!?!", lines.lineAt(1));
      checkEqu("A", lines.lineAt(2));
      // last line:
      lines.insertPart(2, 0, QString("xyz"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abYYc!?!", lines.lineAt(1));
      checkEqu("xyzA", lines.lineAt(2));
      // outside of the line range:
      lines.insertPart(-1, 0, QString("wrong"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abYYc!?!", lines.lineAt(1));
      checkEqu("xyzA", lines.lineAt(2));
      lines.insertPart(3, 0, QString("wrong"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abYYc!?!", lines.lineAt(1));
      checkEqu("xyzA", lines.lineAt(2));
      // outside of the column range:
      lines.insertPart(0, -1, QString("wrong"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abYYc!?!", lines.lineAt(1));
      checkEqu("xyzA", lines.lineAt(2));
      lines.insertPart(1, 99, QString("appended"), true);
      checkEqu(3, lines.lineCount());
      checkEqu("x123", lines.lineAt(0));
      checkEqu("abYYc!?!appended", lines.lineAt(1));
      checkEqu("xyzA", lines.lineAt(2));
   }
   void testRelLinesjoinLines() {
      ReLines lines;
      // inserts into an empty instance:
      lines.insertLines(0, QString("123\nabc\nA\nB"), true);
      checkEqu(4, lines.lineCount());
      // inside the lines:
      lines.joinLines(1);
      checkEqu(3, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("abcA", lines.lineAt(1));
      checkEqu("B", lines.lineAt(2));
      // the last two lines:
      lines.joinLines(1);
      checkEqu(2, lines.lineCount());
      checkEqu("123", lines.lineAt(0));
      checkEqu("abcAB", lines.lineAt(1));
      // the first two lines:
      lines.joinLines(0);
      checkEqu(1, lines.lineCount());
      checkEqu("123abcAB", lines.lineAt(0));
   }
   void testReLinesSplitLine() {
      ReLines lines;
      // inserts into an empty instance:
      lines.insertLines(0, QString("123\nabcdefg"), true);
      // in the middle of the line:
      lines.splitLine(0, 1, true);
      checkEqu(3, lines.lineCount());
      checkEqu("1", lines.lineAt(0));
      checkEqu("23", lines.lineAt(1));
      checkEqu("abcdefg", lines.lineAt(2));
      // at the end of the line ""1\n23\abcdefg":
      lines.splitLine(0, 2, true);
      checkEqu(4, lines.lineCount());
      checkEqu("1", lines.lineAt(0));
      checkEqu("", lines.lineAt(1));
      checkEqu("23", lines.lineAt(2));
      checkEqu("abcdefg", lines.lineAt(3));
      lines.clear();
      lines.insertLines(0, QString("12"), true);
      // in the middle of the last line:
      lines.splitLine(0, 1, true);
      checkEqu(2, lines.lineCount());
      checkEqu("1", lines.lineAt(0));
      checkEqu("2", lines.lineAt(1));
      // in the end of the last line:
      lines.splitLine(1, 99, true);
      checkEqu(3, lines.lineCount());
      checkEqu("1", lines.lineAt(0));
      checkEqu("2", lines.lineAt(1));
      checkEqu("", lines.lineAt(2));
   }
   virtual void runTests() {
      testReLinesInsert();
      testReLinesSplitLine();
      testRelLinesjoinLines();
      testRelLinesInsertPart();
      testRelLinesInsertLines();
      testReLinesRemove();
      testWritableFile();
      testPerformance();
      testBasic();
   }
};
void testReFile() {
   TestReFile test;
}

