/*
 * cuReBench.cpp
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

class TestRplBenchmark: public ReTest {
private:
   const char* m_filename;
   ReSource m_source;
   ReFileReader m_reader;
   ReASTree m_tree;
public:
   TestRplBenchmark() :
      ReTest("RplBenchmark"),
      m_filename("/home/ws/qt/rplqt/bench/mfbench.mf"),
      m_source(),
      m_reader(m_source),
      m_tree() {
      m_source.addReader(&m_reader);
      m_reader.addSource(m_filename);
   }
public:
   void benchmark() {
      time_t start = time(NULL);
      ReMFParser parser(m_source, m_tree);
      parser.parse();
      time_t end = time(NULL);
      printf("compilation: %d sec\n", int(end - start));
   }
   virtual void run() {
      try {
         ReFileSourceUnit* unit = dynamic_cast<ReFileSourceUnit*>(m_reader
                                  .currentSourceUnit());
         if (unit != NULL && !unit->isOpen())
            throw ReException("file not found: %s", m_filename);
         benchmark();
      } catch (ReException ex) {
         printf("%s\n", ex.getMessage().constData());
      }
   }
};
void testRplBenchmark() {
   TestRplBenchmark test;
   test.run();
}

