/*
 * cuReByteStorage.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
/** @file
 * @brief Unit test of the byte and C string storage.
 */

#include "base/rebase.hpp"

class TestReByteStorage: public ReTest {
public:
   TestReByteStorage() :
      ReTest("ReByteStorage") {
      doIt();
   }
private:
   void testChars() {
      ReByteStorage store(100);
      char* s1 = store.allocateChars(4);
      memcpy((void*) s1, "123", 4);
      const char* s2 = store.allocateChars("abc");
      const char* s3 = store.allocateChars("defghij", 3);
      checkEqu(s1, "123");
      checkEqu(s2, "abc");
      checkEqu(s3, "def");
      const char* ptr = s1 + 4;
      checkT(ptr == s2);
      ptr += 4;
      checkT(ptr == s3);
   }

   void testBytes() {
      ReByteStorage store(100);
      uint8_t* s1 = store.allocateBytes(4);
      memcpy((void*) s1, "1234", 4);
      uint8_t* s2 = store.allocateBytes((void*) "abcd", 4);
      uint8_t* s3 = store.allocateBytes((void*) "efghij", 4);
      uint8_t* s4 = store.allocateZeros(4);
      checkEqu("1234abcdefgh", (const char*) s1);
      uint8_t* ptr = s1 + 4;
      checkT(ptr == s2);
      ptr += 4;
      checkT(ptr == s3);
      ptr += 4;
      checkT(ptr == s4);
      for (int ii = 0; ii < 4; ii++)
         checkEqu(0, (int ) s4[ii]);
   }
   void testBufferChange() {
      ReByteStorage store(10);
      char buffer[2];
      buffer[1] = '\0';
      for (int ii = 0; ii < 10000; ii++) {
         buffer[1] = 'A' + ii % 26;
         store.allocateBytes(buffer, 1);
      }
   }

public:
   virtual void runTests() {
      testBufferChange();
      testChars();
      testBytes();
   }
};
void testReByteStorage() {
   TestReByteStorage test;
}

