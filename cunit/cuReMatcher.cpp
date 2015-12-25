/*
 * cuReMatcher.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Unit test of the ReString tools.
 */

#include "../base/rebase.hpp"

class TestReMatcher: public ReTest {
public:
   TestReMatcher() :
      ReTest("ReMatcher") {
      doIt();
   }

public:
   void testBasics() {
      ReMatcher m1("a*b*c*", Qt::CaseSensitive, true);
      checkT(m1.matches("a b c d"));
      checkT(m1.matches("abc d"));
      checkT(m1.matches("ababc"));
      checkT(m1.matches("abc"));
      checkF(m1.matches("aBc"));
      ReMatcher m2("a*b*c", Qt::CaseSensitive, false);
      checkT(m2.matches("a b c d"));
      checkT(m2.matches("ababc"));
      checkT(m2.matches("a b a b c"));
      checkT(m2.matches(" abc "));
      checkF(m2.matches(" ab"));
      ReMatcher m3("a**B*C", Qt::CaseInsensitive, true);
      checkT(m3.matches("a b C"));
      checkT(m3.matches("ab c"));
      checkT(m3.matches("ababc"));
      checkT(m3.matches("abc"));
      checkF(m3.matches("abcd"));
      ReMatcher m4("A*B*c", Qt::CaseInsensitive, false);
      checkT(m4.matches("a b c d"));
      checkT(m4.matches("ababc"));
      checkT(m4.matches("a b a b c"));
      checkT(m4.matches(" abc"));
      checkF(m4.matches(" ab"));
      m4.setPattern("*");
      checkT(m4.matches("x"));
      m4.setPattern("");
      checkT(m4.matches("any"));
   }
   void test1Star() {
      ReMatcher matcher("*abc", Qt::CaseSensitive, true);
      checkT(matcher.matches("abc"));
      checkF(matcher.matches("aBc"));
      checkT(matcher.matches("xyzabc"));
      checkF(matcher.matches("abc "));
      matcher.setCaseSensivitiy(Qt::CaseInsensitive);
      checkT(matcher.matches("abc"));
      checkT(matcher.matches("aBc"));
      checkT(matcher.matches("xyzabc"));
      checkF(matcher.matches("abc "));
      checkT(matcher.matches("AbC"));
      checkT(matcher.matches("aBc"));
      checkT(matcher.matches("xyzAbc"));
      checkF(matcher.matches("abC "));
      matcher.setPattern(matcher.pattern(), false);
      matcher.setCaseSensivitiy(Qt::CaseSensitive);
      checkT(matcher.matches("abc"));
      checkF(matcher.matches("aBc"));
      checkT(matcher.matches("xyzabc"));
      checkT(matcher.matches("abc "));
      checkT(matcher.matches("_abc_"));
      checkF(matcher.matches("_a bc_"));
      matcher.setCaseSensivitiy(Qt::CaseInsensitive);
      checkT(matcher.matches("abc"));
      checkT(matcher.matches("aBc"));
      checkT(matcher.matches("xyzAbc"));
      checkT(matcher.matches("aBc "));
      checkT(matcher.matches("_abC_"));
      checkF(matcher.matches("_a bc_"));
      matcher.setPattern("x*y", true);
      matcher.setCaseSensivitiy(Qt::CaseSensitive);
      checkT(matcher.matches("x y"));
      checkF(matcher.matches("X y"));
      checkF(matcher.matches("x Y"));
      checkT(matcher.matches("xy"));
      checkF(matcher.matches("Xy"));
      checkF(matcher.matches(" xy"));
      checkF(matcher.matches("xy "));
      matcher.setCaseSensivitiy(Qt::CaseInsensitive);
      checkT(matcher.matches("X Y"));
      checkT(matcher.matches("xY"));
      checkT(matcher.matches("Xy"));
      checkF(matcher.matches(" xy"));
      checkF(matcher.matches("xy "));
      matcher.setPattern(matcher.pattern(), false);
      matcher.setCaseSensivitiy(Qt::CaseSensitive);
      checkT(matcher.matches("x y"));
      checkT(matcher.matches("ax y"));
      checkT(matcher.matches("x y!"));
      checkT(matcher.matches("123xyz!"));
      checkF(matcher.matches("x"));
      checkF(matcher.matches("xY"));
      checkF(matcher.matches("Xy"));
      matcher.setCaseSensivitiy(Qt::CaseInsensitive);
      checkT(matcher.matches("X y"));
      checkT(matcher.matches("aX y"));
      checkT(matcher.matches("x Y!"));
      checkT(matcher.matches("123XY!"));
      checkF(matcher.matches("x"));
      checkF(matcher.matches("xY"));
      checkF(matcher.matches("Xy"));
   }
   void test0Star() {
      ReMatcher matcher("abc", Qt::CaseSensitive, true);
      checkT(matcher.matches("abc"));
      checkF(matcher.matches("aBc"));
      checkF(matcher.matches(" abc"));
      checkF(matcher.matches("abc "));
      checkT(matcher.caseSensivitiy());
      matcher.setCaseSensivitiy(Qt::CaseInsensitive);
      checkF(matcher.caseSensivitiy());
      checkT(matcher.matches("abc"));
      checkT(matcher.matches("aBc"));
      checkF(matcher.matches(" aBc"));
      checkF(matcher.matches("aBc "));
      matcher.setPattern(matcher.pattern(), false);
      matcher.setCaseSensivitiy(Qt::CaseSensitive);
      checkT(matcher.matches("abc"));
      checkF(matcher.matches("aBc"));
      checkT(matcher.matches(" abc"));
      checkT(matcher.matches("abc "));
   }

   void testList() {
      QStringList patterns;
      patterns << "*.txt" << "*.doc";
      ReListMatcher matcher(patterns, Qt::CaseInsensitive, true);
      checkT(matcher.matches("README.TXT"));
      checkF(matcher.matches("readme_txt"));
      checkT(matcher.matches("Xyz.Doc"));
      checkF(matcher.matches("a.doc.bak"));
      ReListMatcher matcher2(ReQStringUtils::m_emptyList, Qt::CaseInsensitive, true);
      checkT(matcher2.matches("abc"));
   }

   virtual void runTests(void) {
      testBasics();
      test0Star();
      test1Star();
      testList();
   }
};
void testReMatcher() {
   TestReMatcher test;
}

