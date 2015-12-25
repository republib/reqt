/*
 * cuReEnigma.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"
#include "math/remath.hpp"
/**
 * @brief Unit test for <code>ReEnigma</code>.
 */
class TestReEnigma: public ReTest {
public:
   TestReEnigma() :
      ReTest("ReEnigma") {
      doIt();
   }

public:
   void testOneCharset(const char* value, const char* charSet,
                       const char* expected) {
      ReEnigma enigma;
      enigma.addByteSecret(QByteArray("Geheim"));
      enigma.setSeed(0);
      QByteArray encoded = value;
      QByteArray booster;
      enigma.encode(encoded.data(), encoded.length(), charSet, booster);
      //printString(encoded.constData());
      QByteArray decoded = encoded;
      enigma.setSeed(0);
      enigma.decode(decoded.data(), decoded.length(), charSet, booster);
      checkEqu(value, decoded.constData());
      checkEqu(expected, encoded);
   }

   void printCharSets() {
      QByteArray value;
      value.reserve(256);
      unsigned char cc;
      for (cc = ' '; cc <= 127; cc++) {
         if (cc == '"' || cc == '\\')
            value.append('\\');
         value.append(cc);
      }
      printf("%s\n", value.constData());
      value.resize(0);
      for (cc = 128; cc >= 128; cc++) {
         char buf[10];
         if (cc % 32 == 0)
            value.append("\n");
         sprintf(buf, "\\x%02x", cc);
         value.append(buf);
      }
      printf("%s\n", value.constData());
   }
   void printString(const char* value) {
      QByteArray v;
      unsigned char cc;
      while ((cc = (unsigned char) *value++) != 0) {
         if (cc == '\\' || cc == '"') {
            v.append('\\');
            v.append(cc);
         } else if (cc >= 127) {
            char buffer[10];
            sprintf(buffer, "\\x%02x", cc);
            v.append(buffer);
         } else {
            v.append(cc);
         }
      }
      printf("%s\n", v.constData());
   }
   void testOneBytes(const char* bytes) {
      ReEnigma enigma;
      enigma.addByteSecret("Hello World");
      enigma.setSeed(0x1234);
      QByteArray encoded(bytes);
      enigma.change(encoded);
      enigma.setSeed(0x1234);
      QByteArray decoded(encoded);
      enigma.change(decoded);
      checkEqu(bytes, decoded);
   }

   void testBytes() {
      testOneBytes("abcdefg");
      testOneBytes(
         "01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
   }

   void testCharSet() {
      //testOneCharset("&()[]{}Weiß der Geier/Kuckuck?", ReEnigma::SET_32_255, "2Kc\x9a\xfeQ\xd7\xa84sx)*\xfb\xd2z\xf4\"W\xb0\xee\xb0\xd1\x84\xace\xf8_u*T");
      testOneCharset("\\Weiß der Geier/Kuckuck?", ReEnigma::SET_32_127,
                     "(Z?hßaZ_#/QZ+Oi|SI^=<,)A");
      testOneCharset("01234567890abcdef", ReEnigma::SET_HEXDIGITS,
                     "c4c25b08735c53a63");
      testOneCharset("data$1%3.^~", ReEnigma::SET_FILENAME, "^voazo-n%$b");
      testOneCharset("Weiß der Geier!", ReEnigma::SET_ALPHANUM,
                     "weyß BCk 19NoO!");
      testOneCharset("12345678901234567890", ReEnigma::SET_DECIMALS,
                     "97394833084815683977");
      testOneCharset("000000000000000000000000000", ReEnigma::SET_DECIMALS,
                     "850592651836811261879625929");
   }

   virtual void run() {
      testBytes();
      testCharSet();
   }
};

void testReEnigma() {
   TestReEnigma test;
}
