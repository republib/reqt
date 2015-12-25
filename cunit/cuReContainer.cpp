/*
 * cuReContainer.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"
/**
 * @brief Unit test for <code>ReContainer</code>
 */
class TestReContainer: public ReTest {
public:
   TestReContainer() :
      ReTest("RplContainer") {
      doIt();
   }

public:
   void testBasic() {
      ReContainer container(256);
      //  Rpl&1 09 36[2]cis:!7b Nirwana &lt;0&gt; Y -ab34 A long string with an trailing '0' &lt;0&gt<br>
      container.startBag();
      container.addChar('!');
      container.addInt(123);
      container.addString("Nirwana");
      container.startBag();
      container.addChar('Y');
      container.addInt(-0xab34);
      container.addString("A long string with an trailing '0'");
      QByteArray data = container.getData();
      ReContainer container2(256);
      container2.fill(data);
      checkEqu(2, container2.getCountBags());
      checkEqu('!', container2.nextChar());
      checkEqu(123, container2.nextInt());
      checkEqu("Nirwana", container2.nextString());
      container2.nextBag();
      checkEqu('Y', container2.nextChar());
      checkEqu(-0xab34, container2.nextInt());
      checkEqu("A long string with an trailing '0'", container2.nextString());
      log(("Example: " + data).constData());
   }

   virtual void runTests() {
      testBasic();
   }
};

void testReContainer() {
   TestReContainer test;
}

